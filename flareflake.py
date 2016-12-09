import subprocess
import time
import numpy as np
import shutil
import json
import matplotlib.pyplot as plt
import postprocess1 as pps
import FlareGenerator as FG
import posterior_analysis as pa

def revertoptions(initial=False, default_OPTIONS=''): #Reverts OPTIONS to original values
    if initial:
        default_OPTIONS=(open("./OPTIONS", "r")).read()
        return default_OPTIONS
        
    if (open("./OPTIONS", "r")).read()!=default_OPTIONS:
        options1=open("./OPTIONS.tmp", 'w')
        for i in default_OPTIONS:
            options1.write(i)
        options1.close()
        shutil.move('./OPTIONS.tmp', './OPTIONS')
        print("OPTIONS file returned to default settings.\n")
        return


default_OPTIONS=revertoptions(initial=True)


print("Running FlareGenerator.py\n")
FG
filen=open('./filename.txt') #file is a command, so I couldn't call the variable file
filename=filen.read()
filen.close()

print("\nRunning first Flake run of two.")
subprocess.call(["rm", "filename.txt"])
flake_process=subprocess.Popen(["./Flake-master/flake", "-d", filename, "-f", "./src/example.json"])



checktime=60 #How often postprocess is run
sleeptime=10 #How often the system should report the time remaining
             #Ideally this would be a factor of $checktime
p_samples=0
end = False
while end is False:
    try:
        if p_samples==1:
            print("\nPostprocess error ecountered. Rerunning postprocess in 10 seconds.\n")
            a=checktime-10
        else:
            a=0
            print("\n"+str(checktime-a), "seconds remaining until next postprocess run. (First Flake run of two)\n")
        while a<checktime:
            time.sleep(sleeptime)
            a=a+sleeptime
            if checktime-a>0:
                print("\n"+str(checktime-a), "seconds remaining until next postprocess run. (First Flake run of two)\n")
            else:
                print("\nInitiating postprocess run...\n")
                p_samples=pps.postprocess(save=False, plot=False)
        if p_samples!=1:
            if np.max(p_samples[-10:]) < 1.0e-5:
                end = True
                print("\nThreshold consistently reached. Killing Flake.\nFor reference maximum of last 10 p_samples=", str(np.max(p_samples[-10:])))
            else:
                print("\nThreshold not consistently reached, maximum of last 10 p_samples=", str(np.max(p_samples[-10:]))+". Less than 10^-5 required.\nContinuing Flake run.\n")

    except(KeyboardInterrupt):
        print(" Keyboard Interrupt. Killing Flake.")
        break

flake_process.kill()

#Rewriting Options

samples=np.loadtxt("./sample.txt")
nlevels=len(samples)

options=open("./OPTIONS", "r")
options_data=options.readlines()
options.close()

options_data[6]=str(nlevels)+"\t# maximum number of levels\n"
print("Rewriting OPTIONS file.")
options_tmp=open("./OPTIONS.tmp", 'w')

for i in options_data:
    options_tmp.write(i)
options_tmp.close()

shutil.move('./OPTIONS.tmp', './OPTIONS')

options=open("./OPTIONS", "r")
options_data_new=options.readlines()
for i in range(0, len(options_data)):
    if options_data_new[i]!=options_data[i]:
        print("Critical Failure: OPTIONS file not succesfully rewritten.")
        exit()
print("OPTIONS file succesfully rewritten!")

#Running Flake again
print("\nInitiating second Flake run of two.")
flake_process=subprocess.Popen(["./Flake-master/flake", "-d", filename, "-f", "./src/example.json"])

checktime=120 #How often postprocess is run
sleeptime=10  #How often the system should report the time remaining
              #Ideally this would be a factor of $checktime
n_posterior_samples=150 #How many posterior samples should at least be saved before exiting flake
             
end = False
while end is False:
    try:
        if p_samples==1:
            print("\nPostprocess error ecountered. Rerunning postprocess in 10 seconds.\n")
            a=checktime-10
        else:
            a=0
        print("\n"+str(checktime-a), "seconds remaining until next postprocess run. (Second Flake run of two)\n")
        while a<checktime:
            time.sleep(sleeptime)
            a=a+sleeptime
            if checktime-a>0:
                print("\n"+str(checktime-a), "seconds remaining until next postprocess run. (Second Flake run of two)\n")
            else:
                print("\nInitiating postprocess run...\n")
                p_samples=pps.postprocess(save=False, plot=False, save_posterior=True)
                posterior=np.loadtxt('./posterior_sample.txt')
        if p_samples!=1:   
            if len(posterior)>=n_posterior_samples and len(np.shape(posterior))>1:
                end = True
                print("\nAt least "+str(n_posterior_samples)+" samples acquired ("+str(len(posterior))+"). Killing Flake.\n")
            else:
                print("\nAt least "+str(n_posterior_samples)+" samples not yet acquired. Currently "+str(len(posterior))+" samples have been acquired.\nContinuing Flake run.\n")

    except (KeyboardInterrupt):
        print(" Keyboard Interrupt. Killing Flake.")
        break

flake_process.kill()


#Plotting the flarestuff

pa.analysis(plot=False, posterior=posterior)


revertoptions(default_OPTIONS=default_OPTIONS)
