/*
 * Name: Proc_switch.h
 * Author: Yu Gao, Zhoubin Xu
 * Description: This header file contains declaration of process switch related funcitons
 * Last modified Date: 2019-11-21
 */
#ifndef PROCESS_H_
#define PROCESS_H_

#include "Queue.h"
#include "monitor.h"
#include <stdbool.h>

#define TRUE    1
#define FALSE   0
#define PRIVATE static

#define SVC()   __asm(" SVC #0")
#define disable()   __asm(" cpsid i")   //disable
#define enable()    __asm(" cpsie i")   //enable

#define MSP_RETURN 0xFFFFFFF9    //LR value: exception return using MSP as SP
#define PSP_RETURN 0xFFFFFFFD    //LR value: exception return using PSP as SP
#define THUMB   0x01000000
#define MAX_PCB_NUM 32  //max pcb number
#define STACKSIZE   256 //stack size
#define STKFRAME    16  //stack frame size
#define MAXMSG_SZ   64  //max msg size in mailbox
#define MAXMAIL_NUM 256     //max mail number
#define MSGQARR_SIZE 256    //number of msg queues
#define MAX_PID 99  //max pid value

extern struct pcb *running; //running process

void set_LR(volatile unsigned long);    //set link register
unsigned long get_PSP();    //get the psp value
void set_PSP(volatile unsigned long);
unsigned long get_MSP(void);    //get main sp value
void set_MSP(volatile unsigned long);   //set psp value
unsigned long get_SP(); //get psps value

void volatile save_registers(); //save registers to stack
void volatile restore_registers();  //restore registers to cpu
void PendSV_Handler(void);
void next_process(void);

//process state definition
enum proc_state
{
    RUNNING, WTR, BLOCKED
};

/* Cortex default stack frame */
struct stack_frame
{
    /* Registers saved by s/w (explicit) */
    /* There is no actual need to reserve space for R4-R11, other than
     * for initialization purposes.  Note that r0 is the h/w top-of-stack.
     */
    unsigned long r4;
    unsigned long r5;
    unsigned long r6;
    unsigned long r7;
    unsigned long r8;
    unsigned long r9;
    unsigned long r10;
    unsigned long r11;
    /* Stacked by hardware (implicit)*/
    unsigned long r0;
    unsigned long r1;
    unsigned long r2;
    unsigned long r3;
    unsigned long r12;
    unsigned long lr;
    unsigned long pc;
    unsigned long psr;
};

/*
 * msg structures used to pass to kernel
 */
struct pmsg
{
    int src;    //from whcih msg queue
    int dst;    //send to which msg queue
    int size;   //size of msg
    char * msg; //msg
};

/*
 * mail struct, store msg in msg queue
 */
struct mail
{
    struct mail * next;
    struct mail * prev;
    char msg[MAXMSG_SZ];    //msg
    int src;    //from which proc
    int msgsize;    //msg size
};

/*
 * msg queue
 */
struct msgq
{
    struct msgq * next;
    struct msgq * prev;
    struct pcb * pcbptr;    //msg queue owner
    struct mail * mailq;    //mail queue
};

/* Process control block */
struct pcb
{
    struct pcb *next;
    struct pcb *prev;
    void * stktop;  //top of stack
    enum proc_state state;  //state of process
    unsigned int prio;  //Priority
    unsigned int pid;   //id
    unsigned long sp;   //stack pointer
    struct pmsg * pmsgptr;  //msg arguments
    unsigned int blocked_mailbox;   //blocked by which msg queue
    struct msgq * msgqptr;  //pointer to the msg queue owned
    bool request_input; //requset input from screen
    struct window * windowptr;  //window pointer to the window of its
};

extern struct msgq msgq_arr[MSGQARR_SIZE];  //msg queue array, 256 msg queues
extern struct mail * maillist;  //list of mails
extern struct pcb * pcblist;    //list of pcbs

#endif /*PROCESS_H_*/
