import os
import datetime
import sys
import time
import subprocess

while 1:
    try:
        # read the absolute path
        script_dir = os.path.dirname(__file__)
        # call the .sh to capture the image
        os.system('./webcam.sh')
        #get the date and time, set the date and time as a filename.
        currentdate = datetime.datetime.now().strftime("%Y-%m-%d_%H%M")
        # create the real path
        rel_path = currentdate +".jpg"
        #  join the absolute path and created file name
        abs_file_path = os.path.join(script_dir, rel_path)
        print(abs_file_path)
        #print(currentdate)
        time.sleep(0.1);
    except KeyboardInterrupt:
        print("Keyboard Interrupts. Exiting.")
        exit()