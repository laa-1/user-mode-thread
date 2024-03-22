#define _POSIX_SOURCE // sigprocmask函数等需要这个宏定义才能生效，需要这里通过宏来指定使用POSIX.1标准的

#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "user_mode_thread.h"

#define STACK_SIZE 10240  // 栈的大小
#define TIME_TICK_SIZE 10 // 每个时间片的毫秒数
#define THREAD_STATUS_EXIT 0
#define THREAD_STATUS_RUNNING 1
#define THREAD_STATUS_SLEEP 2

// 存放线程信息的结构体
struct TCB
{
    int64_t id;                     // 线程id
    void (*thd_func)();             // 线程主函数
    int64_t rsp;                    // 栈顶指针
    int status;                     // 线程状态
    unsigned long long wakeup_time; // 唤醒时间
    int counter;                    // 时间片数
    int priority;                   // 优先级
    int64_t stack[STACK_SIZE];      // 线程运行栈
};

struct TCB tcbs[MAX_THREAD_NUM];                                                                 // tcb数组
struct TCB *current = NULL;                                                                      // 当前线程的tcb的指针
int mutex = 0;                                                                                   // 互斥锁
int mutex_owner = 0;                                                                             // 互斥锁持有者
int sems[10] = {-32768, -32768, -32768, -32768, -32768, -32768, -32768, -32768, -32768, -32768}; // 信号量
sigset_t sig_mask;                                                                               // 用于屏蔽alarm信号的相关数据结构
struct sigaction sig_act;                                                                        // 用于设置信号处理函数的相关数据结构

void switch_thread(struct TCB *current, struct TCB *next, struct TCB **current_addr);

// 屏蔽alarm信号
// 触发信号时会切换至信号处理函数执行，这里的信号处理函数是不可重入，如果有可能发生内存的读写冲突，则需要屏蔽alarm信号
void close_alarm()
{
    sigprocmask(SIG_BLOCK, &sig_mask, NULL);
}

// 解除屏蔽alarm信号
void open_alarm()
{
    sigprocmask(SIG_UNBLOCK, &sig_mask, NULL);
}

// 获取毫秒级精度时间
unsigned long long get_ms_time()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

// 获取下一个要执行的线程
struct TCB *get_next()
{
    // 基于优先级的时间片轮转调度
    int max_counter;
    int next_tid;
    while (1)
    {
        max_counter = -1;
        next_tid = 0;
        // 找出剩余时间片数最多的
        for (int i = 0; i < MAX_THREAD_NUM; ++i)
        {
            if (tcbs[i].status == THREAD_STATUS_RUNNING && tcbs[i].counter > max_counter)
            {
                max_counter = tcbs[i].counter;
                next_tid = i;
            }
        }
        if (max_counter == 0)
        {
            // 对每个线程重新调整时间片的值
            for (int i = 0; i < MAX_THREAD_NUM; ++i)
            {
                if (tcbs[i].status != THREAD_STATUS_EXIT)
                {
                    tcbs[i].counter = tcbs[i].counter / 2 + tcbs[i].priority;
                }
            }
        }
        else
        {
            return &(tcbs[next_tid]);
        }
    }
}

// 唤醒已经到唤醒时间的线程
void wakeup()
{
    // 尝试唤醒处于睡眠状态的线程
    // 如果全部都处于睡眠状态，则循环尝试唤醒
    int have_running = 0;
    while (!have_running)
    {
        for (int i = 0; i < MAX_THREAD_NUM; i++)
        {
            if (tcbs[i].status == THREAD_STATUS_SLEEP)
            {
                if (get_ms_time() > tcbs[i].wakeup_time)
                {
                    tcbs[i].status = THREAD_STATUS_RUNNING;
                    have_running = 1;
                }
            }
            else if (tcbs[i].status == THREAD_STATUS_RUNNING)
            {
                have_running = 1;
            }
        }
    }
}

// 主动切换线程
void proactive_switch()
{
    close_alarm();
    wakeup();
    struct TCB *next = get_next();
    open_alarm();
    switch_thread(current, next, &current);
    // 线程切换后，此后的代码将不会在新线程中执行，需要等待到切回旧线程时才会从这里继续执行
}

// 被动切换线程
void passive_switch(int sig)
{
    // 信号回调函数是一次性的，需要再次注册
    sigaction(SIGALRM, &sig_act, 0);
    if (mutex && mutex_owner == current->id)
    {
        // 若已上锁，且当前线程是锁的持有者，则不递减时间片数，也不进行线程切换
        return;
    }
    current->counter--;
    if (current->counter <= 0)
    {
        current->counter = 0;
        wakeup();
        struct TCB *next = get_next();
        switch_thread(current, next, &current);
    }
}

// 初始化
void init()
{
    // main函数是第一个线程
    struct TCB thd = {0, NULL, 0, THREAD_STATUS_RUNNING, 0, DAFLULT_PRIORITY, DAFLULT_PRIORITY, {0}};
    tcbs[0] = thd;
    current = &(tcbs[0]);
    // 设置用于屏蔽SIGALRM信号的相关数据结构
    sigemptyset(&sig_mask);
    sigaddset(&sig_mask, SIGALRM);
    // 设置信号处理函数
    sig_act.sa_handler = passive_switch;
    sigaction(SIGALRM, &sig_act, 0);
    // 设置计时器
    struct itimerval itv;
    itv.it_value.tv_sec = 0;
    itv.it_value.tv_usec = 1000 * TIME_TICK_SIZE;
    itv.it_interval.tv_sec = 0;
    itv.it_interval.tv_usec = 1000 * TIME_TICK_SIZE;
    setitimer(ITIMER_REAL, &itv, NULL);
}

// 线程的启动函数
void start(struct TCB *thd)
{
    thd->thd_func();
    // 线程结束后才会执行此处代码
    // 状态设置为EXIT，切换线程后将不会被分配时间片，即start函数永远不会返回
    thd->status = THREAD_STATUS_EXIT;
    proactive_switch();
}

// 创建线程，成功返回线程id，失败返回-1
int thread_create(void (*thd_func)(), int priority)
{
    close_alarm();
    int tid = 0;
    for (; tid < MAX_THREAD_NUM; tid++)
    {
        if (tcbs[tid].status == THREAD_STATUS_EXIT)
        {
            break;
        }
    }
    if (tid == MAX_THREAD_NUM)
    {
        // 线程数已满
        return -1;
    }
    // 初始化tcb
    tcbs[tid].id = tid;
    tcbs[tid].thd_func = thd_func;
    tcbs[tid].rsp = (int64_t)(tcbs[tid].stack + STACK_SIZE - 18);
    tcbs[tid].wakeup_time = 0;
    tcbs[tid].counter = priority;
    tcbs[tid].priority = priority;
    // 初始化新线程的栈
    // 第一次切换到新线程时，switch_thread函数会pop出rflags到rbp的值，并会调用ret指令跳转一个地址中，这里设置为start函数
    // x64中rdi用于存放函数的第一个参数
    tcbs[tid].stack[STACK_SIZE - 18] = 0;                      // rflags
    tcbs[tid].stack[STACK_SIZE - 17] = 0;                      // r15
    tcbs[tid].stack[STACK_SIZE - 16] = 0;                      // r14
    tcbs[tid].stack[STACK_SIZE - 15] = 0;                      // r13
    tcbs[tid].stack[STACK_SIZE - 14] = 0;                      // r12
    tcbs[tid].stack[STACK_SIZE - 13] = 0;                      // r11
    tcbs[tid].stack[STACK_SIZE - 12] = 0;                      // r10
    tcbs[tid].stack[STACK_SIZE - 11] = 0;                      // r9
    tcbs[tid].stack[STACK_SIZE - 10] = 0;                      // r8
    tcbs[tid].stack[STACK_SIZE - 9] = (int64_t) & (tcbs[tid]); // rdi，也是start函数的参数
    tcbs[tid].stack[STACK_SIZE - 8] = 0;                       // rsi
    tcbs[tid].stack[STACK_SIZE - 7] = 0;                       // rdx
    tcbs[tid].stack[STACK_SIZE - 6] = 0;                       // rcx
    tcbs[tid].stack[STACK_SIZE - 5] = 0;                       // rbx
    tcbs[tid].stack[STACK_SIZE - 4] = 0;                       // rax
    tcbs[tid].stack[STACK_SIZE - 3] = 0;                       // rbp
    tcbs[tid].stack[STACK_SIZE - 2] = (int64_t)start;          // start函数的地址
    tcbs[tid].stack[STACK_SIZE - 1] = 0;                       // start函数永远不会返回，后续的区域没有意义
    tcbs[tid].status = THREAD_STATUS_RUNNING;
    open_alarm();
    return tid;
}

// 线程睡眠，毫秒为单位，不会释放锁
void thread_sleep(unsigned long long msec)
{
    current->wakeup_time = get_ms_time() + msec;
    current->status = THREAD_STATUS_SLEEP;
    proactive_switch();
}

// 等待另一个线程结束，会释放锁
void thread_wait(int tid)
{
    thread_mutex_unlock();
    while (tcbs[tid].status != THREAD_STATUS_EXIT)
    {
        proactive_switch();
    }
    thread_mutex_lock();
}

// 互斥锁上锁，如已经上锁则阻塞
void thread_mutex_lock()
{
    while (mutex)
    {
        thread_sleep(1);
    }
    close_alarm();
    mutex = 1;
    mutex_owner = current->id;
    open_alarm();
}

// 互斥锁解锁，只能由互斥锁持有者调用，非持有者调用则无效果
void thread_mutex_unlock()
{
    close_alarm();
    if (mutex_owner == current->id)
    {
        mutex = 0;
    }
    open_alarm();
}

// 申请一个信号量并初始化，成功返回信号量id，失败返回-1
int thread_sem_init(int sem_init)
{
    if (sem_init <= -32768)
    {
        // 信号量等于-32768时视为未使用，不能初始化为小于或等于-32768的值
        return -1;
    }
    close_alarm();
    for (int i = 0; i < 5; i++)
    {

        if (sems[i] == -32768)
        {
            sems[i] = sem_init;
            open_alarm();
            return i;
        }
    }
    open_alarm();
    return -1;
}

// 释放信号量
void thread_sem_del(int sid)
{
    close_alarm();
    sems[sid] = -32768;
    open_alarm();
}

// 信号量P操作，P操作失败则阻塞
void thread_sem_p(int sid)
{
    while (!sems[sid])
    {
        thread_sleep(1);
    }
    close_alarm();
    sems[sid]--;
    open_alarm();
}

// 信号量V操作
void thread_sem_v(int sid)
{
    close_alarm();
    sems[sid]++;
    open_alarm();
}
