#include <stdio.h>
#include <stdlib.h>
#include "linkqueue.h"

void linkqueue_init(linkqueue_t *linkqueue)
{
    linkqueue->begin = malloc(sizeof(node_t));
    linkqueue->begin->data = 0;
    linkqueue->begin->next = NULL;
    linkqueue->end = linkqueue->begin;
}

void linkqueue_destory(linkqueue_t *linkqueue)
{
    for (node_t *p = linkqueue->begin; p != NULL; p = p->next) {
        free(p);
    }
}

int linkqueue_empty(linkqueue_t *linkqueue)
{
    return linkqueue->begin == linkqueue->end;
}

int linkqueue_front(linkqueue_t *linkqueue)
{
    return linkqueue->begin->next->data;
}

void linkqueue_push_back(linkqueue_t *linkqueue, int data)
{
    linkqueue->end->next = malloc(sizeof(node_t));
    linkqueue->end->next->data = data;
    linkqueue->end->next->next = NULL;
    linkqueue->end = linkqueue->end->next;
}

void linkqueue_pop_front(linkqueue_t *linkqueue)
{
    node_t *p = linkqueue->begin->next;
    linkqueue->begin->next = linkqueue->begin->next->next;
    if (linkqueue->begin->next == NULL) {
        linkqueue->end = linkqueue->begin;
    }
    free(p);
}