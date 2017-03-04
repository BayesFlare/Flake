from __future__ import print_function
import subprocess
import glob
from time import sleep
import numpy as np
import shutil
import json
import matplotlib.pyplot as plt
import postprocess2 as pps
import posterior_analysis as pa
import FlareGenerator as FG

plotoption=True #Automatically plots flarestuff, unless recursive FITS mode is active
modegood=False
while modegood is False:
    mode=raw_input("What mode would you like to run? ") #0 - Debug, 1 - .FITS, 2 - .txt
    if not mode:
        print("Error. Please input mode 0 (Debug mode), 1 (FITS mode) or 2 (txt mode).")
    elif mode=='exit':
        exit(0)
    elif int(mode)==0:
        print("Entering debug mode, expecting flare_info.JSON in current working directory to create flare using FlareGenerator.py...")
        modegood=True
        mode=int(mode)
    elif int(mode)==1:
        print("Entering FITS mode, expecting FITS file in ./FITS Files/ directory to analyse...")
        modegood=True
        mode=int(mode)
    elif int(mode)==2:
        print("Entering txt mode, expecting txt file in current working directory to analyse...")
        modegood=True
        mode=int(mode)
    else:
        print("Error. Please input mode 0 (Debug mode), 1 (FITS mode) or 2 (txt mode).")

        
filename=[''] #Program expects array of filenames, even if it is a 1x1 array

if mode==0:
    print("Running FlareGenerator.py\n")
    FG.FlareGenerator()
    filen=open('./filename.txt') #file is a command, so I couldn't call the variable file
    filename[0]=filen.read()
    filen.close()
    subprocess.call(["rm", "filename.txt"])

  
if mode==1:
    print("Searching for FITS files in ./FITS Files/...")
    FITSlist=glob.glob('./FITS Files/*.fits')
    if not FITSlist: #checks if FITS list is empty
        print("Fatal error. No FITS files found in /FITS Files/.")
        exit(1)
    if len(FITSlist)>1:
        print('Multiple FITS files found:\n', FITSlist)
        index=int(raw_input('Please enter the index of the file you would like to run, or enter -1 for all files to be run: '))
        if index==-1:
            print("Entering recursive FITS mode")
            filename=FITSlist
            plotoption=False
        else:
            filename[0]=FITSlist[index]
            print('Analysing', filename)
    else:
        filename[0]=FITSlist[0]
        print('Found and analysing', filename)

if mode==2:
    print("Searching for txt files...")
    txtlist=glob.glob('*.txt')
    if not txtlist or txtlist[0]=='README.txt' and len(txtlist)==1: #same as above
        print('Fatal error. No valid txt files found in current directory.')
        exit(1)
    if len(txtlist)>1:
        print('Multiple txt files found:\n', txtlist)
        filename[0]=txtlist[int(raw_input('Please enter the index of the file you would like to run: '))]
        
        print("Analysing", filename)
    else:
        filename[0]=txtlist[0]
        print("Found and analysing", filename)


for u in range(0, len(filename)):

    p_samples=0

    plsen=1000000 #Plateau sensitivity.
    #Bascially how many decimal places it checks of the last two level log likelihoods when comparing the two to
    #deduce whether or not the log likelihoods are still rising. Value must be 10^(decimal places) => bigger, more sensitive

    #Running Flake again
    print("\nInitiating Flake run.")
    flake_process=subprocess.Popen(["./flake", "-d", filename[u], "-f", "flake_settings.json"])

    checktime=300 #How often postprocess is run
    sleeptime=30  #How often the system should report the time remaining
                  #Must be a factor of $checktime
    n_posterior_samples=100 #How many posterior samples should at least be saved before exiting flake

    end = False
    while not end:
        try:
            if p_samples==1:
                print("\nPostprocess error ecountered. Rerunning postprocess in 10 seconds.\n")
                a=checktime-10
            else:
                a=0
            print("\n"+str(checktime-a), "seconds remaining until next postprocess run.\n")
            while a!=checktime:
                sleep(sleeptime)
                a=a+sleeptime
                if checktime-a>0:
                    print("\n"+str(checktime-a), "seconds remaining until next postprocess run.\n")
                else:
                    print("\nInitiating postprocess run...\n")
                    p_samples=pps.postprocess(save=False, plot=False, save_posterior=True)
                    posterior=np.loadtxt('./posterior_sample.txt')
            if p_samples!=1:
                loglh=np.loadtxt("levels.txt")[:, 1]
                if np.floor(loglh[len(loglh)-1]*plsen)==np.floor(loglh[len(loglh)-2]*plsen) and len(posterior)>=n_posterior_samples and len(posterior.shape)!=1:
                    end = True
                    print("\nLog Likelihoods of levels beginning to plateau, exiting flake with "+str(len(posterior))+" samples acquireed. Killing Flake.\n")
                elif len(posterior.shape)!=1:
                    if np.floor(loglh[len(loglh)-1]*plsen)!=np.floor(loglh[len(loglh)-2]*plsen) and len(posterior)<n_posterior_samples:
                        print("\nNot enough posterior samples yet acquired ("+str(n_posterior_samples)+" required, have "+str(len(posterior))+")\nAnd log likelihoods not beginning to plateau yet. (Last two "+str(loglh[len(loglh)-2])+" and "+str(loglh[len(loglh)-1])+".\nContinuing Flake run.\n")
                    elif np.floor(loglh[len(loglh)-1]*plsen)!=np.floor(loglh[len(loglh)-2]*plsen):
                        print("\nEnough posterior samples acquired ("+str(n_posterior_samples)+" required, have "+str(len(posterior))+")\nBut log likelihoods not beginning to plateau yet. (Last two "+str(loglh[len(loglh)-2])+" and "+str(loglh[len(loglh)-1])+".\nContinuing Flake run.\n")
                    elif len(posterior)<n_posterior_samples:
                        print("\nNot enough posterior samples yet acquired ("+str(n_posterior_samples)+" required, have "+str(len(posterior))+")\nBut log likelihoods beginning to plateau. (Last two "+str(loglh[len(loglh)-2])+" and "+str(loglh[len(loglh)-1])+".\nContinuing Flake run.\n")
                elif np.floor(loglh[len(loglh)-1]*plsen)!=np.floor(loglh[len(loglh)-2]*plsen):
                    ("\nNot enough posterior samples yet acquired ("+str(n_posterior_samples)+" required, have 1)\nAnd log likelihoods not beginning to plateau yet. (Last two "+str(loglh[len(loglh)-2])+" and "+str(loglh[len(loglh)-1])+".\nContinuing Flake run.\n")
                else:
                    ("\nNot enough posterior samples yet acquired ("+str(n_posterior_samples)+" required, have 1)\nBut log likelihoods beginning to plateau. (Last two "+str(loglh[len(loglh)-2])+" and "+str(loglh[len(loglh)-1])+".\nContinuing Flake run.\n")

        except(KeyboardInterrupt):
            print(" Keyboard Interrupt. Killing Flake.")
            break
            

    flake_process.kill()


    #Plotting the flarestuff
    
    pa.analysis(posterior=posterior, plot=plotoption, filename=filename[u])
