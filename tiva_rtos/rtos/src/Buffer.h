/*
 * Name: Buffer.h
 * Description: This header file contains declaration of uart and systick buffer(queuqe) related function,and the struct of the buffers
 * Last modified Date: 2019-11-16
 */

#ifndef Queue_h
#define Queue_h

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#define QUEUE_SZ 8192 //queue size
#define QUEUE_NUM 3 //how many queues to create
#define UART_IN 0   //uart input queue index
#define UART_OUT 1  //uart output queue index
#define SYSTK_Q 2   //systick queue index
#define EMPTY 0 //queue empty

#define disable()   __asm(" cpsid i")
#define enable()    __asm(" cpsie i")

/*
 * struct of a circular queue, char * data for data storage
 */
typedef struct Queue Queue_t;
struct Queue
{
    char data[QUEUE_SZ];
    int head;   //head of the queue
    int tail;   //tail of the queue
};

/* write to the queue */
void enqueue(int q_num, char data);

/* read from the queue */
char dequeue(int q_num);

/* check is the queue empty */
bool isEmpty(int q_num);

/* check is the queue full */
bool isFull(int q_num);

/* check is the uart  busy or idle */
extern int data_xmitting(char data);

#endif /* Queue_h */
