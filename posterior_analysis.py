import numpy as np
import matplotlib.pyplot as plt
import json
import os.path as path
import subprocess

def analysis(plot=True, posterior=0):

    try:
        if posterior==0:
            print("Critical failure. No posterior argument passed to posterior_analyis.")
            exit(1)
    except:
        ValueError

    if not path.exists("./Flake Found Objects/"):
        print("Flake Found Objects directory nonexistant. Creating directory...")
        subprocess.call(["mkdir", "Flake Found Objects"])
        if not path.exists("./Flake Found Objects/"):
            print("Critical Error. Failed to create directory.")

    cp=0
    NumCPDist=[0]*len(posterior)
    for i in range(0, len(posterior)):
        NumCPDist[i]=posterior[i,3]
        if int(posterior[i, 3])>cp:
            cp=int(posterior[i, 3])
    mns=0
    NumSinDist=[0]*len(posterior)
    for i in range(0, len(posterior)):
        NumSinDist[i]=posterior[i, 9+(2*(cp-1))]
        if int(posterior[i, 9+(2*(cp-1))]):
            mns=int(posterior[i, 9+(2*(cp-1))])
    mnf=0
    NumFlaresDist=[0]*len(posterior)
    for i in range(0, len(posterior)):
        NumFlaresDist[i]=posterior[i, 16+(2*(cp-1))+(3*(mns-1))]
        if int(posterior[i, 16+(2*(cp-1))+(3*(mns-1))])>mnf:
               mnf=int(posterior[i, 16+(2*(cp-1))+(3*(mns-1))])
    mni=0
    NumImpulseDist=[0]*len(posterior)
    for i in range(0, len(posterior)):
        NumImpulseDist[i]=posterior[i, 26+(2*(cp-1))+(3*(mns-1))+(4*(mnf-1))]
        if int(posterior[i, 26+(2*(cp-1))+(3*(mns-1))+(4*(mnf-1))])>mni:
            mni=int(posterior[i, 26+(2*(cp-1))+(3*(mns-1))+(4*(mnf-1))])

    #Need to know cp (Max Change points) and mnf (Max Number of Flares) ans mns (Max Number of Sinusoids) for 0 padding in posterior.txt


    #Flare Section

    plt.ion()
    plt.figure(1)

    
    weights=[1/len(NumFlaresDist)]*len(NumFlaresDist)
    if plot:
        plt.hist(NumFlaresDist, bins=1, weights=weights)
        plt.ylabel('Probability')
        plt.xlabel('Num Flares')
        plt.title('Number of Flares Distribution')

    for j in range(1, mnf+1):

        plt.figure(j+1)
        title='Flare '+str(j)+' Parameters'
        plt.suptitle(title)

        #Amplitude
        FlareAmps=[0]*len(posterior)
        for i in range(0, len(posterior)):
            FlareAmps[i]=posterior[i, (22+(2*(cp-1))+(3*(mns-1))+(j-1))]
        plt.subplot(2,2,1)
        AmpHist=plt.hist(FlareAmps, weights=weights)
        plt.xlabel('Amplitude')
        plt.title('Flare Amplitude')

        #Start Time (t0)
        t0=[0]*len(posterior)
        for i in range(0, len(posterior)):
            t0[i]=posterior[i, (21+(2*(cp-1))+(3*(mns-1))+(j-1))]
        plt.subplot(2,2,2)
        t0Hist=plt.hist(t0, weights=weights)
        plt.xlabel('Time (Days)')
        plt.title('Flare Start Time')

        #Rise Time
        FlareRise=[0]*len(posterior)
        for i in range(0, len(posterior)):
            FlareRise[i]=(posterior[i, (23+(2*(cp-1))+(3*(mns-1))+(2*(mnf-1))+(j-1))])
        plt.subplot(2,2,3)
        RiseHist=plt.hist(FlareRise, weights=weights)
        plt.xlabel('Time (Days)')
        plt.title('Flare Rise Time')

        #Decay Time
        FlareDecay=[0]*len(posterior)
        for i in range(0, len(posterior)):
            FlareDecay[i]=(posterior[i, (24+(2*(cp-1))+(3*(mns-1))+(3*(mnf-1))+(j-1))])
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
            if AmpHist[0][i]>AmpMPP:
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


        t0MP=t0MP*48
        DecayMP=DecayMP*48
        RiseMP=RiseMP*48
        ObservationTime=(t0MP+RiseMP+DecayMP)+2

        parameters={"FlareParameters":[{"GSTD":RiseMP, "EDTC":DecayMP, "Amp":AmpMP, "t0":t0MP, "FlareType":"GRED"}], "GlobalParameters":{"Noise":0, "ObsLen":ObservationTime, "Graph": 1}}
        filename="./Flake Found Objects/FlakeFoundFlare"+str(j)+".json"
        filen=open(filename, 'w')
        json.dump(parameters, filen)
        filen.close()
        badinput=True
        while badinput:
            prompt="Do you wish to plot flare "+str(j)+" isolated using FlareGenerator.py? (y/n) "
            plott=input(prompt)
            if plott=='y' or plott=='n':
                badinput=False
            else:
                print("Please enter y or n only")
        if plott=='y':
            FG

    #Impulse Section

    plt.figure(mnf+2)

    weights=[1/len(NumImpulseDist)]*len(NumImpulseDist)
    if plot:
        plt.hist(NumImpulseDist, bins=1, weights=weights)
        plt.ylabel('Probability')
        plt.xlabel('Num Impulses')
        plt.title('Number of Impulses Distribution')

    for j in range(1, mni+1):

        plt.figure(mnf+2+j)
        title='Impulse '+str(j)+' Parameters'
        plt.suptitle(title)

        #t0
        t0=[0]*len(posterior)
        for i in range(0, len(posterior)):
            t0[i]=(posterior[i, 29+(2*(cp-1))+(3*(mns-1))+(4*(mnf-1))+(j-1)])/2
        plt.subplot(1,2,1)
        t0Hist=plt.hist(t0, weights=weights)
        plt.xlabel('Time')
        plt.title('Impulse Time')

        #Amp
        ImpAmp=[0]*len(posterior)
        for i in range(0, len(posterior)):
            ImpAmp[i]=posterior[i, 29+(2*(cp-1))+(3*(mns-1))+(4*(mnf-1))+(mni-1)+(j-1)]
        plt.subplot(1,2,2)
        AmpHist=plt.hist(ImpAmp, weights=weights)
        plt.xlabel('Amplitude')
        plt.title('Impulse Amplitude')

        for i in range(1,3):
            plt.subplot(1,2,i)
            plt.ylabel('Probability')

        AmpMPP=0   #As before
        t0MPP=0
        AmpMP=0
        t0MP=0

        for i in range(0, len(AmpHist[0])):
            if AmpHist[0][i]>AmpMPP:
                AmpMP=(AmpHist[1][i]+AmpHist[1][i+1])/2
                AmpMPP=AmpHist[0][i]
        for i in range(0, len(t0Hist[0])):
            if t0Hist[0][i]>t0MPP:
                t0MP=(t0Hist[1][i]+t0Hist[1][i+1])/2
                t0MPP=t0Hist[0][i]

        ObservationTime=t0MP+4

        parameters={"FlareParameters":[{"Amp":AmpMP, "t0":t0MP, "FlareType":"Impulse"}], "GlobalParameters":{"Noise":0, "ObsLen":ObservationTime, "Graph": 1}}
        filename="./Flake Found Objects/Impulse"+str(j)+".json"
        filen=open(filename, 'w')
        json.dump(parameters, filen)
        filen.close()

    #Sinusoid Section

    plt.figure(mnf+2+mni+2)
    weights=[1/len(NumSinDist)]*len(NumSinDist)
    if plot:
        plt.hist(NumSinDist, bins=1, weights=weights)
        plt.ylabel('Probability')
        plt.xlabel('Num Sinusoids')
        plt.title('Number of Sinusoids Distribution')

    for j in range(1, mns+1):

        plt.figure(mnf+2+mni+2+j)
        title='Sinusoid '+str(j)+' Parameters'
        plt.suptitle(title)

        #Period
        SinP=[0]*len(posterior)
        for i in range(0, len(posterior)):
            SinP[i]=np.e**(posterior[i, 12+2*(cp-1)])
            #e** as this is the log(period)
        plt.subplot(2,2,1)
        PHist=plt.hist(SinP, weights=weights)
        plt.xlabel('Phase (bins)')
        plt.title('Sinusoid  Phase')

        #Amp
        SinAmp=[0]*len(posterior)
        for i in range(0, len(posterior)):
            SinAmp[i]=posterior[i, 13+2*(cp-1)+(mns-1)]
        plt.subplot(2,2,2)
        AmpHist=plt.hist(SinAmp, weights=weights)
        plt.xlabel('Amplitude')
        plt.title('Sinusoid Amplitude')

        #Phase (In radians. How many bins is one radian?)
        SinPhase=[0]*len(posterior)
        for i in tange(0, len(posterior)):
            SinPhase[i]=posterior[i, 14+2*(cp-1)+2*(mns-1)]
        plt.subplot(2,2,3)
        PhaseHist=plt.hist(SinPhase, weights=weights)
        plt.xlabel('Phase')
        plt.title('Sinusoid Phase')

        for i in range(1,4):
            plt.subplot(2,2,i)
            plt.ylabel('Probability')

        AmpMPP=0        #As before
        PeriodMPP=0
        PhaseMPP=0
        AmpMP=0
        PeriodMP=0
        PhaseMP=0

        for i in range(0, len(AmpHist[0])):
            if AmpHist[0][i]>AmpMPP:
                AmpMP=(AmpHist[1][i]+AmpHist[1][i+1])/2
                AmpMPP=AmpHist[0][i]
        for i in range(0, len(PHist[0])):
            if PHist[0][i]>PeriodMPP:
                PeriodMP=(PHist[1][i]+PHist[1][i+1])/2
                PeriodMPP=PHist[0][i]
        for i in range(0, len(PhaseHist)):
            if PhaseHist[0][i]>PhaseMPP:
                PhaseMP=(PhaseHist[1][i]+PhaseHist[1][i+1])/2
                PhaseMPP=PhaseHist[0][i]

        parameters={"FlareParameters":[{"AmpMP":0, "t0":0, "FlareType":"N/A"}], "GlobalParameters":{"Noise":0, "Obslen":24, "Graph": 1}, "Sinusoids":[{"Period":PeriodMP, "Phase":PhaseMP, "Amp":AmpMP}]}
        filename="./Flake Found Objects/Sinusoid"+str(j)+".json"
        filen=open(filename, 'w')
        json.dump(parameters, filen)
        filen.close()

    #Change Points Section
    plt.figure(mnf+2+mni+2+mns+2)
    weights=[1/len(NumCPDist)]*len(NumCPDist)
    if plot:
        plt.hist(NumCPDist, bins=1, weights=weights)
        plt.ylabel('Probability')
        plt.xlabel('Num Change Points')
        plt.title('Number of Change Points Distribution')

    for j in range(1, cp+1):

        plt.figure(mnf+2+mni+2+mns+2+j)
        title='Change Point '+str(j)+' Parameters'
        plt.suptitle(title)

        #t0
        t0=[0]*len(posterior)
        for i in range(0, len(posterior)):
            t0[i]=(posterior[i, 6+(j-1)])/2
        plt.subplot(1,2,1)
        t0Hist=plt.hist(t0, weights=weights)
        plt.xlabel('Time')
        plt.title('Change Point Time')

        #Amp
        Amp=[0]*len(posterior)
        for i in range(0, len(posterior)):
            Amp[i]=posterior[i, 6+cp+(j-1)]
        plt.subplot(1,2,2)
        AmpHist=plt.hist(Amp, weights=weights)
        plt.xlabel('Amplitude')
        plt.title('Change Point Amplitude')

        for i in range(1,3):
            plt.subplot(1,2,i)
            plt.ylabel('Probability')

        t0MPP=0     #As before
        AmpMPP=0
        t0MP=0
        AmpMP=0

        for i in range(0, len(t0Hist[0])):
            if t0Hist[0][i]>t0MPP:
                t0MP=(t0Hist[1][i]+t0Hist[1][i+1])/2
                t0MPP=t0Hist[0][i]
        for i in range(0, len(AmpHist[0])):
            if AmpHist[0][i]>AmpMPP:
                AmpMP=(AmpHist[1][i]+AmpHist[1][i+1])/2
                AmpMPP=AmpHist[0][i]

        parameters={"FlareParameters":[{"AmpMP":0, "t0":0, "FlareType":"N/A"}], "GlobalParameters":{"Noise":0, "Obslen":24, "Graph": 1}, "ChangePoints":[{"t0":t0MP, "Amp":AmpMP}]}
        filename="./Flake Found Objects/ChangePoint"+str(j)+".json"
        filen=open(filename, 'w')
        json.dump(parameters, filen)
        filen.close()

        #Noise Section

        weights=[1/len(posterior)]*len(posterior)
        plt.figure(mnf+2+mni+2+mns+2+cp+2)
        Noise=[0]*len(posterior)
        for i in range(0, len(posterior)):
            Noise[i]=posterior[i, 0]
        NoiseHist=plt.hist(Noise, weights=weights)
        plt.xlabel('Standard Deviation')
        plt.ylabel('Probability')
        plt.title('Noise Standard Deviation')

        nstdMP=0     #As before
        nstdMPP=0

        for i in range(0, len(NoiseHist[0])):
            if NoiseHist[0][i]>nstdMPP:
                nstdMP=(NoiseHist[1][i]+NoiseHist[1][i+1])/2
                nstdMPP=NoiseHist[0][i]

        parameters={"FlareParameters":[{"AmpMP":0, "t0":0, "FlareType":"N/A"}], "GlobalParameters":{"Noise":nstdMP, "Obslen":24, "Graph": 1}}
        filename="./Flake Found Objects/Noise.json"
        filen=open(filename, 'w')
        json.dump(parameters, filen)
        filen.close()

    if plot:
        plt.ioff()
        plt.show()

analysis(posterior=np.loadtxt('./posterior_sample.txt'))
