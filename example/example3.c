#include <stdio.h>
#include "../src/user_mode_thread.h"

int sem_id = -1;

void producer()
{
    for (int i = 0; i < 5; i++)
    {
        thread_sem_v(sem_id);
        printf("Producer production resources%d\n", i + 1);
        thread_sleep(1000);
    }
}

void consumer()
{
    for (int i = 0; i < 5; i++)
    {
        thread_sem_p(sem_id);
        printf("Consumers consume resources%d\n", i + 1);
    }
}

int main()
{
    init();
    sem_id = thread_sem_init(0);
    int tid1 = thread_create(producer, DAFLULT_PRIORITY);
    int tid2 = thread_create(consumer, DAFLULT_PRIORITY);
    thread_wait(tid1);
    thread_wait(tid2);
}