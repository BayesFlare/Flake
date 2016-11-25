import os.path as path
import subprocess
import time
import sys
import numpy as np
import shutil
import json
import matplotlib.pyplot as plt


def revertoptions(): #Reverts OPTIONS to default values
    default_OPTIONS=('# File containing parameters for DNest3\n', '# Put comments at the top, or at the end of the line.\n', '1	# Number of particles\n', '50000	# new level interval\n','20000	# save interval\n','500	# threadSteps - how many steps each thread should do independently before communication\n','70	# maximum number of levels\n','10	# Backtracking scale length (lambda in the paper)\n','10	# Strength of effect to force histogram to equal push. 0-10 is best. (beta in the paper)\n','2500	# Maximum number of saves (0 = infinite)')    
    options1=open("./OPTIONS.tmp", 'w')
    for i in default_OPTIONS:
        options1.write(i)
    options1.close()
    shutil.move('./OPTIONS.tmp', './OPTIONS')
    print("OPTIONS file returned to default settings.\n")
    return


revertoptions()
import postprocess1 as pps

print("Running FlareGenerator.py\n")
subprocess.call(["python3", "./FlareGenerator.py"])
filen=open('./filename.txt') #file is a command, so I couldn't call the variable file
filename=filen.read()
filen.close()

print("\nRunning first Flake run of two.")
subprocess.call(["rm", "filename.txt"])
flake_process=subprocess.Popen(["./Flake-master/flake", "-d", filename, "-f", "./src/example.json"])



checktime=60 #How often postprocess is run
sleeptime=10 #How often the system should report the time remaining
             #Ideally this would be a factor of $checktime
             
end = False
while end is False:
    try:
        a=0
        print("\n"+str( checktime), "seconds remaining until next postprocess run. (First Flake run of two)\n")
        while a<checktime:
            time.sleep(sleeptime)
            a=a+sleeptime
            if checktime-a>0:
                print("\n"+str(checktime-a), "seconds remaining until next postprocess run. (First Flake run of two)\n")
            else:
                print("\nInitiating postprocess run...\n")
                p_samples=pps.postprocess(save=False, plot=False)
            
        if np.max(p_samples[-10:]) < 1.0e-5:
            end = True
            print("\nThreshold consistently reached. Killing Flake.\nFor reference maximum of last 10 p_samples=", str(np.max(p_samples[-10:])))
        else:
            print("\nThreshold not consistently reached, maximum of last 10 p_samples=", str(np.max(p_samples[-10:]))+". Less than 10^-5 required.\nContinuing Flake run.\n")

    except (KeyboardInterrupt):
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
        a=0
        print("\n"+str( checktime), "seconds remaining until next postprocess run. (Second Flake run of two)\n")
        while a<checktime:
            time.sleep(sleeptime)
            a=a+sleeptime
            if checktime-a>0:
                print("\n"+str(checktime-a), "seconds remaining until next postprocess run. (Second Flake run of two)\n")
            else:
                print("\nInitiating postprocess run...\n")
                pps.postprocess(save=False, plot=False, save_posterior=True)
                posterior=np.loadtxt('./posterior_sample.txt')
            
        if len(samples)>=n_posterior_samples and len(np.shape(posterior))>1:
            end = True
            print("\nAt least "+str(n_posterior_samples)+" samples acquired ("+str(len(posterior))+"). Killing Flake.\n")
        else:
            print("\nAt least "+str(n_posterior_samples)+" samples not yet acquired. Currently "+str(len(posterior))+" samples have been acquired.\nContinuing Flake run.\n")

    except (KeyboardInterrupt):
        print(" Keyboard Interrupt. Killing Flake.")
        break

flake_process.kill()

#On to plotting the flarestuff

plt.ion()
plt.figure(1)

cp=0
for i in range(0, len(posterior)):   
    if int(posterior[i, 5])>cp:
        cp=int(posterior[i, 5])
mnf=0
for i in range(0, len(posterior)):
    if int(posterior[i, 16+(5*(cp-1))])>mnf:
           mnf=int(posterior[i, 16+(5*(cp-1))])

#Need to know cp (Max Change points) and mnf (Max Number of Flares) for 0 padding in posterior.txt


#So far, strictly flares, no impluses

NumFlares=[0]*len(posterior)
for i in range(0, len(posterior)):
    NumFlares[i]=posterior[0, 16+(5*(cp-1))]    
weights=[1/len(NumFlares)]*len(NumFlares)
plt.hist(NumFlares, bins=1, weights=weights)
plt.ylabel('Probability')
plt.xlabel('Num Flares')
plt.title('Number of Flares Distribution')

for j in range(1, int(max(NumFlares)+1)):

    plt.figure(j+1)
    title='Flare '+str(j)+' Parameters'
    plt.suptitle(title)

    #Amplitude
    FlareAmps=[0]*len(posterior)
    for i in range(0, len(posterior)):
        FlareAmps[i]=posterior[i, (22+(5*(cp-1))+(1*(mnf-1))+(j-1))]
    plt.subplot(2,2,1)
    AmpHist=plt.hist(FlareAmps, weights=weights)
    plt.xlabel('Amplitude')
    plt.title('Flare Amplitude')

    #Start Time (t0)
    t0=[0]*len(posterior)
    for i in range(0, len(posterior)):
        t0[i]=posterior[i, (21+(5*(cp-1))+(j-1))]
    plt.subplot(2,2,2)
    t0Hist=plt.hist(t0, weights=weights)
    plt.xlabel('Time (Days)')
    plt.title('Flare Start Time')
    
    #Rise Time
    FlareRise=[0]*len(posterior)
    for i in range(0, len(posterior)):
        FlareRise[i]=(posterior[i, (23+(5*(cp-1))+(2*(mnf-1))+(j-1))])
    plt.subplot(2,2,3)
    RiseHist=plt.hist(FlareRise, weights=weights)
    plt.xlabel('Time (Days)')
    plt.title('Flare Rise Time')

    #Decay Time
    FlareDecay=[0]*len(posterior)
    for i in range(0, len(posterior)):
        FlareDecay[i]=(posterior[i, (24+(5*(cp-1))+(3*(mnf-1))+(j-1))])
    plt.subplot(2,2,4)
    DecayHist=plt.hist(FlareDecay, weights=weights)
    plt.xlabel('Time (Days)')
    plt.title('Flare Decay Time')


    for i in range(1, 5):
        plt.subplot(2,2,i)
        plt.ylabel('Probability')

 
    AmpMPP=0     #MPP Most Probable Probability
    t0MPP=0
    RiseMPP=0
    DecayMPP=0
    AmpMP=0      #MP Most Probable Value
    t0MP=0
    RiseMP=0
    DecayMP=0
    for i in range(0, len(AmpHist[0])):
        if AmpHist[0][1]>AmpMPP:
            AmpMP=(AmpHist[1][i]+AmpHist[1][i+1])/2
            AmpMPP=AmpHist[0][i]
    for i in range(0, len(t0Hist[0])):
        if t0Hist[0][i]>t0MPP:
            t0MP=(t0Hist[1][i]+t0Hist[1][i+1])/2
            t0MPP=t0Hist[0][i]
    for i in range(0, len(RiseHist[0])):
        if RiseHist[0][i]>RiseMPP:
            RiseMP=(RiseHist[1][i]+RiseHist[1][i+1])/2
            RiseMPP=RiseHist[0][i]
    for i in range(0, len(DecayHist[0])):
        if DecayHist[0][i]>DecayMPP:
            DecayMP=(DecayHist[1][i]+DecayHist[1][i+1])/2
            DecayMPP=DecayHist[0][i]
            
    ObservationTime=(t0MP+RiseMP+DecayMP)/2+2 #2 is just a little added extra to make graph look better

    parameters={"FlareParameters":[{"GRT":RiseMP*24, "EDT":DecayMP*24, "Amp":AmpMP, "FStart":t0MP*24, "FlareType":"GRED"}], "GlobalParameters":{"Noise":0, "ObsLen":ObservationTime*24, "Graph": 1}}
    filename="FlakeFoundFlare"+str(j)+".json"
    filen=open(filename, 'w')
    json.dump(parameters, filen)
    filen.close()
plt.ioff()
plt.show()

revertoptions()
