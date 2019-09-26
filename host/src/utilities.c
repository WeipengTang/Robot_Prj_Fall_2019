#include <stdlib.h>


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