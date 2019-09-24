#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "serial.h"
#include "control_functions.h"
#include "utilities.h"

static unsigned int servo_position = 60;
static unsigned int stepper_position = 90;
void camera_up(void){
		
	unsigned char instruction_frame[8];
	servo_position += SERVO_MOVE_INCMT;
	if(servo_position > MAX_SERVO_ANGLE){
		servo_position = MAX_SERVO_ANGLE;
	}
	instruction_frame[0] = (unsigned char)0;
	instruction_frame[1] = (unsigned char)7;
	data_32bit_convertor (instruction_frame, servo_position);
	instruction_frame[6] = (unsigned char)0;
	instruction_frame[7] = (unsigned char)0xA;
	printf("camera up\n");
	UARTSend (instruction_frame, sizeof(instruction_frame));
}

void camera_down(void){
	unsigned char instruction_frame[8];
	servo_position -= SERVO_MOVE_INCMT;
	if(servo_position < MIN_SERVO_ANGLE){
		servo_position = MIN_SERVO_ANGLE;
	}
	instruction_frame[0] = (unsigned char)0;
	instruction_frame[1] = (unsigned char)7;
	data_32bit_convertor (instruction_frame, servo_position);
	instruction_frame[6] = (unsigned char)0;
	instruction_frame[7] = (unsigned char)0xA;
	printf("camera down\n");
	UARTSend (instruction_frame, sizeof(instruction_frame));

}
void camera_left(void){
	unsigned char instruction_frame[8];
	stepper_position += STP_MOVE_INCMT;
	if(stepper_position > 180){
		stepper_position = 180;
	}
	instruction_frame[0] = (unsigned char)0;
	instruction_frame[1] = (unsigned char)1;
	data_32bit_convertor (instruction_frame, stepper_position);
	instruction_frame[6] = (unsigned char)0;
	instruction_frame[7] = (unsigned char)0xA;
	printf("camera left %d\n", stepper_position);
	UARTSend (instruction_frame, sizeof(instruction_frame));
}

void camera_right(void){
	unsigned char instruction_frame[8];
	stepper_position -= STP_MOVE_INCMT;
	if((int)stepper_position < 0){
		stepper_position = 0;
	}
	instruction_frame[0] = (unsigned char)0;
	instruction_frame[1] = (unsigned char)1;
	data_32bit_convertor (instruction_frame, stepper_position);
	instruction_frame[6] = (unsigned char)0;
	instruction_frame[7] = (unsigned char)0xA;
	printf("camera right %d\n", stepper_position);
	UARTSend (instruction_frame, sizeof(instruction_frame));
}
void stepper_home(void){
	unsigned char instruction_frame[8];
	unsigned char buffer[100];
	instruction_frame[0] = (unsigned char)1;
	instruction_frame[1] = (unsigned char)1;
	instruction_frame[6] = (unsigned char)0;
	instruction_frame[7] = (unsigned char)0xA;
	printf("stepper homing\n");
	UARTSend (instruction_frame, sizeof(instruction_frame));
	memset (buffer, 0, 100);
	UARTReceive (buffer, 100);
	printf("%s\n", buffer);
	robot_sync (buffer);

}
void robot_sync(char *buffer){
	printf("%d\n", buffer[1]);
	printf("%d\n", buffer[2]);
	servo_position = (unsigned int)((buffer[1]<<8)|buffer[2]);
	printf("new servo position: %d\n", servo_position);
	stepper_position = (unsigned int)((buffer[4]<<8)|buffer[5]);
	printf("new stepper position: %d\n", stepper_position);
	
}
