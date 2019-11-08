#!/bin/bash

fswebcam --loop 0.5 -i 0 -d v4l2:/dev/video0 -r 640x480 /home/pi/Documents/Robot_project/camera/image.png