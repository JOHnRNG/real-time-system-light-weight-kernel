/*
 *  main.c
 *  4402 Assignment 2
 *  A light-weight messaging kernel
 *  Description:
 *  -Nine modules are printed for this assignment including their headers,CCS startup module is not printed
 *  -This module is our main entry which includes our Uart, time server, registers process and several test processes
 *  Author: Yu Gao, Zhoubin Xu
 *  Last modified Date: 2019-11-23
 */

#include "kernel.h"
#include "Proc_switch.h"
#include "Queue.h"
#include "monitor.h"
#include "UART.h"
#include "Systick.h"
#include "Time.h"
#include "tests.h"

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>

#define TIME_SERVER_ID  0 //TIME SERVER PID
#define INPUT_SERVER_ID  1 //UART SERVER PID
#define OUTPUT_SERVER_ID  2 //UART SERVER PID
#define IDLE_PROCESS_ID 3 //IDLE PROCESS PID

#define TIME_MSG_SZ 11 //max size for time sever msg
#define UART_MSG_SZ    80 //max size for uart msg

unsigned int ProcessNumber = 0; // process number count used to assign window

#define TEST

void time_server(void)
{ //time server
    bind(TIME_SERVER_MSGQ); //bind to time server MSGQ which is MSGQ 256

    char msg[TIME_MSG_SZ];
    int day;
    int src = -1; //SRC MAILBOX or systick handler

    while (1)
    {
        recv(TIME_SERVER_MSGQ, &src, msg, TIME_MSG_SZ); //Receive MSG from time server msgq

        if (src == SYSTICK_HANDLER) //src is systick input
        {
            update_time(TICK * 10, &day); //update time
            update_date(day); //update day
            decimalTime_to_charTime(&decimal_time, char_time); //change integer time to char
            send(OUTPUT_SERVER_MSGQ, TIME_SERVER_MSGQ, char_time, 11); //send to uart server to output the current time
        }
        else    //request from process
        {
            //get msg, check for command
            if (strcasecmp(msg, "TIME") == 0) //if msg from process is cmd "TIME"
            {
                decimalTime_to_charTime(&decimal_time, char_time);
                send(src, TIME_SERVER_MSGQ, char_time, TIME_MSG_SZ); //send the time to the requested process
            }
        }
    }
}

/*
 * function to decode the cmd, used in uart server
 */
void decode_cmd(char in_data)
{    //function to decode the cmd
    static String_t str;    //string to store the input string

    if (in_data == BACKSPACE)  //backspace
    {
        remove_char(&str);  //remove character
        send_char(in_data);  //send backspace
        update_cup(0, -1);  //update cup

    }
    else if (in_data == ENTER)  //Enter
    {
        int pid;
        char * msg = get_cmd(&str, ProcessNumber, &pid); //function to get the cmd, ProcessNumber and pid

        if (pid > 0)    //valid pid
        {
            //check blocked queue
            struct pcb * ptr = blockq;
            do
            {
                if ((ptr->pid == pid) && ptr->request_input)  //find the process
                {
                    send(ptr->blocked_mailbox, INPUT_SERVER_MSGQ, msg,
                    MAX_STR_SZ); //send to the blocked process's blocked mailbox
                    ptr->request_input = false; //clear request flag
                    break;
                }
                ptr = ptr->next;
            }
            while (ptr != blockq);
        }
        reset_str(&str); //clean string
        update_cup(1, -cup.c); //update cup
        send_cup(); //send the cup position, in this "enter" case we want it back to start of the line
        clean_input(MAX_COL); //clean the input cmd line
    }
    else if (in_data == TAB)    //Tab
    {
        send_char(in_data);    //send char
        add_char(&str, in_data);    //change string
        update_cup(0, 4);    //update cup position
    }
    else if (str.length < MAX_STR_SZ)   //character echo back
    {
        send_char(in_data);   //send char
        add_char(&str, in_data);   //change string
        update_cup(0, 1);   //update cup position
    }
}

void input_server(void)
{   //uart server
    bind(INPUT_SERVER_MSGQ);   //bind to uart server msgq

    char msg[UART_MSG_SZ];
    int src = -1;

    while (1)
    {
        recv(INPUT_SERVER_MSGQ, &src, msg, UART_MSG_SZ);
        decode_cmd(msg[0]);    //command entered from screen, decode it
    }
}

void output_server(void)
{
    bind(OUTPUT_SERVER_MSGQ);   //bind to uart server msgq

    char msg[UART_MSG_SZ];
    int src = -1;

    while (1)
    {
        recv(OUTPUT_SERVER_MSGQ, &src, msg, UART_MSG_SZ);
        msgq_arr[src - 1].pcbptr->request_input = true;
        send_to_window(msgq_arr[src - 1].pcbptr->windowptr, msg, NEW_LINE); //send a msg to the window related to the process
    }
}

void idle_proc(void)
{   //idle process which send numbers to putty through uart server
    bind(29);   //bind to mailbox 29
    char msg[5];
    int i = 0;
    while (1)
    {
        int j = 0;
        while (j < 100000)
        {
            j++;   //create delay
        }
        myitoa(i, msg, 5);   //change integer to array
        send(OUTPUT_SERVER_MSGQ, 29, msg, 8);  //send to uart server message queue
        i++;
    }
}

void reg_mails(void)
{  //CREATE 1024 mails(more like envelope)
    int i = 0;
    while (i < MAXMAIL_NUM)
    {
        struct mail * Mail = (struct mail *) malloc(sizeof(struct mail)); //create 1024 mails in total
        enq((struct qitem *) Mail, (struct qitem **) (&maillist)); //enqueue to mail list
        i++;
    }
}

void reg_pcbs(void)
{ //create 32 pcbs
    int i = 0;
    while (i < MAX_PCB_NUM)
    {
        struct pcb * new_pcb = (struct pcb *) malloc(sizeof(struct pcb)); //create 32 pcbs in total
        enq((struct qitem *) new_pcb, (struct qitem **) (&pcblist)); //enqueue to pcb list

        i++;
    }
}

/*
 * function to register a process
 */
int reg_proc(void (*fptr)(void), unsigned int pid, unsigned int priority,
bool req_window)
{
    struct pcb * new_pcb = pcblist;
    void * stktop;

    deq((struct qitem *) new_pcb, (struct qitem **) (&pcblist)); //get a pcb from list

    if (new_pcb)    //successfully get a pcb
    {
        stktop = malloc(STACKSIZE * sizeof(char));

        if (!stktop) //not enough memo
        {
            enq((struct qitem *) new_pcb, (struct qitem **) (&pcblist)); //deallocate
            return -2;
        }
    }
    else
        return -1;  //no pcb left

    unsigned long sp = (unsigned long) stktop + STACKSIZE;  //stack pointer

    struct stack_frame * stkframe = (struct stack_frame *) (sp
            - sizeof(struct stack_frame));
    void (*tptr)(void) = &terminate; //terminate function address
    stkframe->lr = (unsigned long) tptr; //LR get terminate function address
    stkframe->pc = (unsigned long) fptr; //entry address of the process
    stkframe->psr = THUMB; //THUMB MODE
    stkframe->r0 = 0; //initialize registers
    stkframe->r1 = 0;
    stkframe->r2 = 0;
    stkframe->r3 = 0;
    stkframe->r4 = 0;
    stkframe->r5 = 0;
    stkframe->r6 = 0;
    stkframe->r7 = 0;
    stkframe->r8 = 0;
    stkframe->r9 = 0;
    stkframe->r10 = 0;
    stkframe->r11 = 0;
    stkframe->r12 = 0;

    sp = (unsigned long) stkframe; //update SP

    /*INITILIZE PCB*/
    new_pcb->next = NULL;
    new_pcb->prev = NULL;
    new_pcb->pid = pid;
    new_pcb->sp = sp;
    new_pcb->stktop = stktop;
    new_pcb->state = WTR;
    new_pcb->prio = priority;
    new_pcb->blocked_mailbox = 0;
    new_pcb->msgqptr = NULL;
    new_pcb->pmsgptr = NULL;
    new_pcb->request_input = false;
    new_pcb->windowptr = NULL;

    enq((struct qitem *) new_pcb, (struct qitem **) (&pqueue[priority - 1])); //enqueue to priority queue

    if ((running && (running->prio <= new_pcb->prio)) || (!running))
    { //determine the first running process
        running = new_pcb;
    }

    if (req_window)
    {
        ProcessNumber++; //increate process number, used for assign window
        assign_window(new_pcb); //assign window
    }

    return 0;
}

void main(void)

{
    UART0_setup();  //set UART

    kernel_init();  //initial kernel

    reg_windows();   //register a window for each process

    reg_pcbs(); //register 32 pcbs

    reg_mails(); //register 1024 envelopes

    /*Register idle process,Uart and time server*/
    reg_proc(&idle_proc, 3, 1, true);
    reg_proc(&input_server, 2, 5, false);
    reg_proc(&output_server, 1, 5, false);
    reg_proc(&time_server, TIME_SERVER_ID, 5, true);

    seg_windows(ProcessNumber); //assign the coordinates for each window

    InterruptMasterEnable();      // Enable Master (CPU) Interrupts
    send_cup(); //send cup position
    SVC();
}


