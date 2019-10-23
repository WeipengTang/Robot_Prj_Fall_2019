#include "com_manage.h"
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include "utilities.h"
#include "serial.h"
#include "socket_client.h"

extern pthread_mutex_t lock;
extern Robot_control current_robot_control;
extern Robot_info current_robot_info;

void *com_manager(void *arg){
	unsigned int i;
	static unsigned int count = 33;
	while(1){
		ping_robot (count);
		for(i=0; i<(0x00EFFFFF);i++);
		verify_robot_ping (count);
		count++;
		if(count >= 128)
			count=33;
		for(i=0; i<(0x06FFFFFF);i++);
	}
	
}
void ping_robot(unsigned int num){
	char out_buffer[100];
	memset(out_buffer, 0, 100);
	if(current_robot_control.command_option == 0){
	out_buffer[0] = (char)33;
	out_buffer[1] = (char)num;//command number
	out_buffer[2] = (char)33;//command option
	sprintf(out_buffer+3, "$%d$%d$%d$%d$%d$%d$%d$%d$", current_robot_control.stepper_target,
	        								  current_robot_control.stepper_speed,
	        								  current_robot_control.servo_angle,
	        								  current_robot_control.DCM_Left_DIR,
	        								  current_robot_control.DCM_Left_SPD,
	        								  current_robot_control.DCM_Right_DIR,
	        								  current_robot_control.DCM_Right_SPD,
	        								  current_robot_control.LCD_index);
	out_buffer[strlen(out_buffer)+1] = '\n';
	//****************************************************************
	//Need to replace this part with socket send
	//UARTSend ((unsigned char*)out_buffer, (strlen(out_buffer)+2));
	socket_send(out_buffer, (strlen(out_buffer)+2));
	}
	//****************************************************************
	else if(current_robot_control.command_option == 1){
	out_buffer[0] = (char)34;
	out_buffer[1] = (char)num;//command number
	out_buffer[2] = (char)current_robot_control.command_option;
	out_buffer[3] = '\0';
	out_buffer[4] = '\n';
	current_robot_control.command_option = 0;	
	//****************************************************************
	//Need to replace this part with socket send
	//UARTSend ((unsigned char*)out_buffer, (strlen(out_buffer)+2));
	socket_send(out_buffer, (strlen(out_buffer)+2));
	//****************************************************************
	sleep(15);
	
	}
	

}
unsigned int verify_robot_ping(unsigned int num){
	char buffer[100];
	
	static int error_check=0;
	//int i;
	memset(buffer, 0, 100);
	//***************************************************************
	//Need to update this part with socket receive
	//if(UARTReceive (buffer, 100)!=0)
	if(socket_receive(buffer, 100)<0)
		error_check++;
	else
		error_check = 0;
	//***************************************************************
	
	//update_robot_info(buffer);
	//print_info_ts();

	printf("%s\n", buffer);
/*	if(error_check > 5){
		printf("==============================================\n");
		printf("Robot disconnected\n");
		
	}*/
	
	
	return 0;
}


void update_robot_info(unsigned char* buffer){
		//char temp_buffer[50];		
		//UARTString(temp_buffer);
		char temp_num[5];
		int i=0;
		int j=0;
		int k=0;
		int data_array[8];
		
		do{
			if(buffer[i] == '$')
			{
				j=i+1;
				do{
					j++;

				}while(buffer[j]!='$');

				memcpy(temp_num, buffer+i+1, j-i-1);
				temp_num[j-i-1] = '\0';
				data_array[k] = atoi(temp_num);
				k++;
			}
			else
				return;	
			i = j;
		}while(k<8);
	
		current_robot_info.servo_angle = (int)data_array[0];
		current_robot_info.stepper_angle = (int)data_array[1];
		current_robot_info.left_direction = (int)data_array[2];
		current_robot_info.left_speed = (int)data_array[3];
		current_robot_info.right_direction = (int)data_array[4];
		current_robot_info.right_speed = (int)data_array[5];
		current_robot_info.LCD_index = (int)data_array[6];
		current_robot_info.sync_num = (int)data_array[7];


}

void print_info_ts(void)
{
clear_sc(); 
printf("Stepper Position: %d\n",  current_robot_info.stepper_angle);
printf("Servo Angle: %d\n",  current_robot_info.servo_angle);
printf("Left Motor Speed: %d\n",  current_robot_info.left_speed);
printf("Left Motor Direction: %d\n",  current_robot_info.left_direction);
printf("Right Motor Speed: %d\n",  current_robot_info.right_speed);
printf("Right Motor Direction: %d\n",  current_robot_info.right_direction);
printf("Command Number: %d\n",  current_robot_info.sync_num);

printf("\n====================Send out commands==========================\n");
printf("Stepper Position: %d\n",  current_robot_control.stepper_target);
printf("Servo Angle: %d\n",  current_robot_control.servo_angle);
printf("Left Motor Speed: %d\n",  current_robot_control.DCM_Left_SPD);
printf("Left Motor Direction: %d\n",  current_robot_control.DCM_Left_DIR);
printf("Right Motor Speed: %d\n",  current_robot_control.DCM_Right_SPD);
printf("Right Motor Direction: %d\n",  current_robot_control.DCM_Right_DIR);

}
