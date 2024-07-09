#ifndef INCLUDE_LINKQUEUE_H_
#define INCLUDE_LINKQUEUE_H_

typedef struct node_s
{
    int data;
    struct node_s *next;
} node_t;

typedef struct
{
    node_t *begin;
    node_t *end;
} linkqueue_t;

// 必须要初始化
void linkqueue_init(linkqueue_t *linkqueue);

// 使用完必须销毁，否则会内存泄漏
void linkqueue_destory(linkqueue_t *linkqueue);

int linkqueue_empty(linkqueue_t *linkqueue);

// 不会判空，需要队列不为空
int linkqueue_front(linkqueue_t *linkqueue);

// 不会判空，需要队列不为空
// 不会深拷贝数据
void linkqueue_push_back(linkqueue_t *linkqueue, int data);

// 不会判空，需要队列不为空
void linkqueue_pop_front(linkqueue_t *linkqueue);

#endif