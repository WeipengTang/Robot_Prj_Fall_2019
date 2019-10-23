#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "utilities.h"
#include "serial.h"
#include "joystick.h"
#include "com_manage.h"

extern Robot_control current_robot_control;
pthread_mutex_t lock;

int main (void){

	current_robot_control.servo_angle = 60;
	current_robot_control.stepper_target = 90;
	current_robot_control.stepper_speed = 2;
	current_robot_control.DCM_Left_DIR = 1;
	current_robot_control.DCM_Left_SPD = 0;
	current_robot_control.DCM_Right_DIR = 1;
	current_robot_control.DCM_Right_SPD = 0;
	current_robot_control.LCD_index = 0;
	current_robot_control.command_option = 0;

	pthread_t monitor_thread;
    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }

	int err = pthread_create(&monitor_thread, NULL, &com_manager, NULL);
	if(err != 0)
		printf("failed to create a thread.\n");
	
	UARTInit();
	joystick_init ();
	

	joystick_loop ();
	pthread_join(monitor_thread, NULL);
	pthread_mutex_destroy(&lock);
	

	return 0;
}	/* end main */




