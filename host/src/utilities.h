#ifndef _UTILITIES_H_
#define _UTILITIES_H_

void data_32bit_convertor(unsigned char *instruction, int value);
unsigned int mapValue(unsigned int minIn, unsigned int maxIn, unsigned int minOut, unsigned int maxOut, unsigned value);
unsigned int abs_value(int value);
#endif