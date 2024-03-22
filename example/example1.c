#include <stdio.h>
#include "../src/user_mode_thread.h"

void thread1()
{
    for (int i = 0; i < 500; i++)
    {
        printf("thread1\n");
        thread_sleep(700);
    }
}

void thread2()
{
    for (int i = 0; i < 500; i++)
    {
        printf("thread2\n");
        thread_sleep(800);
    }
}

void thread3()
{
    for (int i = 0; i < 500; i++)
    {
        printf("thread3\n");
        thread_sleep(900);
    }
}

void thread4()
{
    for (int i = 0; i < 500; i++)
    {
        printf("thread4\n");
        thread_sleep(1000);
    }
}

void thread5()
{
    for (int i = 0; i < 500; i++)
    {
        printf("thread5\n");
        thread_sleep(1100);
    }
}

void thread6()
{
    for (int i = 0; i < 500; i++)
    {
        printf("thread6\n");
        thread_sleep(1200);
    }
}

int main()
{
    init();
    int tid1 = thread_create(thread1, DAFLULT_PRIORITY);
    int tid2 = thread_create(thread2, DAFLULT_PRIORITY);
    int tid3 = thread_create(thread3, DAFLULT_PRIORITY);
    int tid4 = thread_create(thread4, DAFLULT_PRIORITY);
    int tid5 = thread_create(thread5, DAFLULT_PRIORITY);
    int tid6 = thread_create(thread6, DAFLULT_PRIORITY);
    for (int i = 0; i < 500; i++)
    {
        printf("main\n");
        thread_sleep(1300);
    }
    thread_wait(tid1);
    thread_wait(tid2);
    thread_wait(tid3);
    thread_wait(tid4);
    thread_wait(tid5);
    thread_wait(tid6);
}
