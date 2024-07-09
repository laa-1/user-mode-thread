#include <stdio.h>
#include "../src/user_mode_thread.h"

mutex_t mutex;

void thread1()
{
    for (int i = 0; i < 5; i++)
    {
        thread_mutex_lock(&mutex);
        printf("thread1 occupy the mutex lock\n");
        thread_sleep(1100);
        printf("thread1 release the mutex lock\n");
        thread_mutex_unlock(&mutex);
        thread_sleep(1100);
    }
}

void thread2()
{
    for (int i = 0; i < 5; i++)
    {
        thread_mutex_lock(&mutex);
        printf("thread2 occupy the mutex lock\n");
        thread_sleep(1200);
        printf("thread2 release the mutex lock\n");
        thread_mutex_unlock(&mutex);
        thread_sleep(1200);
    }
}

int main()
{
    thread_main_init();
    thread_mutex_init(&mutex);
    int tid1 = thread_create(thread1, DAFLULT_PRIORITY);
    int tid2 = thread_create(thread2, DAFLULT_PRIORITY);
    for (int i = 0; i < 5; i++)
    {
        thread_mutex_lock(&mutex);
        printf("main occupy the mutex lock\n");
        thread_sleep(1000);
        printf("main release the mutex lock\n");
        thread_mutex_unlock(&mutex);
        thread_sleep(1000);
    }
    thread_join(tid1);
    thread_join(tid2);
}