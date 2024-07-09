#include <stdio.h>
#include "../src/user_mode_thread.h"

void thread1()
{
    for (;;)
    {
        printf("thread1\n");
    }
}

void thread2()
{
    for (;;)
    {
        printf("thread2\n");
    }
}

int main()
{
    thread_main_init();
    thread_create(thread1, DAFLULT_PRIORITY);
    thread_create(thread2, DAFLULT_PRIORITY);
    for (;;)
    {
        printf("main\n");
    }
}
