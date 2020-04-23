/*
 * tests.c
 *
 *  Created on: 2020Äê4ÔÂ22ÈÕ
 *      Author: Yu Gao
 */

#include "tests.h"

/*
 * several tests processes
 */
#ifdef TEST
void f1()
{      //process out put number through 0-19
    bind(50);      //bind to mailbox
    int i = 0;
    char msg[5];
    while (i < 20)
    {
        int j = 0;
        while (j < 700000)
        {      //create delay
            j++;
        }
        myitoa(i, msg, 5);      //integer to array
        send(UART_SERVER_MSGQ, 50, msg, 5);    //send msg(number) to uart server
        i++;
    }
}

void f2()
{      //process out put number through 0-19
    bind(51);      //bind to mailbox
    char msg[5];
    int i = 0;

    while (i < 20)
    {      //create delay
        int j = 0;
        while (j < 700000)
        {
            j++;
        }
        myitoa(i, msg, 5);      //integer to array
        send(UART_SERVER_MSGQ, 51, msg, 5);    //send msg(number) to uart server
        i++;
    }
}

void f3()
{      //process out put number through 0-19
    bind(52);      //bind to mailbox
    char msg[5];
    int i = 0;

    while (i < 20)
    {
        int j = 0;
        while (j < 700000)      //create delay
        {
            j++;
        }
        myitoa(i, msg, 5);      //integer to array
        send(UART_SERVER_MSGQ, 52, msg, 5);    //send msg(number) to uart server
        i++;
    }
}

void f4()
{      //process out put number through 0-19
    bind(53);      //bind to mailbox
    char msg[5];
    int i = 0;
    while (i < 20)
    {
        int j = 0;
        while (j < 700000)      //create delay
        {
            j++;
        }
        myitoa(i, msg, 5);      //integer to array
        send(UART_SERVER_MSGQ, 53, msg, 5);    //send msg(number) to uart server
        i++;
    }
}

void f5()
{      //process out put number through 0-19
    bind(54);      //bind to mailbox
    char msg[5];
    int i = 0;
    while (i < 10)      //output 0-9
    {
        int j = 0;
        while (j < 700000)      //create delay
        {
            j++;
        }
        myitoa(i, msg, 5);      //integer to array
        send(UART_SERVER_MSGQ, 54, msg, 8);    //send msg(number) to uart server

        i++;
    }

    nice(2);      //nice to change priority

    while (i < 20)      //output 10-19
    {
        int j = 0;
        while (j < 700000)      //create delay
        {
            j++;
        }
        myitoa(i, msg, 5);      //integer to array
        send(UART_SERVER_MSGQ, 54, msg, 8);    //send msg(number) to uart server
        i++;
    }

}

void send_test2()
{      //process to send a message from mailbox 13 to mailbox 12
    int rtn = bind(13);      //bind to mailbox
    char * msg = "hello there";

    if (rtn > 0)      //bind succeed
    {
        send(12, 13, msg, strlen(msg));     //send the msg from mailbox 12 to 13
    }
}

void recv_test2()
{      //process receive a message from mailbox 12
    int rtn = bind(12);      //bind to mailbox
    int src = 0;      //variable to record the source mailbox
    char msg[20];      //Array where store the msg
    if (rtn > 0)
        recv(12, &src, msg, 12);      //receive from mailbox12, size 12
    send(UART_SERVER_MSGQ, 12, msg, 15); //send the recieved message to uart server
}

void send_test5()
{      //process which send a message to itself
    int rtn = bind(28);      //bind to mailbox 28
    int src = 0;      //variable to record the source mailbox
    char * msg = "hello there";
    char MSG[20];      //Array where store the msg
    if (rtn > 0)      //bind succeed
    {
        send(28, 28, msg, 15);      //send the msg from mailbox 28 to 28
    }
    recv(28, &src, MSG, 12);      //receive from mailbox28, size 12
    send(UART_SERVER_MSGQ, 28, MSG, 15); //send the recieved message to uart server
}

void output_test1()
{ //process which receive msg from uart server and then send back to uart server to output
    bind(11);
    int src;      //variable to record the source mailbox
    char MSG[20];
    send(UART_SERVER_MSGQ, 11, "Enter: ", 20); //send a message to uart server,request input
    while (1)
    {
        recv(11, &src, MSG, 20);      //Receive from mailbox 11
        send(UART_SERVER_MSGQ, 11, MSG, 20); //send back the message to uart server
    }
}

void output_test2()
{ //process which receive msg from uart server and then send back to uart server to output
    bind(12);
    int src;      //variable to record the source mailbox
    char MSG[20];
    send(UART_SERVER_MSGQ, 12, "Proc2:", 20); //send a message to uart server,request input
    while (1)
    {
        recv(12, &src, MSG, 20);      //Receive from mailbox 12
        send(UART_SERVER_MSGQ, 12, MSG, 20); //send back the message to uart server
    }
}

void output_test3()
{ //process which receive msg from uart server and then send back to uart server to output
    bind(13);
    int src;      //variable to record the source mailbox
    char MSG[20];
    send(UART_SERVER_MSGQ, 13, "Proc3:", 8); //send a message to uart server to output and request input
    while (1)
    {
        recv(13, &src, MSG, 20);      //Receive from mailbox 13
        send(UART_SERVER_MSGQ, 13, MSG, 20); //send back the message to uart server
    }
}

void req_time()
{
    bind(17);      //bind to mailbox 17
    bind(18);      //bind to mailbox 18

    int src;      //variable to record the source mailbox
    char cmd[10];      //variable to record the cmd from uart server
    char time[11];      //variable to record the current time from  time server
    send(UART_SERVER_MSGQ, 18, "Enter TIME to get time!", 22); //send a message to uart server to output and request input
    recv(18, &src, cmd, 8);      //Receive cmd from mailbox 18
    if (strcasecmp(cmd, "TIME") == 0)
    {      //cmd equal time
        send(TIME_SERVER_MSGQ, 17, "TIME", 5); //send msg "TIME" to request current time message from time server
        recv(17, &src, time, 11);      //Receive the time message
        send(UART_SERVER_MSGQ, 18, time, 22); //send time to uart server to output
    }
}

#endif
