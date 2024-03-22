# User Mode Thread 用户态线程

一个适用于 x86-64 的简单的用户态线程库，用于学习线程的上下文切换原理。

目前已经实现了线程的创建、sleep、wait 功能，并提供了互斥锁和信号量的操作。

## 环境

* Ubuntu-22.04 x86-64
* GCC-11.2.0

本项目使用了 _POSIX_SOURCE 宏以及 x86-64 汇编，需要注意平台是否符合条件。

## 使用

user_mode_thread.h 头文件包含了其所有的api，你可以引用后进行使用。

下面是一个简单的示例

```c
void thread1()
{
    for (int i = 0; i < 500; i++)
    {
        printf("thread1\n");
        thread_sleep(1000);
    }
}
int main()
{
    init(); // 必须要先进行初始化
    int tid1 = thread_create(thread1, DAFLULT_PRIORITY);
    for (int i = 0; i < 500; i++)
    {
        printf("main\n");
        thread_sleep(500);
    }
    thread_wait(tid1);
}
```

你也可以在 example 目录查看已有的 3 个示例。

## 注意事项

本项目仅用于学习交流，不适合用于生产环境。
