/*
 * Name: Buffer.c
 * Description: This source file contains implementation of uart and systick buffer(queue) related function
 * Last modified Date: 2019-11-16
 */
#include "Buffer.h"

static volatile Queue_t queue[QUEUE_NUM];

/*
 * if queue is not full, write a char to the queue, inc the tail, inc the
 * length
 */
void enqueue(int q_num, char data)
{
    int head = queue[q_num].head;
    head = (head + 1) & (QUEUE_SZ - 1);

    if (q_num == UART_OUT)
    {
        if (!data_xmitting(data) && (head != queue[q_num].tail)) //uart busy transimitting, queuing
        {
            queue[q_num].data[head] = data;
            queue[q_num].head = head;
        }
    }
    else if (head != queue[q_num].tail)
    {
        queue[q_num].data[head] = data;
        queue[q_num].head = head;
    }

}

/*
 * If queue is not empty, pop a char out of the queue; inc the head, dec the
 * length; return the char. If empty, turn NULL
 */
char dequeue(int q_num)
{
    int tail = queue[q_num].tail;
    int data;

    if (queue[q_num].head == queue[q_num].tail) //queue empty
    {
        return EMPTY;
    }
    else    //dequeue
    {
        queue[q_num].tail = (tail + 1) & (QUEUE_SZ - 1);
        tail = queue[q_num].tail;
        data = queue[q_num].data[tail];
        return data;
    }
}
/* check is the queue empty */
bool isEmpty(int q_num)
{
    return queue[q_num].tail == queue[q_num].head;
}

/* check is the queue full */
bool isFull(int q_num)
{
    int head = queue[q_num].head;
    head = (head + 1) & (QUEUE_SZ - 1);

    return (head == queue[q_num].tail);
}

