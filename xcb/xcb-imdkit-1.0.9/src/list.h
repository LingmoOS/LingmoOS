/*
 * SPDX-FileCopyrightText: 2014 Weng Xuetian <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only
 *
 */

#ifndef _LINK_LIST_H_
#define _LINK_LIST_H_

#include <stdlib.h>

#define list_container_of(ptr, type, member)                                   \
    ((type *)(((void *)(ptr)) - offsetof(type, member)))

typedef struct _list_head {
    struct _list_head *prev;
    struct _list_head *next;
} list_head;

static inline void list_init(list_head *pos) {
    pos->next = pos;
    pos->prev = pos;
}

static inline bool list_is_empty(list_head *pos) { return pos->next == pos; }

static inline void __list_insert_between(list_head *add, list_head *prev,
                                         list_head *next) {
    next->prev = add;
    prev->next = add;
    add->next = next;
    add->prev = prev;
}

static inline void list_prepend(list_head *add, list_head *pos) {
    __list_insert_between(add, pos, pos->next);
}

static inline void list_append(list_head *add, list_head *pos) {
    __list_insert_between(add, pos->prev, pos);
}

static inline void list_remove(list_head *pos) {
    list_head *next = pos->next;
    list_head *prev = pos->prev;
    prev->next = next;
    next->prev = prev;

    pos->next = NULL;
    pos->prev = NULL;
}

#define list_foreach(key, head)                                                \
    for (list_head *key = (head)->next; key != (head); key = key->next)

#define list_foreach_safe(key, head)                                           \
    for (list_head *key = (head)->next, *__n = key->next; key != (head);       \
         key = __n, __n = key->next)

#define list_foreach_nl(key, head)                                             \
    for (key = (head)->next; key != (head); key = key->next)

#define list_foreach_safe_nl(key, __n, head)                                   \
    for (key = (head)->next, __n = key->next; key != (head);                   \
         key = __n, __n = key->next)

#define list_entry_foreach(key, type, head, field)                             \
    for (type *key = list_container_of((head)->next, type, field);             \
         &key->field != (head);                                                \
         key = list_container_of(key->field.next, type, field))

#define list_entry_foreach_nl(key, type, head, field)                          \
    for (key = list_container_of((head)->next, type, field);                   \
         &key->field != (head);                                                \
         key = list_container_of(key->field.next, type, field))

#define list_entry_foreach_safe(key, type, head, field)                        \
    for (type *key = list_container_of((head)->next, type, field),             \
              *__n = list_container_of(key->field.next, type, field);          \
         &key->field != (head);                                                \
         key = __n, __n = list_container_of(key->field.next, type, field))

#define list_entry_foreach_safe_nl(key, __n, type, head, field)                \
    for (key = list_container_of((head)->next, type, field),                   \
        __n = list_container_of(key->field.next, type, field);                 \
         &key->field != (head);                                                \
         key = __n, __n = list_container_of(key->field.next, type, field))

#endif
