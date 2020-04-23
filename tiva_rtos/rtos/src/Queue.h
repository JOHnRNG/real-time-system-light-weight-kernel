/*
 * Name: Queue.h
 * Author: Yu Gao, Zhoubin Xu
 * Description: This header file contains declaration of general queues(universal enqueue and dequeue)
 * Last modified Date: 2019-11-18
 */

#ifndef SRC_QUEUE_H_
#define SRC_QUEUE_H_

#include <stdio.h>

#define PQSIZE 5    //prio queues number
#define HIGHEST_PRIO 5  //highest prio

extern struct pcb * pqueue[PQSIZE]; //priority queues

/*
 * general structure used for enqueue and dequeue
 */
struct qitem
{
    struct qitem * next;
    struct qitem * prev;
};

void enq(struct qitem * new, struct qitem ** head); //general enqueue for doubly linked list
struct qitem * deq(struct qitem * ptr, struct qitem ** head); //general dequeue for doubly linked list

#endif /* SRC_QUEUE_H_ */
