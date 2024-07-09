#include <stdio.h>
#include "../src/user_mode_thread.h"

void thread1()
{
    for (int i = 0; i < 100; i++)
    {
        printf("thread1\n");
        thread_sleep(110);
    }
}

void thread2()
{
    for (int i = 0; i < 100; i++)
    {
        printf("thread2\n");
        thread_sleep(120);
    }
}

int main()
{
    thread_main_init();
    int tid1 = thread_create(thread1, DAFLULT_PRIORITY);
    int tid2 = thread_create(thread2, DAFLULT_PRIORITY);
    for (int i = 0; i < 100; i++)
    {
        printf("main\n");
        thread_sleep(100);
    }
    thread_join(tid1);
    thread_join(tid2);
}
