#include <stdio.h>
#include "../src/user_mode_thread.h"

int buffer[10] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
int count = 0;
mutex_t mutex;
condvar_t condvar_producer;
condvar_t condvar_consumer;

void producer()
{
    for (int i = 0; i < 10; i++)
    {
        thread_mutex_lock(&mutex);
        while (count == 10)
        {
            thread_condvar_wait(&condvar_producer, &mutex);
        }
        buffer[count] = i;
        printf("product: %d\n", i);
        count++;
        thread_condvar_signal(&condvar_consumer);
        thread_mutex_unlock(&mutex);
        thread_sleep(200);
    }
}

void consumer()
{
    for (int i = 0; i < 10; i++)
    {
        thread_mutex_lock(&mutex);
        while (count == 0)
        {
            thread_condvar_wait(&condvar_consumer, &mutex);
        }
        printf("consumer: %d\n", buffer[count - 1]);
        count--;
        thread_condvar_signal(&condvar_producer);
        thread_mutex_unlock(&mutex);
        thread_sleep(500);
    }
}

int main()
{
    thread_main_init();
    thread_mutex_init(&mutex);
    thread_condvar_init(&condvar_producer);
    thread_condvar_init(&condvar_consumer);
    int tid1 = thread_create(producer, DAFLULT_PRIORITY);
    int tid2 = thread_create(producer, DAFLULT_PRIORITY);
    int tid3 = thread_create(producer, DAFLULT_PRIORITY);
    int tid4 = thread_create(consumer, DAFLULT_PRIORITY);
    int tid5 = thread_create(consumer, DAFLULT_PRIORITY);
    int tid6 = thread_create(consumer, DAFLULT_PRIORITY);
    thread_join(tid1);
    thread_join(tid2);
    thread_join(tid3);
    thread_join(tid4);
    thread_join(tid5);
    thread_join(tid6);
}