/*
 * Name: monitor.c
 * Author: Yu Gao, Zhoubin Xu
 * Description: This source file contains implementation of functions related to screen iutput control,
 * the string manipulation, change cup position, and window management
 * Last modified Date: 2019-11-23
 */
#include "monitor.h"

/* stores the location on screen */
struct CUP cup = { ESC, '[', '2', '4', ';', '0', '1', 'H', NUL, 24, 1 };

struct window * asleep_windows = NULL;
struct window * awake_windows = NULL;

/*
 * string to int, if string only has decimal value,
 * return true, else return false
 */
bool my_atoi(char * arg, int * Time)
{
    if (!arg)   //string null
        return true;

    int len = (int) strlen(arg);    //string length
    int i;
    int t = 0;

    for (i = 0; i < len; i++)   //loop through string
    {
        int tmp = arg[i] - 48;

        if ((tmp >= 0) && (tmp <= 9)) //char is a decimal
            t = t * 10 + tmp;
        else
            return false;
    }

    *Time = t;
    return true;
}

/*
 * convert integer to string
 */
void myitoa(int num, char *arr, int length)
{
    int i = length - 2;

    while (i >= 0)
    {
        arr[i] = ITOC(num % 10);    //convert a digit
        num = num / 10; //remove a digit from number

        i--;
    }
    arr[length - 1] = '\0';
}

/*
 * reset the string that stores commands
 */
void reset_str(String_t * str)
{
    int i;

    for (i = 0; i < str->length; i++)   //set all elements to null
    {
        str->data[i] = '\0';
    }

    str->length = 0;
}

/*
 * append a char to a string
 */
void add_char(String_t * str, char data)
{
    if (str->length < MAX_STR_SZ) //temp length is the location that CUP points to
    {
        str->length += 1;   //inc length
        str->data[str->length - 1] = data;  //store char
    }
}

/*
 * remove a char from the string
 */
void remove_char(String_t * str)
{
    if (str->length > 0)
    {
        str->data[str->length - 1] = '\0';  //remove char
        str->length -= 1;   //dec length
    }
}

char* get_cmd(String_t * str, int ProcNumber, int* id)
{
    char * msg;
    char * cmd = strtok(str->data, " \t");

    if (cmd)    //command exist, check if it's PROC
    {
        if (strcasecmp(cmd, "PROC") == 0)   //cmd is PROC
        {
            char * number = strtok(NULL, " \t");
            int num;

            if (my_atoi(number, &num) && (num >= 1) && (num <= MAX_PID)) //valid process number
            {
                *id = num;
                msg = strtok(NULL, " \t");
                return msg;
            }
            else
            {
                *id = -3; //invalid process number
                return NULL;
            }
        }
        else
        {
            *id = -2; //invalid cmd
            return NULL;
        }
    }
    else
    {
        *id = -1; //no cmd
        return NULL;
    }
}

/*
 * clean the input line(line 24)
 */
void clean_input(int cols)
{
    int i = 0;
    while (i < cols)
    {
        send_char(' '); //occupy each position with whitespace
        i++;
    }

    update_cup(0, -cup.c);  //update cup to original
    send_cup(); //change cup position
}

/*
 * send the msg the window, new line indicate update line number to next line
 */
void send_to_window(struct window * windptr, char * msg, bool new_line)
{
    char pre_l[2], pre_c[2];
    pre_l[0] = cup.line[0]; //store the current cup position
    pre_l[1] = cup.line[1];
    pre_c[0] = cup.col[0];
    pre_c[1] = cup.col[1];
    int l = cup.l;
    int c = cup.c;

    //update cup
    cup.col[0] = INT_TO_CHAR((windptr->curr_col % 100) / 10); //convert int CUP to char CUP
    cup.col[1] = INT_TO_CHAR(windptr->curr_col % 10);
    cup.line[0] = INT_TO_CHAR((windptr->curr_row % 100) / 10);
    cup.line[1] = INT_TO_CHAR(windptr->curr_row % 10);
    cup.l = windptr->curr_row;
    cup.c = windptr->curr_col;

    if (new_line)   //update line
    {
        if ((windptr->curr_row + 1) <= windptr->xh)  //within window range
        {
            windptr->curr_row += 1; //next line
            windptr->curr_col = windptr->yl;    //column back to yl
        }
        else    //out of window range
        {
            windptr->curr_row = windptr->xl;    //set to original position
            windptr->curr_col = windptr->yl;
        }
    }
    else
    {
        if ((windptr->curr_row + 1) > windptr->xh)  //without window range
        {
            windptr->curr_row = windptr->xl;     //set to original position
        }
        else    //within window range
        {
            windptr->curr_col += strlen(msg);   //update column
        }
    }

    send_cup(); //change cursor position to process
    send_str(msg);  //send msg

    cup.col[0] = pre_c[0];  //restore cup position
    cup.col[1] = pre_c[1];
    cup.line[0] = pre_l[0];
    cup.line[1] = pre_l[1];
    cup.l = l;
    cup.c = c;

    send_cup(); //change back cup
}

/*
 * update the CUP
 */
void update_cup(int dline, int dcol)
{
    int line, col;

    line = cup.l;
    col = cup.c;

    if (!((dcol > 0) && (((col + dcol) > MAX_COL)))) //right shift, exceed col range
    {
        if ((dcol > 0) && (((col + dcol) <= MAX_COL))) //right shift, with range
            col += dcol;
        else if ((dcol < 0) && (((col + dcol) <= MIN_COL))) //left shift, exceed minimum range
            col = 1;
        else if ((dcol < 0) && (((col + dcol) > MIN_COL))) //left shift, within range
            col += dcol;

        if ((dline > 0) && (((line + dline) <= MAX_LINE))) //down shift, within range
            line += dline;
        else if ((dline > 0) && (((line + dline) > MAX_LINE))) //down shift, exceed range
        {
            //resume to line 21, col 01
            // line = 21;
            col = 1;
        }

    }
    cup.l = line;
    cup.c = col;
    cup.col[0] = INT_TO_CHAR((col % 100) / 10); //convert int CUP to char CUP
    cup.col[1] = INT_TO_CHAR(col % 10);
    cup.line[0] = INT_TO_CHAR((line % 100) / 10);
    cup.line[1] = INT_TO_CHAR(line % 10);

}

/*
 * register a list of windows
 */
void reg_windows(void)
{
    int i = 0;
    while (i < MAX_WINDOWS) //create a new window and link them together
    {
        struct window * wptr = (struct window *) malloc(sizeof(struct window));

        wptr->curr_col = 1;
        wptr->curr_row = 1;
        wptr->pcbptr = NULL;
        wptr->xh = 0;
        wptr->xl = 0;
        wptr->yl = 0;
        wptr->yh = 0;

        enq((struct qitem *) wptr, (struct qitem **) (&asleep_windows));

        i++;
    }
}

void set_window(struct window * ptr, int xl, int xh, int yl, int yh) //segment window into pieces
{
    ptr->xl = xl;
    ptr->xh = xh;
    ptr->yl = yl;
    ptr->yh = yh;
    ptr->curr_row = xl;
    ptr->curr_col = yl;

    //indicate the ownwer of this window with pid output to screen
    char pid[3];
    myitoa(ptr->pcbptr->pid, pid, 3);
    send_to_window(ptr, "PID: ", !NEW_LINE);
    send_to_window(ptr, pid, NEW_LINE);

    ptr->xl += 1;
}

/*
 * assgin a window to a process
 */
bool assign_window(struct pcb * new_pcb)
{
    struct qitem * qptr = (struct qitem *) asleep_windows;
    struct qitem ** queue = (struct qitem **) (&asleep_windows);
    struct window * windptr = (struct window *) deq(qptr, queue);

    if (!windptr)   //no available windows
        return false;
    else
    {
        windptr->pcbptr = new_pcb;  //set window owner
        new_pcb->windowptr = windptr;   //set window pointer of pcb

        enq((struct qitem *) windptr, (struct qitem **) (&awake_windows)); //enqueue window to awake list
        return true;
    }
}

/*
 * split the putty screen to small segments
 */
void seg_windows(unsigned int proc_num) //segment window into pieces
{
    struct window * ptr = awake_windows;

    int row = 1, col = 1;
    int col_windows = 2;    //two columns in total
    int row_windows = (proc_num + 1) / col_windows; //number of windows in rows
    int deltax = (MAX_LINE - 1) / row_windows - 1;   //save 4 rows at the bottom
    int deltay = MAX_COL / col_windows - 1;

    int i = 0;
    while ((i < proc_num) && ptr)   //walk through window list
    {
        ptr->xl = row;
        ptr->xh = row + deltax;
        ptr->yl = col;
        ptr->yh = col + deltay;
        ptr->curr_row = row;
        ptr->curr_col = col;
        col = col + deltay + 1;
        if (col > MAX_COL)
        {
            row = row + deltax + 1;
            col = 1;
        }

        //indicate the ownwer of this window with pid output to screen
        char pid[3];
        myitoa(ptr->pcbptr->pid, pid, 3);
        send_to_window(ptr, "PID: ", !NEW_LINE);
        send_to_window(ptr, pid, NEW_LINE);

        ptr->xl += 1;
        ptr = ptr->next;

        i++;
    }
}


/*
 * send a char to uart output queue
 */
void send_char(char in_data)
{
    enqueue(UART_OUT, in_data); //enqueue the char
}

/*
 * send a string to uart output queue
 */
void send_str(char * str)
{
    int i = 0;
    int length = strlen(str);

    while (i < length)
    {
        send_char(str[i]);
        i += 1;
    }
}

/*
 * send the CUP to uart output queue
 */
void send_cup(void)
{
    send_char(cup.esc);
    send_char(cup.sqrbrkt);
    send_char(cup.line[0]);
    send_char(cup.line[1]);
    send_char(cup.semicolon);
    send_char(cup.col[0]);
    send_char(cup.col[1]);
    send_char(cup.cmdchar);
    send_char(cup.nul);
}
/*
 * send the string back to user, as well as change the CUP
 */
void send_result(char * src)
{
    update_cup(1, -atoi(cup.col));  //update CUP
    send_cup(); //send CUP to queue
    send_str(src);  //send string to queue
    update_cup(1, 0);
    send_cup();
}
