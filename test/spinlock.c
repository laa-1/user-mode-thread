#include <stdio.h>
#include "../src/user_mode_thread.h"

spinlock_t spinlock;

void thread1()
{
    for (int i = 0; i < 5; i++)
    {
        thread_spinlock_lock(&spinlock);
        printf("thread1 occupy the spinlock lock\n");
        thread_sleep(1100);
        printf("thread1 release the spinlock lock\n");
        thread_spinlock_unlock(&spinlock);
        thread_sleep(1100);
    }
}

void thread2()
{
    for (int i = 0; i < 5; i++)
    {
        thread_spinlock_lock(&spinlock);
        printf("thread2 occupy the spinlock lock\n");
        thread_sleep(1200);
        printf("thread2 release the spinlock lock\n");
        thread_spinlock_unlock(&spinlock);
        thread_sleep(1200);
    }
}

int main()
{
    thread_main_init();
    thread_spinlock_init(&spinlock);
    int tid1 = thread_create(thread1, DAFLULT_PRIORITY);
    int tid2 = thread_create(thread2, DAFLULT_PRIORITY);
    for (int i = 0; i < 5; i++)
    {
        thread_spinlock_lock(&spinlock);
        printf("main occupy the spinlock lock\n");
        thread_sleep(1000);
        printf("main release the spinlock lock\n");
        thread_spinlock_unlock(&spinlock);
        thread_sleep(1000);
    }
    thread_join(tid1);
    thread_join(tid2);
}