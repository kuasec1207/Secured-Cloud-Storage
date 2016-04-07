#include "GF/GF.h"
#include <time.h>
#include <stdio.h>
clock_t start_time, end_time;

/* 計算時間 */
#define TOTAL_TIME_ ((float)(end_time-start_time)/CLOCKS_PER_SEC)
#define TOTAL_TIME(EndTime, StartTime) ((float)(EndTime-StartTime)/CLOCKS_PER_SEC)
#define TOUCH_START start_time=clock()
#define TOUCH_END end_time=clock()

#define safe_malloc(type, size) (type *)my_safe_malloc(sizeof(type)*(size))
void* my_safe_malloc(size_t mSize);
size_t fileLens(FILE *fPtr);



/*
    length=filelength(fileno(stream));
    if(length==-1L)
    perror("filelength failed");.
*/
