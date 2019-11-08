#ifndef _CONTROL_FUNCTIONS_H_
#define _CONTROL_FUNCTIONS_H_
#define FRAME_SIZE 9
#define SERVO_MOVE_INCMT 5
#define STP_MOVE_INCMT 5
#define MAX_SERVO_ANGLE 150
#define MIN_SERVO_ANGLE 40
#define LEFT_JS_THRESHOLD 500
#define RIGHT_JS_THRESHOLD 500
#define DATA_THRESHOLD 1000
#define MIN_JS_VALUE 0
#define MAX_JS_VALUE 32767

void camera_reset(void);
void camera_up(void);
void camera_down(void);
void camera_left(void);
void camera_right(void);
void stepper_home(void);
void robot_sync(unsigned char *buffer);
void robot_move_simple(int dir, int value); //simple one direction control
void robot_forward_backward(int value); //for js1 and js2 controling forward/backward and left/right, respectively
void robot_left_right(int value);
void send_camera_data(unsigned int opt_num, int pos_value);
void send_motor_data(void);

#endif
