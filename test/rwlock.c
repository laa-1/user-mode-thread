#include <stdio.h>
#include "../src/user_mode_thread.h"

int counter = 0;
rwlock_t rwlock;

void reader()
{
    for (;;)
    {
        thread_rwlock_read_lock(&rwlock);
        printf("counter: %d\n", counter);
        if (counter == 5)
        {
            thread_rwlock_read_unlock(&rwlock);
            break;
        }
        else
        {
            thread_rwlock_read_unlock(&rwlock);
            thread_sleep(200);
        }
    }
}

void writer()
{
    for (int i = 0; i < 5; i++)
    {
        thread_rwlock_write_lock(&rwlock);
        counter++;
        printf("counter + 1\n");
        thread_sleep(500);
        thread_rwlock_write_unlock(&rwlock);
        thread_sleep(500);
    }
}

int main()
{
    thread_main_init();
    thread_rwlock_init(&rwlock);
    int tid1 = thread_create(writer, DAFLULT_PRIORITY);
    int tid2 = thread_create(reader, DAFLULT_PRIORITY);
    int tid3 = thread_create(reader, DAFLULT_PRIORITY);
    thread_join(tid1);
    thread_join(tid2);
    thread_join(tid3);
}