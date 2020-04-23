/*
 * Name: Proc_switch.c
 * Author: Yu Gao, Zhoubin Xu
 * Description:
 * -This source file contains implementation of process switch related funcitons
 * -several functions taken and modified from eced 4402 course brightspace
 * -Last modified Date: 2019-11-21
 */

#include "Proc_switch.h"
#include <stdio.h>

struct msgq msgq_arr[MSGQARR_SIZE] = { NULL };
struct mail * maillist = NULL;
struct pcb * pcblist = NULL;

unsigned long get_PSP(void)
{
    /* Returns contents of PSP (current process stack */
    __asm(" mrs     r0, psp");
    __asm(" bx  lr");
    return 0; /***** Not executed -- shuts compiler up */
    /***** If used, will clobber 'r0' */
}

unsigned long get_MSP(void)
{
    /* Returns contents of MSP (main stack) */
    __asm(" mrs     r0, msp");
    __asm(" bx  lr");
    return 0;
}

void set_PSP(volatile unsigned long ProcessStack)
{
    /* set PSP to ProcessStack */
    __asm(" msr psp, r0");
}

void set_MSP(volatile unsigned long MainStack)
{
    /* Set MSP to MainStack */
    __asm(" msr msp, r0");
}

void volatile save_registers()
{
    /* Save r4..r11 on process stack */
    __asm(" mrs     r0,psp");
    /* Store multiple, decrement before; '!' - update R0 after each store */
    __asm(" stmdb   r0!,{r4-r11}");
    __asm(" msr psp,r0");
}

void volatile restore_registers()
{
    /* Restore r4..r11 from stack to CPU */
    __asm(" mrs r0,psp");
    /* Load multiple, increment after; '!' - update R0 */
    __asm(" ldmia   r0!,{r4-r11}");
    __asm(" msr psp,r0");
}

unsigned long get_SP()
{
    /**** Leading space required -- for label ****/
    __asm("     mov     r0,SP");
    __asm(" bx  lr");
    return 0;
}

/*
 * switch to next process
 */
void next_process(void)
{
    disable();

    running->sp = get_PSP();    //save sp
    running = running->next;    //get next process pcb

    set_PSP(running->sp); //set sp of new process

    enable();
}

/*
 * process switch
 */
void PendSV_Handler(void)
{
    save_registers(); //save registers
    next_process(); //change to next process
    restore_registers(); //restore registers
}

