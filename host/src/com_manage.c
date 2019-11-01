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
#define MAXINPUTLEN	50

extern pthread_mutex_t lock;
extern Robot_control current_robot_control;
extern Robot_info current_robot_info;

void *com_manager(void *arg){
	
	static unsigned int count = 33;
	while(1){
/*		ping_robot (count);
		msleep(30);
		verify_robot_ping (count);
		count++;
		if(count >= 128)
			count=33;
		msleep(75);*/
		request_image ();
		msleep(100);
		receive_image ();
		msleep(900);
	}
	
}
void ping_robot(unsigned int num){
	char out_buffer[MAXINPUTLEN];
	memset(out_buffer, 0, MAXINPUTLEN);
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
	socket_send(out_buffer, (strlen(out_buffer)+2));
	//****************************************************************
	sleep(15);
	
	}
	

}
unsigned int verify_robot_ping(unsigned int num){
	char buffer[MAXINPUTLEN];
	
	static int error_check=0;
	//int i;
	memset(buffer, 0, MAXINPUTLEN);
	//***************************************************************
	//Need to update this part with socket receive
	if(socket_receive(buffer, MAXINPUTLEN)<0)
		error_check++;
	else
		error_check = 0;
	//***************************************************************
	//printf("error check: %d\n", error_check);

	update_robot_info((unsigned char*)buffer);
	print_info_ts();

	//printf("%s\n", buffer);
	if(error_check > 5){
		printf("==============================================\n");
		printf("Robot disconnected\n");
		
	}
	
	
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

void request_image(void){
	char out_buffer[2];
	out_buffer[0] = (char)35;
	out_buffer[1] = '\0';
	socket_send(out_buffer, (strlen(out_buffer)+1));

}
void receive_image(void){
	int receive_size;
	char receive_buffer[90000];
	receive_size = socket_receive(receive_buffer, 90000);
	printf("receive_size: %d\n", receive_size);

}


/*void receive_image(void){
	int32_t image_size=0;
	int32_t image_flag=0; //0 - successful, -1 - failed
	int receive_count = 0;
	char *image_buffer;
	FILE *image_fptr;
	//get image size
	if(socket_receive((char*)&image_size, sizeof(image_size))<0){
		printf("Error receiving image size.\n");
		image_flag = -1;
		socket_send ((char*)&image_flag, sizeof(image_flag));
		return;
	}
	//check if server is able to open the image file
	if(image_size < 0){
		printf("Server cannot open the image file.\n");
		return;
	}
		
	printf("Image size: %d\n", image_size);

	//check if client is able to reserve enough space for image
	image_buffer = (char*)malloc(sizeof(char)*image_size);
	if(image_buffer == NULL){
		printf("Client failed to allocate space for image.\n");
		image_flag = -1;
		socket_send ((char*)&image_flag, sizeof(image_flag));
		return;
	}
	memset(image_buffer, 0, image_size);
	
	//check if client is able to open/create the image file
	image_fptr = fopen("/home/a3154-18/Documents/Robot_Prj_Fall_2019/camera/image.png", "wb");
	if(image_fptr == NULL){
		printf("Client cannot open/create image file.\n");
		image_flag = -1;
		socket_send ((char*)&image_flag, sizeof(image_flag));
		return;
		free(image_buffer);
	}
	
	//ready to receive the image
	socket_send ((char*)&image_flag, sizeof(image_flag));

	//receive image data
	while(receive_count < image_size){
		receive_count = socket_receive(image_buffer, image_size);
		if(receive_count>0){
			fwrite(image_buffer, 1, image_size, image_fptr);
			receive_count+=receive_count;		
		}
	}
	

	fclose(image_fptr);
	free(image_buffer);
	
	


}
*/