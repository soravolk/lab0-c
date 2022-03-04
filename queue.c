#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "harness.h"
#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

/*
 * Create empty queue.
 * Return NULL if could not allocate space.
 */
struct list_head *q_new()
{
    struct list_head *q_head = malloc(sizeof(struct list_head));
    if (!q_head) {
        return NULL;
    }
    INIT_LIST_HEAD(q_head);
    return q_head;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (!l) {
        return;
    }
    struct list_head *node, *safe;
    list_for_each_safe (node, safe, l) {
        list_del_init(node);
        q_release_element(list_entry(node, element_t, list));
    }
    free(node);
}

/*
 * Attempt to insert element at head of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head) {
        return false;
    }
    element_t *q_element = malloc(sizeof(element_t));
    if (!q_element) {
        return false;
    }
    /* assign value */
    size_t len = strlen(s);
    q_element->value = malloc(len + 1);
    if (!q_element->value) {
        free(q_element);
        return false;
    }
    memcpy(q_element->value, s, len);
    q_element->value[len] = '\0';

    /* initialize and add a new node */
    list_add(&q_element->list, head);
    return true;
}

/*
 * Attempt to insert element at tail of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head) {
        return false;
    }
    element_t *q_element = malloc(sizeof(element_t));
    if (!q_element) {
        return false;
    }
    // assign value
    size_t len = strlen(s);
    q_element->value = malloc(len + 1);
    if (!q_element->value) {
        free(q_element);
        return false;
    }
    memcpy(q_element->value, s, len);
    q_element->value[len] = '\0';
    /* initialize and add a new node */
    list_add_tail(&q_element->list, head);
    return true;
}

/*
 * Attempt to remove element from head of queue.
 * Return target element.
 * Return NULL if queue is NULL or empty.
 * If sp is non-NULL and an element is removed, copy the removed string to *sp
 * (up to a maximum of bufsize-1 characters, plus a null terminator.)
 *
 * NOTE: "remove" is different from "delete"
 * The space used by the list element and the string should not be freed.
 * The only thing "remove" need to do is unlink it.
 *
 * REF:
 * https://english.stackexchange.com/questions/52508/difference-between-delete-and-remove
 */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || head->next == head) {
        return NULL;
    }
    element_t *ele = list_entry(head->next, element_t, list);
    if (sp) {
        size_t size = (strlen(ele->value) < bufsize - 1) ? strlen(ele->value)
                                                         : bufsize - 1;
        memcpy(sp, ele->value, size);
        sp[size] = '\0';
    }
    list_del_init(head->next);
    return ele;
}

/*
 * Attempt to remove element from tail of queue.
 * Other attribute is as same as q_remove_head.
 */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || head->next == head) {
        return NULL;
    }
    element_t *ele = list_entry(head->prev, element_t, list);
    if (sp) {
        size_t size = (strlen(ele->value) < bufsize - 1) ? strlen(ele->value)
                                                         : bufsize - 1;
        memcpy(sp, ele->value, size);
        sp[size] = '\0';
    }
    list_del_init(head->prev);
    return ele;
}

/*
 * WARN: This is for external usage, don't modify it
 * Attempt to release element.
 */
void q_release_element(element_t *e)
{
    free(e->value);
    free(e);
}

/*
 * Return number of elements in queue.
 * Return 0 if q is NULL or empty
 */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;

    int len = 0;
    struct list_head *li;

    list_for_each (li, head)
        len++;
    return len;
}

struct list_head *find_mid(struct list_head *head)
{
    struct list_head *slow, *fast;
    for (slow = fast = head->next; fast != head && fast->next != head;
         slow = slow->next, fast = fast->next->next)
        ;

    return slow;
}
/*
 * Delete the middle node in list.
 * The middle node of a linked list of size n is the
 * ⌊n / 2⌋th node from the start using 0-based indexing.
 * If there're six element, the third member should be return.
 * Return true if successful.
 * Return false if list is NULL or empty.
 */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (!head || head->next == head) {
        return false;
    }

    struct list_head *mid = find_mid(head);

    list_del_init(mid);
    q_release_element(list_entry(mid, element_t, list));

    return true;
}

/*
 * Delete all nodes that have duplicate string,
 * leaving only distinct strings from the original list.
 * Return true if successful.
 * Return false if list is NULL.
 *
 * Note: this function always be called after sorting, in other words,
 * list is guaranteed to be sorted in ascending order.
 */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (!head || head->next == head) {
        return false;
    }
    struct list_head **new_list = &head->next;
    struct list_head *cur = head->next;
    while (cur != head && cur->next != head) {
        if (!strcmp(list_entry(cur, element_t, list)->value,
                    list_entry(cur->next, element_t, list)->value)) {
            char *str = list_entry(cur, element_t, list)->value;
            cur = cur->next->next;
            while (cur != head &&
                   !strcmp(list_entry(cur, element_t, list)->value, str)) {
                cur = cur->next;
            }
        } else {
            *new_list = cur;
            cur = cur->next;
            new_list = &(*new_list)->next;
        }
    }
    *new_list = cur;

    return true;
}

/*
 * Attempt to swap every two adjacent nodes.
 */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/

    struct list_head **node = &(head->next);
    struct list_head *cur, *temp;
    cur = head->next;

    while (cur != head && cur->next != head) {
        temp = cur;
        cur = cur->next->next;
        temp->next->prev = (*node)->prev;
        *node = temp->next;
        (*node)->next = temp;
        temp->prev = *node;
        node = &((*node)->next->next);
    }
    cur->prev = (*node)->prev;
    *node = cur;
}

/*
 * Reverse elements in queue
 * No effect if q is NULL or empty
 * This function should not allocate or free any list elements
 * (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
 * It should rearrange the existing ones.
 */
void q_reverse(struct list_head *head)
{
    if (!head || head->next == head)
        return;
    struct list_head *temp, *cur;
    cur = head->next;
    while (cur != head) {
        temp = cur->next;
        cur->next = cur->prev;
        cur->prev = temp;
        cur = temp;
    }
    temp = head->next;
    head->next = head->prev;
    head->prev = temp;
}

void merge_sort(struct list_head **head)
{
    if (!*head || !(*head)->next)
        return;
    struct list_head *l1, *l2, *temp;

    for (l1 = l2 = *head; l2 && l2->next; l1 = l1->next, l2 = l2->next->next)
        ;
    l2 = l1->next;
    l1->next = NULL;
    l1 = *head;

    merge_sort(&l1);
    merge_sort(&l2);

    struct list_head **new_head = &l1;
    while (l1 && l2) {
        temp = (*new_head)->prev;
        if (strcmp(list_entry(l1, element_t, list)->value,
                   list_entry(l2, element_t, list)->value) < 0) {
            *new_head = l1;
            l1 = l1->next;
        } else {
            *new_head = l2;
            l2 = l2->next;
        }
        (*new_head)->prev = temp;
        new_head = &(*new_head)->next;
    }
    *new_head = l1 ? l1 : l2;
}
/*
 * Sort elements of queue in ascending order
 * No effect if q is NULL or empty. In addition, if q has only one
 * element, do nothing.
 */
void q_sort(struct list_head *head)
{
    if (!head || head->next == head || head->next->next == head)
        return;
    head->prev->next = NULL;
    merge_sort(&head->next);
    struct list_head *cur = head->next;
    for (; cur->next; cur = cur->next)
        ;
    cur->next = head;
    head->prev = cur;
}
