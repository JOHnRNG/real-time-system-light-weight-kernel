/*
 * Name: Systick.h
 * Description: This header file contains declaration of Systick related function
 * taken and modified from eced 4402 course brightspace
 * Last modified Date: 2019-11-16
 */
#ifndef Systick_h
#define Systick_h

#include <stdio.h>
#include "Buffer.h"
#include "kernel.h"
#include "Uart.h"

/*
 - SysTick sample code
 - Originally written for the Stellaris (2013)
 - Will need to use debugger to "see" interrupts
 - Code uses bit-operations to access SysTick bits
 */
// SysTick Registers
// SysTick Control and Status Register (STCTRL)
#define ST_CTRL_R   (*((volatile unsigned long *)0xE000E010))
// Systick Reload Value Register (STRELOAD)
#define ST_RELOAD_R (*((volatile unsigned long *)0xE000E014))

// SysTick defines
#define ST_CTRL_COUNT      0x00010000  // Count Flag for STCTRL
#define ST_CTRL_CLK_SRC    0x00000004  // Clock Source for STCTRL
#define ST_CTRL_INTEN      0x00000002  // Interrupt Enable for STCTRL
#define ST_CTRL_ENABLE     0x00000001  // Enable for STCTRL
#define SVC_HIGHEST_PRI    0x0

#define NVIC_INT_CTRL_R    (*((volatile unsigned long*)0xE000ED04))
#define NVIC_SYS_PRI13_R   (*((volatile unsigned long*)0xE000ED20))
#define SYS_PRI_2_R          (*((volatile unsigned long*)0xE000ED1C))
#define TRIGGER_PENDSV     0x10000000
#define CLEAR_PENDSV       0x08000000
#define PENDSV_LOWEST_PRIORITY  0x00E00000

// Maximum period
#define MAX_WAIT           0x186A00   /* 160,000 */

#define TRUE    1
#define FALSE   0

#define TICK 1  //0.1 second

#define SYSTICK_HANDLER 0

/* Global to signal SysTick interrupt */
extern volatile int st_overflow_count;

void SysTickStart(void);
void SysTickStop(void);
void SysTickPeriod(unsigned long Period);
void SysTickIntEnable(void);
void SysTickIntDisable(void);
void SysTickHandler(void);
void Systick_init(void);

#endif /* Systick_h */
