import os.path as path
import subprocess
import time
import sys
import numpy as np

sys.path.insert(0, './src/') #To access flake

import postprocess1 as pps


print("Running FlareGenerator.py\n")
subprocess.call(["python3", "./FlareGenerator.py"])
filen=open('./filename.txt') #file is a command, so I couldn't call the variable file
filename=filen.read()
filen.close()
subprocess.call(["rm", "filename.txt"])
flake_process=subprocess.Popen(["./Flake-master/flake", "-d", filename])



checktime=60 #How often postprocess is run
sleeptime=10 #How often the system should report the time remaining
             #Ideally this would be a factor of $checktime
threshold=1  #1 will never be reached, here for now

             
end = False
while end is False:
    try:
        a=0
        print(checktime, "seconds remaining until next postprocess run")
        while a<checktime:
            time.sleep(sleeptime)
            a=a+sleeptime
            print(checktime-a, "seconds remaining until next postprocess run")
        logz_estimate, H_estimate, logx_samples=pps.postprocess(save=False)

        if logz_estimate>threshold:
            end = True
            print("Threshold reached. Killing Flake.")


    except (KeyboardInterrupt):
        break
        print("Keyboard Interrupt. Killing Flake.")


flake_process.kill()
