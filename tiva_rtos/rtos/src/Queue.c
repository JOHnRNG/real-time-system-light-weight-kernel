/*
 * Name: Queue.c
 * Author: Yu Gao, Zhoubin Xu
 * Description: This header file contains implementation of general queues(universal enqueue and dequeue)
 * Last modified Date: 2019-11-18
 */

#include "Proc_switch.h"
#include "Queue.h"

struct pcb * pqueue[PQSIZE] = { NULL };

/*
 * general enqueue for doubly linked list
 */
void enq(struct qitem * new, struct qitem ** head)
{
    if (!(*head)) //queue empty
    {
        *head = new;

        (*head)->next = new;
        (*head)->prev = new;
    }
    else    //not empty
    {
        new->next = *head;  //set next of new to head
        new->prev = (*head)->prev;
        (*head)->prev->next = new;
        (*head)->prev = new;

        *head = new;
    }
}

/*
 * general dequeue for doubly linked list
 */
struct qitem * deq(struct qitem * ptr, struct qitem ** head)
{
    if (!*head) //empty queue
        return NULL;

    if (*head == (*head)->next) //one item in queue
        *head = NULL;
    else if (ptr == *head)  //the item to be dequeued is teh head of queue
    {
        *head = ptr->next;  //set head to next
        ptr->prev->next = ptr->next;
        ptr->next->prev = ptr->prev;
    }
    else    //break the link
    {
        ptr->prev->next = ptr->next;
        ptr->next->prev = ptr->prev;
    }

    ptr->next = ptr;
    ptr->prev = ptr;

    return ptr;
}

