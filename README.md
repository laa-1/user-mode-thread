# user-mode-thread 用户态线程

一个适用于 Linux x86-64 平台的用户态多线程库。

支持线程的 create、sleep、join、yield 操作。

支持互斥锁、自旋锁、读写锁、条件变量的各类操作。

## 环境

* Ubuntu 22.04.4 LST x86-64
* gcc 11.4.0

本项目使用了 _POSIX_SOURCE 宏以及 x86-64 汇编，需要注意平台是否支持。

## 使用

user_mode_thread.h 头文件中包含了所有的 api。

你可以在 test 目录中查看所有的示例。

```c
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
```

