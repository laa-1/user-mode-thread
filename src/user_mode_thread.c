#define _POSIX_SOURCE

#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "user_mode_thread.h"

#define STACK_SIZE (1024 * 1024 / 8) // 单位是8字节，这里设置为1M字节
#define TIME_SLICE_MSEC 10
#define THREAD_EXIT 0
#define THREAD_RUNNABLE 1
#define THREAD_SLEEP 2
#define THREAD_WAIT 3
#define MUTEX_LOCK 1
#define MUTEX_UNLOCK 0
#define SPINLOCK_LOCK 1
#define SPINLOCK_UNLOCK 0

// 存放线程信息的结构体
typedef struct
{
    int64_t rsp;                    // 栈顶指针
    void (*run)();                  // 入口函数
    int id;                         // id
    char status;                    // 状态
    char priority;                  // 优先级
    char counter;                   // 时间片数
    unsigned long long wakeup_time; // 唤醒时间
    int64_t stack[STACK_SIZE];      // 运行时栈
} tcb_t;

tcb_t *tcbs[MAX_THREAD_NUM];
tcb_t *cur_tcb = NULL;
sigset_t alarm_sigset;
struct sigaction alarm_sigaction;

void switch_thread(tcb_t *next, tcb_t *cur_tcb_value, tcb_t **cur_tcb_addr);

unsigned long long get_now_msec()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

// 相当于关中断，避免某些操作在执行时中途切换线程
void close_interrupt()
{
    sigprocmask(SIG_BLOCK, &alarm_sigset, NULL);
}

void open_interrupt()
{
    sigprocmask(SIG_UNBLOCK, &alarm_sigset, NULL);
}

tcb_t *pick_next_thread()
{
    // 基于优先级的时间片轮转调度
    int max_counter;
    int next_tid;
    while (1)
    {
        max_counter = 0;
        next_tid = 0;
        // 找出剩余时间片数最多的
        for (int i = 0; i < MAX_THREAD_NUM; i++)
        {
            if (tcbs[i] == NULL || tcbs[i]->status == THREAD_EXIT)
            {
                continue;
            }
            if (tcbs[i]->status == THREAD_RUNNABLE && tcbs[i]->counter > max_counter)
            {
                max_counter = tcbs[i]->counter;
                next_tid = i;
            }
        }
        if (max_counter == 0)
        {
            // 可选择的线程都没有时间片，重新分配时间片
            for (int i = 0; i < MAX_THREAD_NUM; i++)
            {
                if (tcbs[i] == NULL || tcbs[i]->status == THREAD_EXIT)
                {
                    continue;
                }
                tcbs[i]->counter = tcbs[i]->counter / 2 + tcbs[i]->priority;
            }
        }
        else
        {
            return tcbs[next_tid];
        }
    }
}

void try_wakeup_thread()
{
    int have_running = 0;
    // 如果全部都处于睡眠状态，则循环尝试唤醒
    while (!have_running)
    {
        for (int i = 0; i < MAX_THREAD_NUM; i++)
        {
            if (tcbs[i] != NULL)
            {
                if (tcbs[i]->status == THREAD_SLEEP)
                {
                    if (get_now_msec() > tcbs[i]->wakeup_time)
                    {
                        tcbs[i]->status = THREAD_RUNNABLE;
                        have_running = 1;
                    }
                }
                else if (tcbs[i]->status == THREAD_RUNNABLE)
                {
                    have_running = 1;
                }
            }
        }
    }
}

void try_clean_thread()
{
    for (int i = 0; i < MAX_THREAD_NUM; i++)
    {
        if (tcbs[i] != NULL)
        {
            // 当前线程依赖于其对应的栈，不能对当前线程进行清理
            if (tcbs[i] != cur_tcb && tcbs[i]->status == THREAD_EXIT)
            {
                free(tcbs[i]);
                tcbs[i] = NULL;
            }
        }
    }
}

void proactive_switch_thread()
{
    close_interrupt();
    try_clean_thread();
    try_wakeup_thread();
    switch_thread(pick_next_thread(), cur_tcb, &cur_tcb);
    // 线程切换后，此后的代码将不会在新线程中执行，需要等待到切回旧线程时才会从这里继续执行
}

void passive_switch_thread()
{
    // 信号回调函数是一次性的，需要再次注册
    sigaction(SIGALRM, &alarm_sigaction, 0);
    cur_tcb->counter--;
    if (cur_tcb->counter <= 0)
    {
        cur_tcb->counter = 0;
        try_clean_thread();
        try_wakeup_thread();
        switch_thread(pick_next_thread(), cur_tcb, &cur_tcb);
        // 线程切换后，此后的代码将不会在新线程中执行，需要等待到切回旧线程时才会从这里继续执行
    }
}

void run_before(tcb_t *tcb)
{
    tcb->run();
    // 线程结束后才会执行此处代码
    tcb->status = THREAD_EXIT;
    proactive_switch_thread();
    // 状态设置为EXIT，切换线程后将不会被分配时间片，该函数永远不会返回
}

void thread_main_init()
{
    // main函数是第一个线程
    tcbs[0] = malloc(sizeof(tcb_t));
    tcbs[0]->id = 0;
    tcbs[0]->status = THREAD_RUNNABLE;
    tcbs[0]->priority = DAFLULT_PRIORITY;
    tcbs[0]->counter = DAFLULT_PRIORITY;
    tcbs[0]->wakeup_time = 0;
    cur_tcb = tcbs[0];
    sigemptyset(&alarm_sigset);
    sigaddset(&alarm_sigset, SIGALRM);
    alarm_sigaction.sa_handler = passive_switch_thread;
    struct itimerval alarm_itimerval;
    alarm_itimerval.it_value.tv_sec = 0;
    alarm_itimerval.it_value.tv_usec = 1000 * TIME_SLICE_MSEC;
    alarm_itimerval.it_interval.tv_sec = 0;
    alarm_itimerval.it_interval.tv_usec = 1000 * TIME_SLICE_MSEC;
    sigaction(SIGALRM, &alarm_sigaction, 0);
    setitimer(ITIMER_REAL, &alarm_itimerval, NULL);
}

int thread_create(void (*run)(), int priority)
{
    close_interrupt();
    int tid = 0;
    for (; tid < MAX_THREAD_NUM; tid++)
    {
        if (tcbs[tid] == NULL)
        {
            break;
        }
    }
    if (tid == MAX_THREAD_NUM)
    {
        open_interrupt();
        return -1;
    }
    tcbs[tid] = malloc(sizeof(tcb_t));
    tcbs[tid]->rsp = (int64_t)(tcbs[tid]->stack + STACK_SIZE - 18);
    tcbs[tid]->run = run;
    tcbs[tid]->id = tid;
    tcbs[tid]->status = THREAD_RUNNABLE;
    tcbs[tid]->priority = priority;
    tcbs[tid]->counter = priority;
    tcbs[tid]->wakeup_time = 0;
    tcbs[tid]->stack[STACK_SIZE - 18] = 0;                  // rflags
    tcbs[tid]->stack[STACK_SIZE - 17] = 0;                  // r15
    tcbs[tid]->stack[STACK_SIZE - 16] = 0;                  // r14
    tcbs[tid]->stack[STACK_SIZE - 15] = 0;                  // r13
    tcbs[tid]->stack[STACK_SIZE - 14] = 0;                  // r12
    tcbs[tid]->stack[STACK_SIZE - 13] = 0;                  // r11
    tcbs[tid]->stack[STACK_SIZE - 12] = 0;                  // r10
    tcbs[tid]->stack[STACK_SIZE - 11] = 0;                  // r9
    tcbs[tid]->stack[STACK_SIZE - 10] = 0;                  // r8
    tcbs[tid]->stack[STACK_SIZE - 9] = (int64_t)tcbs[tid];  // rdi，也是run_before函数的参数，x64中rdi用于存放函数的第一个参数
    tcbs[tid]->stack[STACK_SIZE - 8] = 0;                   // rsi
    tcbs[tid]->stack[STACK_SIZE - 7] = 0;                   // rdx
    tcbs[tid]->stack[STACK_SIZE - 6] = 0;                   // rcx
    tcbs[tid]->stack[STACK_SIZE - 5] = 0;                   // rbx
    tcbs[tid]->stack[STACK_SIZE - 4] = 0;                   // rax
    tcbs[tid]->stack[STACK_SIZE - 3] = 0;                   // rbp
    tcbs[tid]->stack[STACK_SIZE - 2] = (int64_t)run_before; // run_before函数的地址，第一次切换到新线程时，switch_thread函数会pop出rflags到rbp的值，并会调用ret指令返回至此处
    tcbs[tid]->stack[STACK_SIZE - 1] = 0;                   // 无意义，run_before函数永远不会返回
    open_interrupt();
    return tid;
}

void thread_sleep(unsigned long long msec)
{
    cur_tcb->wakeup_time = get_now_msec() + msec;
    cur_tcb->status = THREAD_SLEEP;
    proactive_switch_thread();
}

void thread_join(int tid)
{
    while (!(tcbs[tid] == NULL || tcbs[tid]->status == THREAD_EXIT))
    {
        cur_tcb->status = THREAD_SLEEP;
        proactive_switch_thread();
    }
}

void thread_yield()
{
    proactive_switch_thread();
}

void thread_mutex_init(mutex_t *mutex)
{
    mutex->status = MUTEX_UNLOCK;
    mutex->owner_tid = -1;
}

void thread_mutex_lock(mutex_t *mutex)
{
REPEAT:
    while (mutex->status == MUTEX_LOCK)
    {
        proactive_switch_thread();
    }
    close_interrupt();
    // 此处可能会出现，在检测到解锁之后，关中断之前，被切换了线程，故需要再次检查
    if (mutex->status == MUTEX_LOCK)
    {
        open_interrupt();
        goto REPEAT;
    }
    else
    {
        mutex->status = MUTEX_LOCK;
        mutex->owner_tid = cur_tcb->id;
        open_interrupt();
    }
}

void thread_mutex_unlock(mutex_t *mutex)
{
    if (mutex->owner_tid == cur_tcb->id)
    {
        close_interrupt();
        mutex->status = MUTEX_UNLOCK;
        mutex->owner_tid = -1;
        open_interrupt();
    }
}

void thread_spinlock_init(spinlock_t *spinlock)
{
    spinlock->status = SPINLOCK_UNLOCK;
    spinlock->owner_tid = -1;
}

void thread_spinlock_lock(spinlock_t *spinlock)
{
REPEAT:
    while (spinlock->status == SPINLOCK_LOCK)
    {
        // 忙等待
    }
    close_interrupt();
    // 此处可能会出现，在检测到解锁之后，关中断之前，被切换了线程，故需要再次检查（不在每次循环中都启用关中断来检查是为了降低性能开销）
    if (spinlock->status == SPINLOCK_LOCK)
    {
        open_interrupt();
        goto REPEAT;
    }
    else
    {
        spinlock->status = SPINLOCK_LOCK;
        spinlock->owner_tid = cur_tcb->id;
        open_interrupt();
    }
}

void thread_spinlock_unlock(spinlock_t *spinlock)
{
    if (spinlock->owner_tid == cur_tcb->id)
    {
        close_interrupt();
        spinlock->status = SPINLOCK_UNLOCK;
        spinlock->owner_tid = -1;
        open_interrupt();
    }
}

void thread_rwlock_init(rwlock_t *rwlock)
{
    rwlock->read_count = 0;
    rwlock->write_count = 0;
}

void thread_rwlock_read_lock(rwlock_t *rwlock)
{
REPEAT:
    while (rwlock->write_count != 0)
    {
        proactive_switch_thread();
    }
    close_interrupt();
    // 此处可能会出现，在检测到写锁归零之后，关中断之前，被切换了线程，故需要再次检查（不在每次循环中都启用关中断来检查是为了降低性能开销）
    if (rwlock->write_count != 0)
    {
        open_interrupt();
        goto REPEAT;
    }
    else
    {
        rwlock->read_count++;
        open_interrupt();
    }
}

void thread_rwlock_read_unlock(rwlock_t *rwlock)
{
    close_interrupt();
    rwlock->read_count--;
    open_interrupt();
}

void thread_rwlock_write_lock(rwlock_t *rwlock)
{
REPEAT:
    while (rwlock->read_count != 0 || rwlock->write_count != 0)
    {
        proactive_switch_thread();
    }
    close_interrupt();
    // 此处可能会出现，在检测到写锁归零之后，关中断之前，被切换了线程，故需要再次检查（不在每次循环中都启用关中断来检查是为了降低性能开销）
    if (rwlock->read_count != 0 || rwlock->write_count != 0)
    {
        open_interrupt();
        goto REPEAT;
    }
    else
    {
        rwlock->write_count++;
        open_interrupt();
    }
}

void thread_rwlock_write_unlock(rwlock_t *rwlock)
{
    close_interrupt();
    rwlock->write_count--;
    open_interrupt();
}

void thread_condvar_init(condvar_t *condvar)
{
    linkqueue_init(&(condvar->linkqueue));
}

void thread_condvar_destroy(condvar_t *condvar)
{
    linkqueue_destory(&(condvar->linkqueue));
}

void thread_condvar_wait(condvar_t *condvar, mutex_t *mutex)
{
    close_interrupt();
    cur_tcb->status = THREAD_WAIT;
    linkqueue_push_back(&(condvar->linkqueue), cur_tcb->id);
    open_interrupt();
    thread_mutex_unlock(mutex);
    proactive_switch_thread();
    thread_mutex_lock(mutex);
}

void thread_condvar_signal(condvar_t *condvar)
{
    close_interrupt();
    if (!linkqueue_empty(&(condvar->linkqueue)))
    {
        tcbs[linkqueue_front(&(condvar->linkqueue))]->status = THREAD_RUNNABLE;
        linkqueue_pop_front(&(condvar->linkqueue));
    }
    open_interrupt();
}

void thread_condvar_broadcast(condvar_t *condvar)
{
    close_interrupt();
    while (!linkqueue_empty(&(condvar->linkqueue)))
    {
        tcbs[linkqueue_front(&(condvar->linkqueue))]->status = THREAD_RUNNABLE;
        linkqueue_pop_front(&(condvar->linkqueue));
    }
    open_interrupt();
}
