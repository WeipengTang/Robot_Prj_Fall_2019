#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include "utilities.h"

Robot_info current_robot_info;
Robot_control current_robot_control;
void data_32bit_convertor(unsigned char *instruction, int value){
	instruction[2] = (unsigned char)(((value&0xFF000000)>>24)&0xFF);
	instruction[3] = (unsigned char)(((value&0x00FF0000)>>16)&0xFF);
	instruction[4] = (unsigned char)(((value&0x0000FF00)>>8)&0xFF);
	instruction[5] = (unsigned char)(value&0x000000FF);

}

unsigned int mapValue(unsigned int minIn, unsigned int maxIn, unsigned int minOut, unsigned int maxOut, unsigned value){
	
	//to ensure input not exceed input limits
	if(value < minIn) value = minIn;
	if(value > maxIn) value = maxIn;
	
	return ((value - minIn)*(maxOut - minOut)/(maxIn - minIn) + minOut);
	
}

unsigned int abs_value(int value){

if(value < 0)
		return value*(-1);
else
		return value;

}
int msleep(long msec)
{	
	int errno;
    struct timespec ts;
    int res;

    if (msec < 0)
    {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}