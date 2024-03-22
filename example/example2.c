#include <stdio.h>
#include "../src/user_mode_thread.h"

void thread1()
{
    for (int i = 0; i < 5; i++)
    {
        thread_mutex_lock();
        printf("thread1 occupy the mutex lock\n");
        thread_sleep(1000);
        printf("thread1 release the mutex lock\n");
        thread_mutex_unlock();
        thread_sleep(1);
    }
}

void thread2()
{
    for (int i = 0; i < 5; i++)
    {
        thread_mutex_lock();
        printf("thread2 occupy the mutex lock\n");
        thread_sleep(1100);
        printf("thread2 release the mutex lock\n");
        thread_mutex_unlock();
        thread_sleep(1);
    }
}

int main()
{
    init();
    int tid1 = thread_create(thread1, DAFLULT_PRIORITY);
    int tid2 = thread_create(thread2, DAFLULT_PRIORITY);
    for (int i = 0; i < 5; i++)
    {
        thread_mutex_lock();
        printf("main occupy the mutex lock\n");
        thread_sleep(1200);
        printf("main release the mutex lock\n");
        thread_mutex_unlock();
        thread_sleep(1);
    }
    thread_wait(tid1);
    thread_wait(tid2);
}