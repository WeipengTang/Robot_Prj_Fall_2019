#ifndef _CONTROL_FUNCTIONS_H_
#define _CONTROL_FUNCTIONS_H_
#define SERVO_MOVE_INCMT 5
#define STP_MOVE_INCMT 5
#define MAX_SERVO_ANGLE 150
#define MIN_SERVO_ANGLE 40

void camera_up(void);
void camera_down(void);
void camera_left(void);
void camera_right(void);
void stepper_home(void);
void robot_sync(char *buffer);

#endif
