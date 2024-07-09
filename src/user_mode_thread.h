#ifndef INCLUDE_USER_MODE_THREAD_H_
#define INCLUDE_USER_MODE_THREAD_H_

#include "linkqueue.h"

#define MAX_THREAD_NUM 128
#define DAFLULT_PRIORITY 15 // 默认优先级

typedef struct
{
    int status;
    int owner_tid;
} mutex_t;

typedef struct
{
    int status;
    int owner_tid;
} spinlock_t;

typedef struct
{
    int read_count;
    int write_count;
} rwlock_t;

typedef struct
{
    linkqueue_t linkqueue;
} condvar_t;

// 主线程初始化，不初始化会导致后续主线程丢失
void thread_main_init();
// 线程创建，成功返回线程id，失败返回-1
int thread_create(void (*run)(), int priority);
// 线程睡眠，单位毫秒
void thread_sleep(unsigned long long msec);
// 线程等待另一个线程结束
void thread_join(int tid);
// 线程主动让出CPU
void thread_yield();
// 互斥锁初始化
void thread_mutex_init(mutex_t *mutex);
// 互斥锁上锁，如已经上锁则阻塞
void thread_mutex_lock(mutex_t *mutex);
// 互斥锁解锁，非持有者无效
void thread_mutex_unlock(mutex_t *mutex);
// 自旋锁初始化
void thread_spinlock_init(spinlock_t *spinlock);
// 自旋锁上锁，如已经上锁则忙等待
void thread_spinlock_lock(spinlock_t *spinlock);
// 自旋锁解锁，非持有者无效
void thread_spinlock_unlock(spinlock_t *spinlock);
// 读写锁初始化
void thread_rwlock_init(rwlock_t *rwlock);
// 读锁上锁
void thread_rwlock_read_lock(rwlock_t *rwlock);
// 读锁解锁，不会检查是否为加锁者
void thread_rwlock_read_unlock(rwlock_t *rwlock);
// 写锁上锁
void thread_rwlock_write_lock(rwlock_t *rwlock);
// 写锁解锁，不会检查是否为加锁者
void thread_rwlock_write_unlock(rwlock_t *rwlock);
// 条件变量初始化
void thread_condvar_init(condvar_t *condvar);
// 条件变量销毁，不销毁会导致内存泄漏
void thread_condvar_destroy(condvar_t *condvar);
// 等待一个条件变量
void thread_condvar_wait(condvar_t *condvar, mutex_t *mutex);
// 唤醒一个等待该条件变量的线程
void thread_condvar_signal(condvar_t *condvar);
// 唤醒所有等待该条件变量的线程
void thread_condvar_broadcast(condvar_t *condvar);


#endif