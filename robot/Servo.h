#ifndef _SERVO_H_
#define _SERVO_H_
#include <stdint.h>

#define SERVO_PORT B
#define SERVO_PIN 11	

void ServoInit(void);
void servoPosition(uint32_t angle);



#endif

