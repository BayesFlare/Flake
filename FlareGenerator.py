import matplotlib.pyplot as plt
import numpy as np
import random
import os.path as path
import json

def FlareGenerator(pathh=0):
   #Read in json file
   if pathh!=0:
      jsonname=pathh
      plt.ion()
      plt.figure(100+int(jsonname[len(jsonname)-6]))
      print("Generating Flare using", jsonname+".")
      jp=True
      jfo=open(jsonname, 'r')

   if path.exists('flare_info.json')==1 and pathh==0:
      jp=True #jp = JSON present
      jfo=open('flare_info.json', 'r') #jfo = JSON file opener
   elif pathh==0:
       jp=False
       print(">flare_info.json not found, randomly generating variables...")
       print("\tDefaulting to Gaussian Rise with Exponential Decay Flare (2), with noise...")

   jf=json.load(jfo)      #jf = JSON file
   jfo.close()
   print(jp)

   #Parameters
   if jp:

      #Observation Length
      if 'ObsLen' in jf['GlobalParameters']:
         observation_length=round((jf['GlobalParameters']['ObsLen']))
      else:
         print('>Observation length not specified in flare_info.json\n\tRandomly generating...')
         observation_length=random.randint(24, 72)

      #NumFlares
      if 'FlareParameters' in jf:
         NumFlares=len(jf['FlareParameters'])
         flaretype=[0]*NumFlares #Used to handle different flare types
      else:
         print('>FlareParameters not present in flare_info.JSON\n\tRandomly generating number of flares...')
         NumFlares=random.randint(1,3)
         flaretype=[0]*NumFlares

      #Graphing
      if 'Graph' in jf['GlobalParameters']:
         if (jf['GlobalParameters']['Graph'])==1: #If graph of flare is to be drawn for human check
            graph=True
         else:
            graph=False
      else:
         print('>Graphing preference not specified in flare_info.json\n\tDefaulting to true...')
         graph=True
   else:
       observation_length=random.randint(24, 72)
       NumFlares=random.randint(1,3)
       flaretype=[2]*NumFlares

   #Initial variable initialisation and assignment

   time=np.arange(0.0, observation_length+0.5, 0.5) #Time axis for flare
   flare=[0.0]*len(time)                            #Light curve
   t0s=[0]*NumFlares                             #Used later to avoid creation of identical flares
   GSTDs=[0]*NumFlares                               #As above
   EDTCs=[0]*NumFlares                               #As above
   pi=np.pi             #Set pi
   e=np.e               #Set e

   #Main start

   #Flare Types -  1: Impulse ; 2: Gaussian Rise with Exponential Decay (GRED) 

   if jp:
       for i in range(0, NumFlares):
          if 'FlareType' in jf['FlareParameters'][i]:
             flaretype[i]=jf['FlareParameters'][i]['FlareType']
          else:
             print('>Flare Type not specified for flare', str(i)+'.\n\tDefaulting to Type 2 (Gaussian Rise and Exponential Decay)')
             flaretype[i]=2

   print('Observation Summary\n\tObservation Length:', observation_length, 'hours\n\tNumber of flares:', NumFlares, '\n\tFlare Type(s):', flaretype)

   badcurve=True #Allow new curves to be generated if one is rejected
   while badcurve:
      if jp:
         if 'Noise' in jf['GlobalParameters']:
            if jf['GlobalParameters']['Noise']!=0:
               noise=True
               noisestd=jf['GlobalParameters']['Noise']
            else:
               noise=False
         else:
            print('Noise preference not specified in flare_info.json\n\tDefaulting to adding noise')
            noise=True
      else:
         noise=True

      if noise:
         for i in range(0, len(flare)): #Generate Gaussian noise
            flare[i]=(noisestd*np.random.randn())
      else:
         for i in range(0, len(flare)):
            flare[i]=0
      for i in range(0, NumFlares):

         ###Flare Type 2
         if flaretype[i]=='GRED':
         #Flare injector v3: Imperfect (realistic), half gaussian, half exponential decay, injector

            #Amplitude
            if jp:
                if 'Amp' in jf['FlareParameters'][i]:
                   amplitude=jf['FlareParameters'][i]['Amp']
                else:
                   print('>Amplitude not specified in flare_info.json\n\tRandomly generating...')
                   amplitude=random.randint(40,60)
            else:
                amplitude=random.randint(40,60)

   #Generate Gauss and Decay


            identical=True
            while identical:     #Avoids creation of identical flares       
               for k in range(0, len(t0s)):
                  t0s[k]=0
                  GSTDs[k]=0
                  EDTCs[k]=0

               AllFalse='false' #Used to pass to random time generator is no times are given in .json file
               if jp:
                  if 't0' in jf['FlareParameters'][i]:
                     t0=round(jf['FlareParameters'][i]['t0'])
                     t0p=True
                     if t0!=jf['FlareParameters'][i]['t0']:
                        t0out=jf['FlareParameters'][i]['t0']
                     else:
                        t0out=t0
                  else:
                     t0p=False
                  if 'GSTD' in jf['FlareParameters'][i]:
                     GSTDP=True
                     GSTD=jf['FlareParameters'][i]['GSTD']
                  else:
                     GSTDP=False
                  if 'EDTC' in jf['FlareParameters'][i]:
                     EDTCP=True
                     EDTC=jf['FlareParameters'][i]['EDTC']
                  else:
                     EDTCP=False

                  if t0p and GSTDP and EDTCP:
                     #do nothing
                     dummy=0 #Dummy variable
                  elif not EDTCP:
                     print('>Exponential Decay time constant not specified in flare_info.json\n\tRandomly generating...')
                     EDTC=random.randint(0, int(len(time)/2))
                  elif not GSTDP:
                     print('>Gaussian standard deviation not specified in flare_info.json\n\tRandomly generating...')
                     GSTD=random.randint(0, EDTC)
                  elif not t0p:
                     print('>Flare mid time not specified in flare_info.json\n\tRandomly generating...')
                     t0=random.randint(0, int(len(time)/2))

               if t0 not in t0s or GSTD not in GSTDs or EDTC not in EDTCs:
                     print('')
                     identical=False
                     t0s[i]=t0             #Avoids creation of identical flares, more of an rng checker
                     GSTDs[i]=GSTD
                     EDTCs[i]=EDTC
                     print('Flare', i+1,'\n\tType: Gaussian Rise and Exponential Decay\n\tAmplitude:', amplitude, '\n\tt0:', t0out/2, 'hours\n\tGaussian Standard Deviation:', GSTD/2, 'hours\n\tExponential Decay Time Constant:', EDTC/2, 'hours')

            for t in range(0, t0):
               flare[t]=flare[t]+amplitude*e**(-(((t-t0)**2)/(2*(GSTD**2))))
            for t in range(t0, len(flare)):
               flare[t]=flare[t]+amplitude*e**(-((t-t0)/EDTC))


         ### Flare Type 1
         if flaretype[i]=='Impulse':

            if jp:
               if 't0' in jf['FlareParameters'][i]:
                  t0=jf['FlareParameters'][i]['t0']
               else:
                  print('>Impulse Time not specified in flare_info.json\n\tRandomly generating')
                  start=random.randint(0, len(time)-1)
            else:
               start=random.randint(0, len(time)-1) 
            if start>len(time)-2:
               print('\n\t>Fatal Error\n\t\tImpulse Flare Time outwith time axes')
               exit()
            #Amplitude
            if jp:
               if 'Amp' in jf['FlareParameters'][i]:
                  amplitude=jf['FlareParameters'][i]['Amp']
               else:
                  print('>Amplitude not specified in flare_info.json\n\tandomly generating...')
                  amplitude=random.randint(40,60)
            else:
               amplitude=random.randint(40,60)

            print('Flare', i+1, '\n\tType: Impulse\n\tAmplitude:', amplitude, '\n\tPeak:', (start+1)/2, 'hours')

            flare[t0]=amplitude
      #Noise
      #Sinusoids
      if "Sinusoids" in jf:
         for i in range(0, len(jf["Sinusoids"])):
            if "Amp" not in jf["Sinusoids"][i]:
               print("Sinusoid", i+1, "Amplitude not specified, randomly generating...")
               sinamp=random.random()*5
            else:
               sinamp=jf["Sinusoids"][i]["Amp"]
            if "Period" not in jf["Sinusoids"][i]:
               print("Sinusoid", i+1, "Period not specified, randomly generating...")
               sinT=(random.random()*12)+5
            else:
               sinT=jf["Sinusoids"][i]["Period"]
            if "Phase" not in jf["Sinusoids"][i]:
               print("Sinusoid", i+1, "Phase not specified, randomly generating...")
               sinP=(random.random())*sinT
            else:
               sinP=jf["Sinusoids"][i]["Phase"]

            for j in range(0, len(time)):
               flare[j]=flare[j]+(sinamp*np.sin((time[j]*2*pi)/sinT))

      #ChangePoints
      if "ChangePoints" in jf:
         for i in range(0, len(jf["ChangePoints"])):
            if "t0" not in jf["ChangePoints"][i]:
               print("Dropout", i+1, "time not specified, randomly generating...")
               dropt0=(round(random.random()*max(time)*2))/2
            elif jf["ChangePoints"][i]["t0"]>max(time):
               print("Dropout", i+1, "time outwith observation length, randomly generating one that is within...")
               dropt0=(round(random.random()*max(time)*2))/2
            else:
               dropt0=(round(jf["ChangePoints"][i]["t0"]*2))/2

            if "Amp" not in jf["ChangePoints"][i]:
               print("Dropout", i+1, "amplitude not specified, randomly generating...")
               dropamp=random.random()*4
            else:
               dropamp=jf["ChangePoints"][i]["Amp"]


            for j in range(int(dropt0*2), len(time)):
               flare[j]=flare[j]+dropamp

      #Transit
      if "Transits" in jf:
         G=6.673*(10**-11)
         for i in range(0, len(jf["Transits"])):
            if "Rs" not in jf["Transits"][i]:
               print("Stellar radius", i+1, "not specified, defaulting to Solar radius.")
               Rs=6.96*(10**8)
            else:
               Rs=jf["Transits"][i]["Rs"]
            if "Rp" not in jf["Transits"][i]:
               print("Planetary radius", i+1, "not specified, defaulting to Jovian radius.")
               Rp=6.37814*(10**6)*11.2
            else:
               Rp=jf["Transits"][i]["Rp"]
            if "Ms" not in jf["Transits"][i]:
               print("Stellar mass", i+1, "not specified, defaulting to Solar mass.")
               Ms=1.9891*(10**30)
            else:
               Ms=jf["Transits"][i]["Ms"]
            if "Ro" not in jf["Transits"][i]:
               print("Transiting planet", str(i+1)+"'s orbital radius not specified, defaulting to 1 AU.")
               Ro=1.495979*(10**11)
            else:
               Ro=jf["Transits"][i]["Ro"]
            if "t0" not in jf["Transits"][i]:
               print("Transit", i+1, "start time not specified, defaulting to middle of observation.")
               t0i=int(floor(max(time)))
            else:
               t0i=int((jf["Transits"][i]["t0"])*2) #t0 index: Multiply by 2 for index
            t0is=t0i #Time nought index start
            t0i=t0i-1

            thetastart=np.arccos(Rs/Ro)
            vmax=np.sqrt((G*Ms)/Ro)
            d=Rs+Rp-1
            while d>0:
               t0i=t0i+1
               if t0i>=len(flare):
                  break
               if d<=Rs-Rp:
                  s=pi*(Rp**2)
               else:
                  s=(Rp**2)*np.arccos(((d**2)+(Rp**2)-(Rs**2))/(2*d*Rp))+(Rs**2)*np.arccos(((d**2)+(Rs**2)-(Rp**2))/(2*d*Rs))-0.5*(np.sqrt(((-d+Rp+Rs)*(d+Rp-Rs)*(d-Rp+Rs)*(d+Rp+Rs))))
               flare[t0i]=(flare[t0i])*(((pi*(Rs**2))-s)/(pi*(Rs**2)))
               d=d-(1800*(vmax*np.sin((vmax/Ro)+thetastart))) #This is per second, so *1800 for per half hour

            for k in range(t0i, 2*t0i-t0is):
               if k>=len(flare):
                  break
               if d<=Rs-Rp:
                  s=pi*(Rp**2)
               else:
                  s=(Rp**2)*np.arccos((d**2+Rp**2-Rs**2)/(2*d*Rp))+(Rs**2)*np.arccos((d**2+Rs**2-Rp**2)/(2*d*Rs))-0.5*(np.sqrt(((-d+Rp+Rs)*(d+Rp-Rs)*(d-Rp+Rs)*(d+Rp+Rs))))
               flare[k]=flare[k]*(((pi*(Rs**2))-s)/(pi*(Rs**2)))
               d=abs(d-(1800*(vmax*np.sin((vmax/Ro)+thetastart))))



      for i in range(0, len(time)):
         time[i]=time[i]/24
      if graph:
         plt.plot(time, flare)
         plt.xlabel('Time (Days)')              #Plots curve(s) for human confirmation
         plt.ylabel('Intensity')
         if NumFlares>1:
            plt.title('Stellar Flares')
         else:
            plt.title('Stellar Flare')
         plt.show()


      badinput=True                                      
      while badinput: #Allows program to get the answer it requires to continue
         if graph:
            use=input('Do you wish to save this/these flare(s) as an ASCII file? (y/n) ')
         else:
            use='y'

         if use=='n' and jp:
            badinput=False
            badcurve=False
         elif use=='n':
            badinput=False
            print('\nYou selected no, generating new curve(s)...\n')
         elif use=='exit':
            exit()
         elif use=='y':
            badinput=False
            badcurve=False
            file_name='flare-'+str(NumFlares)+'f-'+str(observation_length)+'h.txt'
            name_exists=True #Avoiding overwriting previously saved same name files
            i=1
            while name_exists:  
               if path.exists(file_name)==1:
                  file_name='flare-'+str(NumFlares)+'f-'+str(observation_length)+'h('+str(i)+').txt'
                  i=i+1
               else:
                  name_exists=False

            output=open(file_name, 'w') #Writing data to .txt file
            print(">> Writing data to file")
            output.write('#Time (Days)\tSignal\n')
            for i in range(0, len(time)):
               out=str(time[i])+'\t'+str(flare[i])+'\n'
               output.write(out)
            output.close()
            if path.exists(file_name)==1:
               print('\tSuccess!')
               fileoutname=open('./filename.txt', 'w')
               fileoutname.write(file_name)
               fileoutname.close()
            else:
               print("Unexpected error, file somehow not present in current path.")
               exit()
         else:
            print('Error: Please input y or n only. Or to exit without txt file, type "exit"')
