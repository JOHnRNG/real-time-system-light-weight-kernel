# real-time-operating-system

This project is about real time operating system design based on tiva c microcontroller. 
Supported microcontroller: Tiva TM4C1294NCPDT Microcontroller

Features:
 
 Priority queue: each process is assigned to a priority queue from priority 1 to priority 5, only the process with current highest     priority gets to occupy CPU.
 
 Time quanta: each process is allowed to occupy the CPU for a certain tiem quanta, when the timer expires, it will be swapped out, and next process gets to run.
 
 Inter-process Communication (IPC): processes realize communication with each other by sending messages to each other's message queues. 
 
 Pre-emptive kernel: when a running process requests for resources (messages or hardware resources), and no resources available, then it will be swapped out and re-allocated to a blocked queue.
 
 Kernel protection: processes and the kernel use seperate stack memories (psp, msp)
  
 Window manage for VT100 emulators: processes are allowed to request for windows (a small segement on VT100 terminal), the information output from the processes will be printed only to its own window.
  
Setup Environments:
  1. download Code Composer Studio v7 (CCS) from https://software-dl.ti.com/ccs/esd/documents/ccs_downloads.html
  2. after successfully installation, open CCS, select workspace and create new project
  3. select the following settings
