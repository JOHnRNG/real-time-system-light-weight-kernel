/*
 * Name: Kernel.c
 * Author: Yu Gao, Zhoubin Xu
 * Description:
 * -This source file contains implementation of kernel call
 * -several functions taken and modified from eced 4402 course brightspace
 * Last modified Date: 2019-11-23
 */

#include "kernel.h"
#include "Proc_switch.h"
#include <stdio.h>
#include "Systick.h"

struct pcb * blockq = NULL;     //block queue first is NULL
bool kernel = false;

/*
 * init pendsv to lowest priority
 */
void kernel_init(void)
{
    SYS_PRI_2_R = SVC_HIGHEST_PRI;
    NVIC_SYS_PRI13_R |= PENDSV_LOWEST_PRIORITY;
}

/* Assign 'data' to R7; since the first argument is R0, this is
 * simply a MOV from R0 to R7
 */
void assignR7(volatile unsigned long data)
{
    __asm(" mov r7,r0");
}

/*
 Process-kernel call function.  Supplies code and kernel message to the
 kernel is a kernel request.  Returns the result (return code) to the caller.
 */
int pkcall(enum kcodes code, unsigned long arg)
{
    volatile struct kcallargs arglist;
    /* Pass code and pkmsg to kernel in arglist structure */
    arglist.code = code;
    arglist.arg = arg;

    /* R7 = address of arglist structure */
    assignR7((unsigned long) &arglist);
    /* Call kernel */
    SVC();
    /* Return result of request to caller */
    return arglist.rtnvalue;
}

/*
 * process kernel call, get the id of itself
 */
int getid(void)
{
    return pkcall(GETID, NULL);
}

/*
 * process kernel call, request for new priority
 */
int nice(unsigned int prio)
{
    if (prio == running->prio)
        return prio;
    else if ((prio < 1) || (prio > HIGHEST_PRIO))
        return -1;
    else
        return pkcall(NICE, prio);
}

/*
 * change the priority of the running process
 */
void k_nice(unsigned long prio)
{
    send_to_window(running->windowptr, "Nice", NEW_LINE); //output to screen to indicate nice
    deq((struct qitem *) running,  //dequeue running from current priority queue
        (struct qitem **) (&pqueue[running->prio - 1]));
    enq((struct qitem *) running, (struct qitem **) (&pqueue[prio - 1])); //enqueue to new priority

    if (running->prio > prio) //move to lower priority
    {
        running->sp = get_PSP();    //store running sp

        running->prio = prio;   //store new priority
        int i = HIGHEST_PRIO;
        while (!pqueue[i - 1])  //walk through priority from high to low
            i--;
        running = pqueue[i - 1];    //a process found from higher priority

        set_PSP(running->sp);   //set psp
    }
    else if (running->prio < prio)   //move to higher priority
    {
        running->prio = prio;   //set new priority
    }
}

/*
 * process terminate function
 */
void terminate(void)
{
    pkcall(TERMINATE, NULL);
}

/*
 * change running pcb from same or highr priority
 */
void change_running(struct pcb * pcbptr)
{
    if (running == running->next)   //only one pcb in this priority
    {
        deq((struct qitem *) running,   //dequeue running from prio queue
            (struct qitem **) (&pqueue[running->prio - 1]));
        int i = running->prio - 2;  //current priority
        while (pqueue[i] == NULL)   //walk through priority queues
            i--;

        running = pqueue[i];    //set running to pcb in this priority
    }
    else    //more pcb in this priority
    {
        running = running->next;    //set running to next in this prio
        deq((struct qitem *) pcbptr,    //dequeue pcb from prio queue
            (struct qitem **) (&pqueue[pcbptr->prio - 1]));
    }
}

/*
 * return mails back to the mail list
 */
void return_mail(struct mail * mptr, struct msgq * ptr)
{
    if (mptr)   //mail exist
    {
        struct mail * tmp = mptr;
        do
        {
            mptr = tmp;
            tmp = tmp->next;

            deq((struct qitem *) mptr, (struct qitem **) &ptr->mailq); //dequeue mail from mail queue
            enq((struct qitem *) mptr, (struct qitem **) &maillist); //enqueue mail to mail list

            int i;
            for (i = 0; i < MAXMSG_SZ; i++)   //clear the msg
            {
                mptr->msg[i] = NULL;
            }

            mptr->msgsize = 0;  //clear msg size
            mptr->src = 0;  //clear msg source

        }
        while (ptr->mailq);
    }
}

/*
 * terminate process, return pcb to pcb list, return mails to mail list
 */
void k_terminate(void)
{
    send_to_window(running->windowptr, "Terminate", !NEW_LINE); //output to indicate terminate

    if (running->msgqptr) //msgq exist
    {
        struct msgq * ptr = running->msgqptr;   //msgqs of running
        struct msgq * tmpptr = ptr;
        do
        {
            ptr = tmpptr;
            tmpptr = tmpptr->next;

            //dequeue masg queue from running pcb's msg queue list
            deq((struct qitem *) ptr, (struct qitem **) &running->msgqptr);
            ptr->pcbptr = NULL; //set msg queue pcb pointer to NULL

            struct mail * mptr = ptr->mailq;
            return_mail(mptr, ptr); //return mails to mail list
        }
        while (running->msgqptr);
    }

    struct pcb * pcbptr = running;

    change_running(pcbptr); //change running process

    enq((struct qitem *) pcbptr, (struct qitem **) &pcblist); //enqueue the pcb back to pcb list

    set_PSP(running->sp);   //set new PSP
}

/*
 * process request bind to a msg queue
 */
int bind(int msgq_num)
{
    if ((msgq_num < 0) || (msgq_num > MSGQARR_SIZE))
        return -1;  //invalid msgq number
    else
        return pkcall(BIND, msgq_num);  //pass args to kernel
}

/*
 * bind a msg queue to the process
 */
int k_bind(int msgq_num)
{
    if (msgq_num == 0)  //bind any msg queue
    {
        int i;
        for (i = 0; i < MSGQARR_SIZE; i++)  //find a empty msg queue
        {
            if (msgq_arr[i].pcbptr == NULL) //empty queue found
            {
                msgq_arr[i].pcbptr = running;   //set msg queue owner

                enq((struct qitem *) (&msgq_arr[i]), //enqueue msg queue to msgq list of running
                    (struct qitem **) (&running->msgqptr));

                return (i + 1); //return msg queue number
            }
        }
        return -3;  //all msgq are occupied
    }
    else if (msgq_arr[msgq_num - 1].pcbptr != NULL)
    {
        return -2;  //msgq occupied
    }
    else    //specific msg queue empty
    {
        msgq_arr[msgq_num - 1].pcbptr = running;    //set msg queue owner

        enq((struct qitem *) (&msgq_arr[msgq_num - 1]), //enqueue msg queue to msgq list of running
            (struct qitem **) (&running->msgqptr));

        return msgq_num;    //msg queue number
    }
}

/*
 * process request for unbind a msg queue
 */
int unbind(int msgq_num)
{
    if ((msgq_num < 1) || (msgq_num > MSGQARR_SIZE))
        return -1;  //invalid msgq number
    else if (msgq_arr[msgq_num - 1].pcbptr != running)
        return -2;  //msgq does not belong to itself
    else
        return pkcall(UNBIND, msgq_num);    //sned request to kernel
}

/*
 * unbind the msg queue from the running process
 */
int k_unbind(int msgq_num)
{
    struct mail * ptr = msgq_arr[msgq_num - 1].mailq;
    msgq_arr[msgq_num - 1].pcbptr = NULL;   //set the msgq owner to null

    return_mail(ptr, &msgq_arr[msgq_num - 1]);  //return the mails

    deq((struct qitem *) (&msgq_arr[msgq_num - 1]),
        (struct qitem **) (&running->msgqptr)); //dequeue from the pcb msg queue list

    return msgq_num;    //msg queue number

}

/*
 * process request to send a msg to a msg queue
 */
int send(int dst, int src, char * msg, int size)
{

    if (((src < 1) || (src > MSGQARR_SIZE))
            || msgq_arr[src - 1].pcbptr != running)
        return -2; //invalid src
    else if ((dst < 1) || (dst > MSGQARR_SIZE)
            || msgq_arr[dst - 1].pcbptr == NULL)
        return -1;  //invalid dst
    else if (!msg)
        return -3;  //invalid msg
    else if ((size < 1) && (size > MAXMSG_SZ))
        return -4;  //invalid size
    else    //valid msg
    {
        struct pmsg args;
        args.dst = dst;
        args.src = src;
        args.msg = msg;
        args.size = size;

        return pkcall(SEND, (unsigned long) (&args)); //send request to kernel
    }
}

/*
 * sned the msg to the destination msg queue
 */
int k_send(struct pmsg * pmsgptr, int SRC)
{
    int dst, src, sz;

    dst = pmsgptr->dst;
    src = pmsgptr->src;
    sz = pmsgptr->size;

    struct pcb * pcbptr = msgq_arr[dst - 1].pcbptr;
    if ((pcbptr->state == BLOCKED) && (pcbptr->blocked_mailbox == dst))
    //receiver blocked
    {
        deq((struct qitem *) pcbptr, (struct qitem **) (&blockq)); //Dequeue the pcb from block queue
        enq((struct qitem *) pcbptr,    //enqueue the pcb back to prio queue
            (struct qitem **) (&pqueue[pcbptr->prio - 1]));

        pcbptr->state = WTR;    //change state back to wtr

        int i = 0;
        while ((i < pcbptr->pmsgptr->size) && (i < pmsgptr->size)) //copy the msg to process
        {
            pcbptr->pmsgptr->msg[i] = pmsgptr->msg[i];
            i++;
        }

        pcbptr->pmsgptr->msg[i] = '\0';
        pcbptr->pmsgptr->src = src; //set the msg source

        if ((pcbptr->prio >= running->prio) && (SRC == PROCESS)) //the reusmed process has higher prio
        {
            running->sp = get_PSP();    //save sp
            running = pcbptr;   //change running
            set_PSP(running->sp);   //set psp
        }
        else if ((pcbptr->prio >= running->prio) && (SRC == HANDLER)) //the reusmed process has higher prio
        {
            //NVIC_INT_CTRL_R |= CLEAR_PENDSV;    //disable pendsv
            save_registers();   //save registers
            running->sp = get_PSP();    //save sp
            running = pcbptr;   //change running
            set_PSP(running->sp);   //set psp
            restore_registers();
        }
    }
    else    //receiver not blocked
    {
        struct mail * mailptr = (struct mail *) deq((struct qitem *) maillist, //get a mail from the list
                (struct qitem **) (&maillist));

        if (!mailptr)   //no mail available
            return -5;

        mailptr->src = src; //set source
        mailptr->msgsize = sz;  //set size
        int i = 0;
        while (i < sz) //copy the msg to mailbox
        {
            mailptr->msg[i] = pmsgptr->msg[i];
            i++;
        }

        enq((struct qitem *) mailptr,
            (struct qitem **) (&msgq_arr[dst - 1].mailq)); //enqueue the mail
    }
    return src;
}

/*
 * process request to receive a mesg at a msg queue
 */
int recv(int dst, int * src, char * msg, int size)
{
    if ((dst < 1) || (dst > MSGQARR_SIZE)
            || msgq_arr[dst - 1].pcbptr != running)
        return -1;  //invalid dst
    else if ((size < 1) && (size > MAXMSG_SZ))
        return -2;  //invalid size
    else    //valid dst and size
    {
        struct pmsg args;
        args.dst = dst;
        args.msg = msg;
        args.size = size;

        //receive the msg
        int received = pkcall(RECV, (unsigned long) (&args));

        *src = args.src;    //get the src

        return received;
    }
}

/*
 * kernel receive a msg from the msg queue
 */
int k_recv(struct pmsg * pmsgptr)
{
    int dst;

    dst = pmsgptr->dst;

    struct msgq mq = msgq_arr[dst - 1];

    struct pcb * pcbptr = mq.pcbptr;

    if (!mq.mailq)  //no mails in the mail queue of msg queue
    {   //block the running process
        running->sp = get_PSP();        //save sp

        change_running(pcbptr);     //change running process
        enq((struct qitem *) pcbptr, (struct qitem **) (&blockq)); //enqueue pcb to block queue
        pcbptr->blocked_mailbox = dst;  //set blocked by which msg queue
        pcbptr->pmsgptr = pmsgptr;  //save the address of the msg to pcb
        pcbptr->state = BLOCKED;    //Change state to block
        set_PSP(running->sp);   //set new psp

        return 0;
    }
    else    //mails in the mail queue
    {
        struct mail * mailptr = (struct mail *) deq(    //get the mail
                (struct qitem *) mq.mailq->prev, (struct qitem **) (&mq.mailq));
        pmsgptr->src = mailptr->src; //set the src in msg argument(process stack)

        int i = 0;
        while ((i < mailptr->msgsize) && (i < pmsgptr->size)) //copy the msg from mailbox
        {
            pmsgptr->msg[i] = mailptr->msg[i];
            i++;
        }
        enq((struct qitem *) mailptr, (struct qitem **) (&maillist)); //return the mail to mail list

        return i;
    }
}

/* Supervisor call (trap) entry point
 * Using MSP - trapping process either MSP or PSP (specified in LR)
 * Source is specified in LR: F1 (MSP) or FD (PSP)
 * Save r4-r11 on trapping process stack (MSP or PSP)
 * Restore r4-r11 from trapping process stack to CPU
 * SVCHandler is called with r0 equal to MSP or PSP to access any arguments
 */
void SVCall(void)
{
    kernel = true;
    /* Save LR for return via MSP or PSP */
    __asm("     PUSH    {LR}");

    /* Trapping source: MSP or PSP? */
    __asm("     TST     LR,#4");
    /* Bit #3 (0100b) indicates MSP (0) or PSP (1) */
    __asm("     BNE     RtnViaPSP");

    /* Trapping source is MSP - save r4-r11 on stack (default, so just push) */
    __asm("     PUSH    {r4-r11}");
    __asm("     MRS r0,msp");
    __asm("     BL  SVCHandler");
    /* r0 is MSP */
    __asm("     POP {r4-r11}");
    __asm("     POP     {PC}");

    /* Trapping source is PSP - save r4-r11 on psp stack (MSP is active stack) */
    __asm("RtnViaPSP:");
    __asm("     mrs     r0,psp");
    __asm("     stmdb   r0!,{r4-r11}");
    /* Store multiple, decrement before */
    __asm("     msr psp,r0");
    __asm("     BL  SVCHandler");
    /* r0 Is PSP */

    /* Restore r4..r11 from trapping process stack  */
    __asm("     mrs     r0,psp");
    __asm("     ldmia   r0!,{r4-r11}");
    /* Load multiple, increment after */
    __asm("     msr psp,r0");
    __asm("     POP     {PC}");

}

/*
 * Supervisor call handler
 * Handle startup of initial process
 * Handle all other SVCs such as getid, terminate, etc.
 * Assumes first call is from startup code
 * Argptr points to (i.e., has the value of) either:
 - the top of the MSP stack (startup initial process)
 - the top of the PSP stack (all subsequent calls)
 * Argptr points to the full stack consisting of both hardware and software
 register pushes (i.e., R0..xPSR and R4..R10); this is defined in type
 stack_frame
 * Argptr is actually R0 -- setup in SVCall(), above.
 * Since this has been called as a trap (Cortex exception), the code is in
 Handler mode and uses the MSP
 */
void SVCHandler(struct stack_frame *argptr)
{
    static int firstSVCcall = TRUE;
    struct kcallargs *kcaptr;

    if (firstSVCcall)
    {
        /*
         * Force a return using PSP
         * This will be the first process to run, so the eight "soft pulled" registers
         (R4..R11) must be ignored otherwise PSP will be pointing to the wrong
         location; the PSP should be pointing to the registers R0..xPSR, which will
         be "hard pulled"by the BX LR instruction.
         * To do this, it is necessary to ensure that the PSP points to (i.e., has) the
         address of R0; at this moment, it points to R4.
         * Since there are eight registers (R4..R11) to skip, the value of the sp
         should be increased by 8 * sizeof(unsigned int).
         * sp is increased because the stack runs from low to high memory.
         */
        set_PSP(running->sp + 8 * sizeof(unsigned int));

        firstSVCcall = FALSE;
        /* Start SysTick */
        Systick_init();

        /*
         - Change the current LR to indicate return to Thread mode using the PSP
         - Assembler required to change LR to FFFF.FFFD (Thread/PSP)
         - BX LR loads PC from PSP stack (also, R0 through xPSR) - "hard pull"
         */
        __asm(" movw    LR,#0xFFFD");
        /* Lower 16 [and clear top 16] */
        __asm(" movt    LR,#0xFFFF");
        /* Upper 16 only */
        __asm(" bx  LR");
        /* Force return to PSP */
    }
    else /* Subsequent SVCs */
    {
        /*
         * kcaptr points to the arguments associated with this kernel call
         * argptr is the value of the PSP (passed in R0 and pointing to the TOS)
         * the TOS is the complete stack_frame (R4-R10, R0-xPSR)
         * in this example, R7 contains the address of the structure supplied by
         the process - the structure is assumed to hold the arguments to the
         kernel function.
         * to get the address and store it in kcaptr, it is simply a matter of
         assigning the value of R7 (arptr -> r7) to kcaptr
         */
        kcaptr = (struct kcallargs *) argptr->r7;
        switch (kcaptr->code)
        {
        case GETID:
            kcaptr->rtnvalue = running->pid;    //get the id in pcb
            break;
        case NICE:
            k_nice(kcaptr->arg);
            kcaptr->rtnvalue = kcaptr->arg; //set return value
            break;
        case TERMINATE:
            k_terminate();
            break;
        case SEND:
            kcaptr->rtnvalue = k_send((struct pmsg *) kcaptr->arg, PROCESS); //set return value
            break;
        case RECV:
            kcaptr->rtnvalue = k_recv((struct pmsg *) kcaptr->arg); //set return value
            break;
        case BIND:
            kcaptr->rtnvalue = k_bind(kcaptr->arg); //set return value
            break;
        case UNBIND:
            kcaptr->rtnvalue = k_unbind(kcaptr->arg); //set return value
            break;
        }
    }
    kernel = false;
}
