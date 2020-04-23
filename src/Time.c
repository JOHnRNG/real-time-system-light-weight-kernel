/*
 * Name: Time.c
 * Author: Yu Gao
 * Description:This source file contains implementation of time and date related function
 * Last modified date: 2019-11-18
 */

#include "Time.h"

/*
 * date table used for get max days for each month
 */
Days_t date_tbl[MONTH_PER_YEAR] = { { "JAN", 31, 31 }, { "FEB", 28, 29 }, {
        "MAR", 31, 31 },
                                    { "APR", 30, 30 }, { "MAY", 31, 31 }, {
                                            "JUN", 30, 30 },
                                    { "JUL", 31, 31 }, { "AUG", 31, 31 }, {
                                            "SEP", 30, 30 },
                                    { "OCT", 31, 31 }, { "NOV", 30, 30 }, {
                                            "DEC", 31, 31 } };

/* store time as integers, when update time, this is updated */
Time_t decimal_time = { 0, 0, 0, 0 };

/* store date as integers, when update date, this is updated */
Date_t decimal_date = { 1900, 1, 1 };

char char_time[11] = { '0', '0', ':', '0', '0', ':', '0', '0', '.', '0' };
char char_date[12] = { '0', '1', '-', 'J', 'A', 'N', '-', '1', '9', '0', '0' };
/*
 * Update current time
 */
void update_time(int tick, int * extra_day)
{
    int extra_sec, extra_min, extra_hour;

    extra_sec = (decimal_time.tsec + tick) / 10;        //get extra second
    extra_min = (decimal_time.sec + extra_sec) / 60;    //get extra MINUTE
    extra_hour = (decimal_time.min + extra_min) / 60;   //get extra HOUR
    *extra_day = (decimal_time.hour + extra_hour) / 24; //get extra day

    decimal_time.tsec = (decimal_time.tsec + tick) % 10; //add extra tenth sec to current value
    decimal_time.sec = (decimal_time.sec + extra_sec) % 60; //add extra sec to current value
    decimal_time.min = (decimal_time.min + extra_min) % 60; //add extra min to current value
    decimal_time.hour = (decimal_time.hour + extra_hour) % 24; //add extra hour to current value
}
/*
 * Update current date
 */

void update_date(int extra_day)
{
    int extra_year, extra_mon;
    int d, m, y;

    d = decimal_date.day;
    m = decimal_date.month;
    y = decimal_date.year;

    if ((m == 2) && LEAP_YEAR(y))   //leap year and February
    {
        extra_mon = (extra_day + d - 1) / (date_tbl[m - 1].days + 1); //get extra month
        decimal_date.day = 1 + (d + extra_day - 1) % (date_tbl[m - 1].days + 1); //update day
    }
    else    //not leap year
    {
        extra_mon = (extra_day + d - 1) / date_tbl[m - 1].days;
        decimal_date.day = 1 + (d + extra_day - 1) % date_tbl[m - 1].days;
    }

    extra_year = (m + extra_mon - 1) / 12;  //get extra year
    decimal_date.month = 1 + (m + extra_mon - 1) % 12;  //update month
    decimal_date.year = (y + extra_year) % 10000;   //update year
}

void decimalTime_to_charTime(Time_t * src, char * dst)
{
    dst[0] = INT_TO_CHAR(src->hour / 10);   //get first char of hour
    dst[1] = INT_TO_CHAR(src->hour % 10);   //get second char of hour
    dst[3] = INT_TO_CHAR(src->min / 10);    //get first char of minute
    dst[4] = INT_TO_CHAR(src->min % 10);    //get second char of minute
    dst[6] = INT_TO_CHAR(src->sec / 10);    //get first char of sec
    dst[7] = INT_TO_CHAR(src->sec % 10);    //get second char of sec
    dst[9] = INT_TO_CHAR(src->tsec);        //get char of tenth sec
}

/*
 * convert decimal date to a string
 */
void decimalDate_to_charDate(Date_t * src, char * dst)
{
    dst[3] = date_tbl[src->month - 1].month[0]; //get the char month
    dst[4] = date_tbl[src->month - 1].month[1]; //get the char month
    dst[5] = date_tbl[src->month - 1].month[2]; //get the char month
    dst[0] = INT_TO_CHAR(src->day / 10);    //get first char of day
    dst[1] = INT_TO_CHAR(src->day % 10);    //get second char of day

    int div10 = src->year / 10; //first digit of year
    int div100 = div10 / 10;    //second digit
    int div1000 = div100 / 10;  //third digit

    dst[10] = INT_TO_CHAR(src->year % 10);  //get first char of year
    dst[9] = INT_TO_CHAR(div10 % 10);   //get next char of year
    dst[8] = INT_TO_CHAR(div100 % 10);  //get next char of year
    dst[7] = INT_TO_CHAR(div1000 % 10); //get next char of year
}
