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
static int forw_backw_value = 0; //store previous forward/backward raw value (-32767, 32767)
static int left_right_value = 0;//store previous left/right raw value (-32767, 32767)
extern Robot_control current_robot_control;
extern Robot_info current_robot_info;
int turn_mode = 1; //mode 0 -- not pivot
				   //mode 1 -- pivot

void camera_reset(void){
	current_robot_control.servo_angle = 60;
	current_robot_control.stepper_target=90;
	servo_position = 60;
	stepper_position = 90;

}

void camera_down(void){		      
	servo_position += SERVO_MOVE_INCMT;
	if(servo_position > MAX_SERVO_ANGLE){
		servo_position = MAX_SERVO_ANGLE;
	}
	//printf("camera up %d\n", servo_position);

	current_robot_control.servo_angle = servo_position;
}

void camera_up(void){
	servo_position -= SERVO_MOVE_INCMT;
	if(servo_position < MIN_SERVO_ANGLE){
		servo_position = MIN_SERVO_ANGLE;
	}
	//printf("camera down %d\n", servo_position);

	current_robot_control.servo_angle = servo_position;

}
void camera_left(void){
	stepper_position += STP_MOVE_INCMT;
	if(stepper_position > 180){
		stepper_position = 180;
	}
	//printf("camera left %d\n", stepper_position);

	current_robot_control.stepper_target = stepper_position;
}

void camera_right(void){
	stepper_position -= STP_MOVE_INCMT;
	if((int)stepper_position < 0){
		stepper_position = 0;
	}
	//printf("camera right %d\n", stepper_position);

	current_robot_control.stepper_target = stepper_position;
}

void stepper_home(void){
	robot_move_simple (1, -32767);
	current_robot_control.command_option = 1;
	current_robot_control.stepper_target = 90;

}

void robot_move_simple(int dir, int value){
	value += 32767;//move the raw value range from (-32767, 32767) to (0, 65534)

	float spd_difference;

	//determine direction
	//left_dir = dir;
	//right_dir = dir;
	current_robot_control.DCM_Left_DIR = dir;
	current_robot_control.DCM_Right_DIR = dir;

	//store forward/backward value into forw_backw_value
	forw_backw_value = dir*value/2;

	//determine motor speed without no speed difference
	left_spd = mapValue(0, MAX_JS_VALUE*2, 0, 100, value);
	right_spd = mapValue(0, MAX_JS_VALUE*2, 0, 100, value);

	//adjust speed with speed difference
	if(left_right_value < 0){
		left_right_value = (-1)*(left_right_value);
		spd_difference = mapValue(MIN_JS_VALUE, MAX_JS_VALUE, 0, 100, left_right_value);
		left_spd = left_spd*(1-spd_difference/100);
	}
	else if(left_right_value > 0){
		spd_difference = mapValue(MIN_JS_VALUE, MAX_JS_VALUE, 0, 100, left_right_value);
		right_spd = right_spd*(1-spd_difference/100);
	}

	//send data to robot
	//send_motor_data();
	current_robot_control.DCM_Left_SPD = left_spd;
	current_robot_control.DCM_Right_SPD = right_spd;
	
}


void robot_left_right(int js_value){
	if((js_value-left_right_value > RIGHT_JS_THRESHOLD) || (js_value-left_right_value < (-1)*RIGHT_JS_THRESHOLD)) {

		//process raw data
		float spd_difference;
		int local_forw_backw_value;
		int local_left_spd, local_right_spd;

		//round extreme value to the end
		if(js_value > 32000)
			js_value = MAX_JS_VALUE;
		else if(js_value < -32000)
			js_value = (-1)*MAX_JS_VALUE;
		if(js_value < RIGHT_JS_THRESHOLD && js_value > (-1)*RIGHT_JS_THRESHOLD)
			js_value = 0;

		//store left/right value
		left_right_value = js_value;

		if(turn_mode == 0){
			//determine motors' directions
			if(forw_backw_value < 0){
				current_robot_control.DCM_Left_DIR = -1;
				current_robot_control.DCM_Right_DIR = -1;
				local_forw_backw_value = (-1)*forw_backw_value;
			}
			else{
				current_robot_control.DCM_Left_DIR = 1;
				current_robot_control.DCM_Right_DIR = 1;
				local_forw_backw_value = forw_backw_value;
			}

			left_spd = mapValue(MIN_JS_VALUE, MAX_JS_VALUE, 0, 100, local_forw_backw_value);
			right_spd = mapValue(MIN_JS_VALUE, MAX_JS_VALUE, 0, 100, local_forw_backw_value);

			if(js_value < 0){ //turn left
				js_value = (-1)*(js_value);
				spd_difference = mapValue(MIN_JS_VALUE, MAX_JS_VALUE, 0, 100, js_value);
				left_spd = left_spd*(1-spd_difference/100);
			}
			else if(js_value > 0){ //turn right
				spd_difference = mapValue(MIN_JS_VALUE, MAX_JS_VALUE, 0, 100, js_value);
				right_spd = right_spd*(1-spd_difference/100);
			}

			//send data to robot
			current_robot_control.DCM_Left_SPD = left_spd;
			current_robot_control.DCM_Right_SPD = right_spd;	
		}else if(turn_mode == 1){
			//pivot mode
			//calculate velocity(speed and direction) according to forw_backw_value and js_value, which both have range from -32767 to 32767
			if(js_value <= 0){ //turn left
				js_value = abs_value(js_value);
				local_right_spd = mapValue(-1*MAX_JS_VALUE, MAX_JS_VALUE, -100, 100, forw_backw_value);
				spd_difference = mapValue (MIN_JS_VALUE, MAX_JS_VALUE, 0, 100, js_value);
				local_left_spd = local_right_spd*(1-2*spd_difference/100);
				
			}
			else if(js_value > 0){ //turn right
				local_left_spd = mapValue(-1*MAX_JS_VALUE, MAX_JS_VALUE, -100, 100, forw_backw_value);
				spd_difference = mapValue (MIN_JS_VALUE, MAX_JS_VALUE, 0, 100, js_value);
				local_right_spd = local_left_spd*(1-2*spd_difference/100);
			}
			//update host values
			left_spd = abs_value(local_left_spd);
			left_dir = direction(local_left_spd);
			right_spd = abs_value(local_right_spd);
			right_dir = direction(local_right_spd);

			//update control instruction
			current_robot_control.DCM_Left_SPD = left_spd;
			current_robot_control.DCM_Left_DIR = left_dir;
			current_robot_control.DCM_Right_SPD = right_spd;
			current_robot_control.DCM_Right_DIR = right_dir;

		}
	}//end of if - data test
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

void send_camera_data(unsigned int opt_num, int pos_value){
	unsigned char instruction_frame[FRAME_SIZE];
	instruction_frame[0] = (unsigned char)0;
	instruction_frame[1] = (unsigned char)opt_num;
	data_32bit_convertor (instruction_frame, pos_value);
	instruction_frame[7] = (unsigned char)0;
	instruction_frame[8] = (unsigned char)0xA;
	UARTSend (instruction_frame, sizeof(instruction_frame));
}


void send_motor_data(void){
	//generate 4 control commands according to left_dir, right_dir, left_spd, right_spd
	//send left motor dir
	unsigned char instruction_frame[FRAME_SIZE];
	instruction_frame[0] = (unsigned char)0;	
	instruction_frame[1] = (unsigned char)3;
	data_32bit_convertor (instruction_frame, left_dir);
	instruction_frame[7] = (unsigned char)0;
	instruction_frame[8] = (unsigned char)0xA;
	printf("left dir: %d\n", left_dir);
	UARTSend (instruction_frame, sizeof(instruction_frame));

	//send right motor dir
	instruction_frame[0] = (unsigned char)0;	
	instruction_frame[1] = (unsigned char)5;
	data_32bit_convertor (instruction_frame, right_dir);
	instruction_frame[7] = (unsigned char)0;
	instruction_frame[8] = (unsigned char)0xA;
	printf("right dir: %d\n", right_dir);
	UARTSend (instruction_frame, sizeof(instruction_frame));

	//send left speed
	instruction_frame[0] = (unsigned char)0;	
	instruction_frame[1] = (unsigned char)4;
	data_32bit_convertor (instruction_frame, left_spd);
	instruction_frame[7] = (unsigned char)0;
	instruction_frame[8] = (unsigned char)0xA;
	printf("left speed: %d\n", left_spd);
	UARTSend (instruction_frame, sizeof(instruction_frame));

	//send right speed
	instruction_frame[0] = (unsigned char)0;	
	instruction_frame[1] = (unsigned char)6;
	data_32bit_convertor (instruction_frame, right_spd);
	instruction_frame[7] = (unsigned char)0;
	instruction_frame[8] = (unsigned char)0xA;
	printf("right speed: %d\n", right_spd);
	UARTSend (instruction_frame, sizeof(instruction_frame));

	//sleep(0.2);
		
}
