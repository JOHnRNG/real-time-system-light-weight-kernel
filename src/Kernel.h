/*
 * Name: Kernel.h
 * Author: Yu Gao, Zhoubin Xu
 * Description: This header file contains declaration of kernel call
 * functions and kernel call argument structures
 * Last modified Date: 2019-11-21
 */

#ifndef KERNEL_H_
#define KERNEL_H_

#include "Queue.h"
#include "Proc_switch.h"
#include "monitor.h"


#define PROCESS 0
#define HANDLER 1

struct pcb *running;    //pcb of running process

enum kcodes //code passed to kernel when svc
{
    GETID, NICE, TERMINATE, SEND, RECV, BIND, UNBIND
};

/*
 * argument passed to kernel from process
 * code: kernel call type
 * rtnvalue: the value to be returned to process
 * arg: argument specific for each kernel call
 */
struct kcallargs
{
    enum kcodes code;
    int rtnvalue;
    unsigned long arg;
};

extern struct pcb * blockq; //head of block queue
extern void myitoa(int num, char *arr, int length); //convert integer to string
void kernel_init(void); //set pendsv to lowest priority
void terminate(void);   //process terminate
int getid(void);    //process kernel call, get the id of itself
int nice(unsigned int prio);    //process kernel call, request for new priority
int bind(int msgq_num);     //process kernel call, request for a msg queue
int unbind(int msgq_num);  //process kernel call, request for unbind a msg queue
int send(int dst, int src, char * msg, int size); //process kernel call, send a msg to a msg queue
int recv(int dst, int * src, char * msg, int size); //process kernel call, receive a msg from a msg queue
int k_send(struct pmsg * pmsgptr, int SRC); //kernel send a msg to a msg queue
int k_recv(struct pmsg * pmsgptr);  //kernel receive a msg from a msg queue

#endif /*KERNEL_H_*/
