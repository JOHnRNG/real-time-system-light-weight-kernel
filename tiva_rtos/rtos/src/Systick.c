/*
 * Name: Systick.c
 * Author: Yu Gao, Zhoubin Xu
 * Description: This source file contains implementation of Systick related function
 * functions taken and modified from eced 4402 course brightspace
 * Last modified Date: 2019-11-18
 */
#include "Systick.h"

volatile int count = 0;

// global variable to count number of interrupts on PORTF0 (falling edge)

void SysTickStart(void)
{
    // Set the clock source to internal and enable the counter to interrupt
    ST_CTRL_R |= ST_CTRL_CLK_SRC | ST_CTRL_ENABLE;
}

void SysTickStop(void)
{
    // Clear the enable bit to stop the counter
    ST_CTRL_R &= ~(ST_CTRL_ENABLE);
}

void SysTickPeriod(unsigned long Period)
{
    /*
     For an interrupt, must be between 2 and 16777216 (0x100.0000 or 2^24)
     */
    ST_RELOAD_R = Period - 1; /* 1 to 0xff.ffff */
}

void SysTickIntEnable(void)
{
    // Set the interrupt bit in STCTRL
    ST_CTRL_R |= ST_CTRL_INTEN;
}

void SysTickIntDisable(void)
{
    // Clear the interrupt bit in STCTRL
    ST_CTRL_R &= ~(ST_CTRL_INTEN);
}

/*
 * systick ISR
 */
void SysTickHandler(void)
{
    static int tick_cnt = 0;
    NVIC_INT_CTRL_R |= TRIGGER_PENDSV;  //enable pendsv

    //send a msg to time server
    if (tick_cnt == 10) //update time each seconds
    {
        tick_cnt = 0;
        struct pmsg args;
        args.src = SYSTICK_HANDLER;
        args.dst = TIME_SERVER_MSGQ;
        args.msg = NULL;
        args.size = 1;
        k_send(&args, HANDLER); //send msg to time server
    }

    tick_cnt ++;   //inc tick
}

/*
 * init systick
 */
void Systick_init(void)
{
    SysTickPeriod(MAX_WAIT);
    SysTickIntEnable();
    SysTickStart();
}

