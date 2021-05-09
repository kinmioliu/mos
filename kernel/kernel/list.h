#ifndef __LIST_H__
#define __LIST_H__

#include <type.h>

#define LIST_POS1 0x11011011
#define LIST_POS2 0x22022022

struct list_head {
    struct list_head *prev;
    struct list_head *next;
};

static inline void INIT_LIST_HEAD(struct list_head *head)
{
    head->next = head;
    head->prev = head;
}

static inline void INIT_LIST_NODE(struct list_head *node)
{
    node->prev = LIST_POS1;
    node->next = LIST_POS2;
}

static inline void __list_add(struct list_head *node, struct list_head *prev, struct list_head *next)
{
    prev->next = node;
    next->prev = node;
    node->next = next;
    node->prev = prev;
}

/* add head after the node */
static inline void list_add(struct list_head *node, struct list_head *head)
{
    __list_add(node, head, head->next);
    /*
    head->next->prev = node;
    node->next = head->next;
    head->next = node;
    node->prev = head;
    */
}

/* add head before the node */
static inline void list_add_tail(struct list_head *node, struct list_head *head)
{
    __list_add(node, head->prev, head);
    /*
    head->prev->next = node;
    node->prev = head->prev;
    node->next = head;
    head->prev = node;
    */
}

static inline void list_del(struct list_head *node)
{
    node->prev->next = node->next;
    node->next->prev = node->prev;
    INIT_LIST_NODE(node);
}

static inline bool list_empty(struct list_head *head)
{
    return head->next == head;
}

#define offsetof(TYPE, MEMBER) \
    &((TYPE *)0->MEMBER)

#define container_of(ptr, TYPE, MEMBER) \
    (char *)ptr - offsetof(TYPE, MEMBER)

#define list_entry(ptr, TYPE, MEMBER) \
    container_of(ptr, TYPE, MEMBER)


#define list_for_each(pos, head) \
    for (pos = pos->next; pos != head; pos = pos->next)


#endif
