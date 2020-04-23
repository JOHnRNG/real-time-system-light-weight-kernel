/*
 * Name: Time.h
 * Author: Yu Gao
 * Description:This header file contains declaration of time and date related function
 * Last modified date: 2019-11-18
 */

#ifndef SRC_TIME_H_
#define SRC_TIME_H_

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAX_STR_SZ 80   //maximum string size
#define MONTH_PER_YEAR 12   //a year has 12 month
#define LEAP_DAY 29     //February days in leap year
#define IsDecimal(a) ((a >= 48) && (a <= 57))   //number 0 ... 9

//leap year
#define LEAP_YEAR(y) (((y % 4 == 0) && (y % 100 != 0)) || ((y % 400 == 0) && (y % 3200 != 0)))
#define IS_YEAR(y) ((y >= 0) && (y <= 9999))
#define IS_HOUR(h) ((h < 24) && (h >= 0))   //check hour range
#define IS_MINUTE(m) ((m < 60) && (m >= 0)) //minute range
#define IS_SEC(s) ((s < 60) && (s >= 0))    //second range
#define IS_TSEC(t) ((t <= 9) && (t >= 0))   //tenth second range
#define IS_DOT(d) (d == 46)                 //" . "
#define IS_COLON(c) (c == 58)   //" : "

#define INT_TO_CHAR(a) (a + 48)     //convert integer to char
#define CHAR_TO_INT(a) (a - 48)     //convert char to int

/*
 * struct days: defines date table entry,
 * specifies for each month, how many days exist
 */
typedef struct Days
{
    char month[4];
    int days;
    int leap;
} Days_t;

/*
 * struct time: stores hour, minute, second, tenth second
 * as integers
 */
typedef struct Time
{
    int hour;
    int min;
    int sec;
    int tsec;
} Time_t;

/*
 * struct date: stores year, month, day
 * as integers
 */
typedef struct Date
{
    int year;
    int month;
    int day;
} Date_t;

extern Time_t decimal_time;
extern Date_t decimal_date;
extern char char_time[11];
extern char char_date[12];

void update_time(int tick, int * extra_day);
void update_date(int extra_day);
void decimalTime_to_charTime(Time_t * src, char * dst);
void decimalDate_to_charDate(Date_t * src, char * dst);

#endif /* SRC_TIME_H_ */
