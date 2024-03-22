#ifndef INCLUDE_USER_MODE_THREAD_H_
#define INCLUDE_USER_MODE_THREAD_H_

#define MAX_THREAD_NUM 128
#define DAFLULT_PRIORITY 15 // 默认优先级

// 初始化
void init();
// 创建线程，参数是线程主函数，成功返回线程id，失败返回-1
int thread_create(void (*thd_func)(), int priority);
// 线程睡眠，毫秒为单位，不会释放锁
void thread_sleep(unsigned long long msec);
// 等待另一个线程结束，会释放锁
void thread_wait(int tid);
// 互斥锁上锁，如已经上锁则阻塞
void thread_mutex_lock();
// 互斥锁解锁，只能由互斥锁持有者解开，非持有者无效
void thread_mutex_unlock();
// 申请一个信号量并初始化，成功返回信号量id，失败返回-1
int thread_sem_init(int sem_init);
// 释放信号量
void thread_sem_del(int sid);
// 信号量P操作，P操作失败则阻塞
void thread_sem_p(int sid);
// 信号量V操作
void thread_sem_v(int sid);

#endif