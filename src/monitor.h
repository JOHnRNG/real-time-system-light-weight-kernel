/*
 * Name: monitor.h
 * Author: Yu Gao, Zhoubin Xu
 * Description: This header file contains declaration of functions related to screen iutput control,
 * the string manipulation, change cup position, and window management
 * Last modified Date: 2019-11-23
 */

#ifndef SRC_VT100_H_
#define SRC_VT100_H_

#include <strings.h>
#include <stdbool.h>
#include "Buffer.h"
#include "Queue.h"
#include "Proc_Switch.h"

//ASCII chars
#define NUL 0x00
#define ESC 0x1b
#define ENTER 0x0d
#define TAB 0x09
#define BACKSPACE 127

#define NEW_LINE 1  //change to a new line to output
#define MAX_LINE 24 //screen max line
#define MAX_COL 80  //screen max column
#define MIN_LINE 1  //screen min line
#define MIN_COL 1   //screen min column
#define INT_TO_CHAR(a) (a + 48)     //convert integer to char
#define CHAR_TO_INT(a) (a - 48)     //convert char to int
#define MAX_WINDOWS 32  //max window number
#define WINDOW_WIDTH 40 //max window width
#define MAX_STR_SZ 80   //max string size

#define ITOC(n) (n+48)  //integer to char
/*
 * struct CUP: stores VT100 sequence that is uesd to set the cursor
 * location
 */
struct CUP
{
    char esc;
    char sqrbrkt;
    char line[2]; /* 01 through 24 */
    char semicolon;
    char col[2]; /* 01 through 80 */
    char cmdchar;
    char nul;

    int l;
    int c;
};

/*
 * a struct of window, stores the window width(yl, yh), length(xl, xh)
 * stores current line, current row position, the owner pcb
 */
struct window
{
    struct window * next;
    struct window * prev;
    struct pcb * pcbptr;
    int curr_row;
    int curr_col;
    int yl;
    int yh;
    int xl;
    int xh;
};

/*
 * struct String to store chars with length
 */
typedef struct String String_t;
struct String
{
    int length;
    char data[MAX_STR_SZ];
};

extern struct CUP cup;  //cursor position in VT100

extern struct window * asleep_windows; //list of asleep windows (not used by process)
extern struct window * awake_windows;  //list of awake windows (used by process)

/* update CUP location with inc/dec value for line, inc/dec value for col*/
void update_cup(int delta_line, int delta_col);

/* send the CUP to uart output queue */
void send_cup(void);

/* send single char to uart output queue */
void send_char(char in_data);

/* send a string to uart output queue */
void send_str(char * str);

/* send the string as well as change cup position */
void send_result(char * src);

/* register a list of windows */
void reg_windows(void);

/* assign a window to a process */
bool assign_window(struct pcb * new_pcb);

/* segment the screen to small windows */
void seg_windows(unsigned int proc_num);

/* convert integer to string */
void myitoa(int num, char *arr, int length);

/* remove a char from string */
void remove_char(String_t * str);

/* add a char to string */
void add_char(String_t * str, char data);

/* reset the string to null */
void reset_str(String_t * str);

/* get the command which is send from the screen to process */
char* get_cmd(String_t * str, int ProcNumber, int * id);

/* convert string to integer */
extern bool my_atoi(char * arg, int * Time);

/* send the msg to a window */
void send_to_window(struct window * windptr, char * msg, bool new_line);

/* clean the input line (24) */
void clean_input(int cols);

#endif /* SRC_VT100_H_ */
