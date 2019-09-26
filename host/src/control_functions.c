#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "serial.h"
#include "control_functions.h"
#include "utilities.h"

static unsigned int servo_position = 60;
static unsigned int stepper_position = 90;
static unsigned int left_spd = 0;
static int left_dir = 1;
static unsigned int right_spd = 0;
static int right_dir = 1;
static int left_js = 0; //store previous forward/backward raw value (-32767, 32767)
static int right_js = 0;//store previous left/right raw value (-32767, 32767)


void camera_down(void){
		
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
	printf("camera up %d\n", servo_position);
	UARTSend (instruction_frame, sizeof(instruction_frame));
}

void camera_up(void){
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
	printf("camera down %d\n", servo_position);
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
	//sync the system
	robot_sync (buffer);

}
void robot_move_simple(int dir, int value){
	value += 32767;//move the raw value range from (-32767, 32767) to (0, 65534)

	float spd_difference;

	//determine direction
	left_dir = dir;
	right_dir = dir;

	//store forward/backward value into left_js
	left_js = dir*value/2;

	//determine motor speed without no speed difference
	left_spd = mapValue(0, MAX_JS_VALUE*2, 0, 100, value);
	right_spd = mapValue(0, MAX_JS_VALUE*2, 0, 100, value);

	//adjust speed with speed difference
	if(right_js < 0){
		right_js = (-1)*(right_js);
		spd_difference = mapValue(MIN_JS_VALUE, MAX_JS_VALUE, 0, 100, right_js);
		left_spd = left_spd*(1-spd_difference/100);
	}
	else if(right_js > 0){
		spd_difference = mapValue(MIN_JS_VALUE, MAX_JS_VALUE, 0, 100, right_js);
		right_spd = right_spd*(1-spd_difference/100);
	}

	//send data to robot
	send_motor_data();
	
}

void robot_forward_backward(int left_value){
	left_value = (-1)*left_value; //to set left joystick: forward-positive
	if((left_value-left_js > LEFT_JS_THRESHOLD) || (left_value-left_js < (-1)*LEFT_JS_THRESHOLD)) {

		//process raw data
		float spd_difference;
		int local_right_js;

		//round extreme value to the end
		if(left_value > 32000)
			left_value = MAX_JS_VALUE;
		else if(left_value < -32000)
			left_value = (-1)*MAX_JS_VALUE;
		if(left_value < LEFT_JS_THRESHOLD && left_value > (-1)*LEFT_JS_THRESHOLD)
			left_value = 0;

		//store forward/backward raw value
		left_js = left_value;
		
		
		//determine motors' directions
		if(left_value < 0){
			left_dir = -1;
			right_dir = -1;
			left_value = (-1)*left_value;
		}
		else{
			left_dir = 1;
			right_dir = 1;
		}

		left_spd = mapValue(MIN_JS_VALUE, MAX_JS_VALUE, 0, 100, left_value);
		right_spd = mapValue(MIN_JS_VALUE, MAX_JS_VALUE, 0, 100, left_value);

		if(right_js < 0){
			local_right_js = (-1)*(right_js);
			spd_difference = mapValue(MIN_JS_VALUE, MAX_JS_VALUE, 0, 100, local_right_js);
			left_spd = left_spd*(1-spd_difference/100);
		}
		else if(right_js > 0){
			local_right_js = right_js;
			spd_difference = mapValue(MIN_JS_VALUE, MAX_JS_VALUE, 0, 100, local_right_js);
			right_spd = right_spd*(1-spd_difference/100);
		}

		//send data to robot
		send_motor_data();
			
	}
}
void robot_left_right(int right_value){
	if((right_value-right_js > RIGHT_JS_THRESHOLD) || (right_value-right_js < (-1)*RIGHT_JS_THRESHOLD)) {

		//process raw data
		float spd_difference;
		int local_left_js;

		//round extreme value to the end
		if(right_value > 32000)
			right_value = MAX_JS_VALUE;
		else if(right_value < -32000)
			right_value = (-1)*MAX_JS_VALUE;
		if(right_value < RIGHT_JS_THRESHOLD && right_value > (-1)*RIGHT_JS_THRESHOLD)
			right_value = 0;

		//store left/right value
		right_js = right_value;
		
		//determine motors' directions
		if(left_js < 0){
			left_dir = -1;
			right_dir = -1;
			local_left_js = (-1)*left_js;
		}
		else{
			left_dir = 1;
			right_dir = 1;
			local_left_js = left_js;
		}

		left_spd = mapValue(MIN_JS_VALUE, MAX_JS_VALUE, 0, 100, local_left_js);
		right_spd = mapValue(MIN_JS_VALUE, MAX_JS_VALUE, 0, 100, local_left_js);

		if(right_value < 0){
			right_value = (-1)*(right_value);
			spd_difference = mapValue(MIN_JS_VALUE, MAX_JS_VALUE, 0, 100, right_value);
			left_spd = left_spd*(1-spd_difference/100);
		}
		else if(right_value > 0){
			spd_difference = mapValue(MIN_JS_VALUE, MAX_JS_VALUE, 0, 100, right_value);
			right_spd = right_spd*(1-spd_difference/100);
		}

		//send data to robot
		send_motor_data();
			
	}
}


void robot_sync(unsigned char *buffer){
	unsigned char temp[5];
	//sync servo angle
	temp[0] = buffer[1];
	temp[1] = buffer[2];
	temp[2] = '\0';
	servo_position = (unsigned int)(atoi((char*)temp));
	printf("new servo position: %d\n", servo_position);
	
	//sync stepper motor position
	temp[0] = buffer[4];
	temp[1] = buffer[5];
	temp[2] = '\0';
	stepper_position = (unsigned int)(atoi((char*)temp));
	printf("new stepper position: %d\n", stepper_position);
	
}

void send_motor_data(void){
	//generate 4 control commands according to left_dir, right_dir, left_spd, right_spd
	//send left motor dir
	unsigned char instruction_frame[8];
	instruction_frame[0] = (unsigned char)0;	
	instruction_frame[1] = (unsigned char)3;
	data_32bit_convertor (instruction_frame, left_dir);
	instruction_frame[6] = (unsigned char)0;
	instruction_frame[7] = (unsigned char)0xA;
	printf("left dir: %d\n", left_dir);
	UARTSend (instruction_frame, sizeof(instruction_frame));

	//send right motor dir
	instruction_frame[0] = (unsigned char)0;	
	instruction_frame[1] = (unsigned char)5;
	data_32bit_convertor (instruction_frame, right_dir);
	instruction_frame[6] = (unsigned char)0;
	instruction_frame[7] = (unsigned char)0xA;
	printf("right dir: %d\n", right_dir);
	UARTSend (instruction_frame, sizeof(instruction_frame));

	//send left speed
	instruction_frame[0] = (unsigned char)0;	
	instruction_frame[1] = (unsigned char)4;
	data_32bit_convertor (instruction_frame, left_spd);
	instruction_frame[6] = (unsigned char)0;
	instruction_frame[7] = (unsigned char)0xA;
	printf("left speed: %d\n", left_spd);
	UARTSend (instruction_frame, sizeof(instruction_frame));

	//send right speed
	instruction_frame[0] = (unsigned char)0;	
	instruction_frame[1] = (unsigned char)6;
	data_32bit_convertor (instruction_frame, right_spd);
	instruction_frame[6] = (unsigned char)0;
	instruction_frame[7] = (unsigned char)0xA;
	printf("right speed: %d\n", right_spd);
	UARTSend (instruction_frame, sizeof(instruction_frame));

	sleep(0.2);
		
}
