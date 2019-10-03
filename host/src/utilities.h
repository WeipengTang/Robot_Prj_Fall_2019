#ifndef _UTILITIES_H_
#define _UTILITIES_H_

struct robot_info{
	int stepper_angle;
	int servo_angle;
	int left_speed;
	int right_speed;
	int sync_num;
	int LCD_index;
};
typedef struct robot_info Robot_info;

struct robot_control{
	unsigned int stepper_target;
	unsigned int stepper_speed;
	unsigned int servo_angle;
	int DCM_Left_DIR;
	unsigned int DCM_Left_SPD;
	int DCM_Right_DIR;
	unsigned int DCM_Right_SPD;
	unsigned int LCD_index;
	unsigned int command_option;
};

typedef struct robot_control Robot_control;


void data_32bit_convertor(unsigned char *instruction, int value);
unsigned int mapValue(unsigned int minIn, unsigned int maxIn, unsigned int minOut, unsigned int maxOut, unsigned value);
unsigned int abs_value(int value);
#endif