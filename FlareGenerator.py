import matplotlib.pyplot as plt
import numpy as np
import random
import os.path as path
import json

#Read in json file
if path.exists('FlakeFoundFlare.json')==1:
   jp='true'
   jfo=open('FlakeFoundFlare.json', 'r')
   jf=json.load(jfo)
   jfo.close()

elif path.exists('flare_info.json')==1:
   jp='true' #jp = JSON present
   jfo=open('flare_info.json', 'r') #jfo = JSON file opener
   jf=json.load(jfo)      #jf = JSON file
   jfo.close()
else:
    jp='false'
    print(">flare_info.json not found, randomly generating variables...")
    print("\tDefaulting to Gaussian Rise with Exponential Decay Flare (2), with noise...")

#Parameters
if jp=='true':

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
      graph_true=(jf['GlobalParameters']['Graph']) #If graph of flare is to be drawn for human check
   else:
      print('>Graphing preference not specified in flare_info.json\n\tDefaulting to true...')
      graph_true=1
else:
    observation_length=random.randint(24, 72)
    NumFlares=random.randint(1,3)
    flaretype=[2]*NumFlares

#Initial variable initialisation and assignment


time=np.arange(0.0, observation_length+0.5, 0.5) #Time axis for flare
flare=[0.0]*len(time)                            #Light curve
zeroliney=[0,0]                                  #Used to generate 0 line
zerolinex=[0, observation_length/24]             #As above
starts=[0]*NumFlares                             #Used later to avoid creation of identical flares
mids=[0]*NumFlares                               #As above
ends=[0]*NumFlares                               #As above
pi=np.pi             #Set pi
e=np.e               #Set e
y=1000               #Size of smooth arrays
x=np.arange(0, y)    #Generates x space to generate smooth dist. over
fxg=[0]*y            #Generates array for gaussian rise flare dist
fxed=[0]*y           #Generates array for exponential decay flare dist

#Main start

#Flare Types -  1: Impulse ; 2: Gaussian Rise with Exponential Decay (GRED) 

if jp=='true':
    for i in range(0, NumFlares):
       if 'FlareType' in jf['FlareParameters'][i]:
          flaretype[i]=jf['FlareParameters'][i]['FlareType']
       else:
          print('>Flare Type not specified for flare', str(i)+'.\n\tDefaulting to Type 2 (Gaussian Rise and Exponential Decay)')
          flaretype[i]=2

print('Observation Summary\n\tObservation Length:', observation_length, 'hours\n\tNumber of flares:', NumFlares, '\n\tFlare Type(s):', flaretype)

goodcurve='false' #Allow new curves to be generated if one is rejected
while goodcurve=='false':
   if jp=='true':
      if 'Noise' in jf['GlobalParameters']:
         if jf['GlobalParameters']['Noise']==1:
            noise='true'
         else:
            noise='false'
      else:
         print('Noise preference not specified in flare_info.json\n\tDefaulting to adding noise')
         noise='true'
   else:
      noise='true'
      
   if noise=='true':
      for i in range(0, len(flare)): #Generate Gaussian noise
         flare[i]=(2*random.random())-1
   else:
      for i in range(0, len(flare)):
         flare[i]=0
   
   for i in range(0, NumFlares):

      ###Flare Type 2
      if flaretype[i]=='GRED':
      #Flare injector v3: Imperfect (realistic), half gaussian, half exponential decay, injector

         #Amplitude
         if jp=='true':
             if 'Amp' in jf['FlareParameters'][i]:
                amplitude=jf['FlareParameters'][i]['Amp']
             else:
                print('>Amplitude not specified in flare_info.json\n\tRandomly generating...')
                amplitude=random.randint(40,60)
         else:
             amplitude=random.randint(40,60)

         #Generate half Gauss  
         gauss_sig=300
         for j in range(0, len(fxg)):
             fxg[j]=amplitude*(np.exp(((x[j]-999)**2)/(-2*((gauss_sig)**2))))

         #Generate half decay
         k=(np.log(amplitude)-np.log(1))/(len(x))

         for j in x:
             fxed[j]=amplitude*np.exp(-k*x[j])


         identical='true'
         while identical=='true':     #Avoids creation of identical flares       
            for k in range(0, len(starts)):
               starts[k]=0
               mids[k]=0
               ends[k]=0

            AllFalse='false' #Used to pass to random time generator is no times are given in .json file
            if jp=='true':
               if 'FStart' in jf['FlareParameters'][i]:
                  g_start=round((jf['FlareParameters'][i]['FStart']))
                  FStart='true'
               else:
                  FStart='false'
               if 'GRT' in jf['FlareParameters'][i]:
                  GRT='true'
                  mid=(round(jf['FlareParameters'][i]['GRT']))+g_start
               else:
                  GRT='false'
               if 'EDT' in jf['FlareParameters'][i]:
                  EDT='true'
                  ed_end=(round(jf['FlareParameters'][i]['EDT']))+mid
               else:
                  EDT='false'

               if FStart=='true' and GRT=='true' and EDT=='true':
                  #do nothing
                  dummy=0 #Dummy variable
               elif FStart=='true' and  GRT=='true' and EDT=='false':
                  print('>Exponential Decay time not specified in flare_info.json\n\tRandomly generating...')
                  ed_end=random.randint(mid, len(time)-2)
               elif FStart=='true' and GRT=='false' and EDT=='true':
                  print('>Gaussian Rise time not specified in flare_info.json\n\tRandomly generating...')
                  mid=random.randint(g_start, ed_end)
               elif FStart=='false' and GRT=='true' and EDT=='true':
                  print('>Flare Start time not specified in flare_info.json\n\tRandomly generating...')
                  g_start=random.randint(0, mid)
               elif FStart=='false' and GRT=='false' and EDT=='true':
                  print('>Flare Start and Gaussian Rise times not specified in flare_info.json\n\tRandomly generating...')
                  a=random.randint(0, ed_end)
                  b=random.randint(0, ed_end)
                  if a>b:
                     mid=a
                     g_start=b
                  elif b>a:
                     mid=b
                     g_start=a
                  else:
                     mid=ed_end-6
                     g_start=mid-3
               elif FStart=='true' and GRT=='false' and EDT=='false':
                  print('>Gaussian Rise and Exponential Decay times not specified in flare_info.json\n\tRandomly generating...')
                  a=random.randint(g_start, len(time)-1)
                  b=random.randint(g_start, len(time)-1)
                  if a>b:
                     mid=b
                     ed_end=a
                  elif b>a:
                     mid=a
                     ed_end=b
                  else:
                     mid=g_start+3
                     ed_end=mid+6
               elif FStart=='false' and GRT=='true' and EDT=='false':
                  print('>Flare Start and Exponential Decay times not specified in flare_info.json\n\tRandomly generating...')
                  g_start=random.randint(0, mid)
                  ed_end=random.randint(mid+1, len(time)-2)
               else:
                  print('>Flare Start, Gaussian Rise and Exponential Decay times not specified in flare_info.json\n\tRandomly generating...')
                  AllFalse='true'

            if AllFalse=='true' or jp=='false':                
               a=random.randint(0, len(time)-1) #Generates flare duration if all unspecified
               b=random.randint(0, len(time)-1)
               c=random.randint(0, len(time)-1)

               if a+1<b and a+1<c:
                  g_start=a
                  if b+1<c:
                     mid=b
                     ed_end=c
                  elif c+1<b:
                     mid=c
                     ed_end=b
                  else:
                     mid=b
                     ed_end=b+3

               elif b+1<a and b+1<c:
                  g_start=b
                  if a+1<c:
                     mid=a
                     ed_end=c
                  elif c+1<a:
                     mid=c
                     ed_end=a
                  else:
                     mid=a
                     ed_end=a+3

               elif c+1<a and c+1<b:
                  g_start=c
                  if a+1<b:
                     mid=a
                     ed_end=b
                  elif b+1<a:
                     mid=b
                     ed_end=a
                  else:
                     mid=a
                     ed_end=a+3

               else:
                  g_start=a
                  mid=a+3
                  ed_end=a+6

            if jp=='true':
               if g_start>len(flare)-1 or mid>len(flare)-1 or ed_end>len(flare)-1:
                  print('\n\tFatal Error\n\t\tType 2 Flare FStart, GRT or EDT cause flare to exceed time axes')
               
            if g_start>len(flare)-1 or mid>len(flare)-1 or ed_end>len(flare)-1:
               indentical='true' #Generate again
            elif g_start not in starts or mid not in mids or ed_end not in ends:
                  print('')
                  identical='false'
                  starts[i]=g_start             #Avoids creation of identical flares, more of an rng checker
                  mids[i]=mid
                  ends[i]=ed_end
                  print('Flare', i+1,'\n\tType: Gaussian Rise and Exponential Decay\n\tAmplitude:', amplitude, '\n\tStart Time:', g_start/2, 'hours\n\tGaussian Rise Length:', (mid-g_start)/2, 'hours\n\tExponential Decay Length:', (ed_end-mid)/2, 'hours')
         sample_start=random.randint(0, int(np.floor(len(fxg)/(mid-g_start)))) #Generates gaussian sample start  


         if sample_start==0:    #Samples smooth dists and injects them as flare(s)
            for j in range(g_start, mid+1):
               flare[j]=flare[j]+fxg[int((np.floor((len(fxg)-1)/(mid-g_start)))*(j-g_start))]
               
            for j in range(mid+1, ed_end):
               flare[j]=flare[j]+fxed[int(np.floor((len(fxed)-1)/(ed_end-mid-1)))*(j-mid-1)]
               
         else:
            for j in range(g_start, mid):
               flare[j]=flare[j]+fxg[sample_start+int((np.floor((len(fxg)-1)/(mid-g_start)))*(j-g_start))]
               
            for j in range(mid, ed_end-1):
               flare[j]=flare[j]+fxed[int(np.floor((len(fxed)-1)/(ed_end-mid)))*(j-mid)]

      ### Flare Type 1
      if flaretype[i]=='Impulse':

         if jp=='true':
            if 'FStart' in jf['FlareParameters'][i]:
               start=jf['FlareParameters'][i]['FStart']
            else:
               print('>Impulse Time not specified in flare_info.json\n\tRandomly generating')
               start=random.randint(0, len(time)-1)
         else:
            start=random.randint(0, len(time)-1) 
         if start>len(time)-2:
            print('\n\t>Fatal Error\n\t\tImpulse Flare Time outwith time axes')
            exit()
         #Amplitude
         if jp=='true':
            if 'Amp' in jf['FlareParameters'][i]:
               amplitude=jf['FlareParameters'][i]['Amp']
            else:
               print('>Amplitude not specified in flare_info.json\n\tandomly generating...')
               amplitude=random.randint(40,60)
         else:
                amplitude=random.randint(40,60)

                

         print('Flare', i+1, '\n\tType: Impulse\n\tAmplitude:', amplitude, '\n\tPeak:', (start+1)/2, 'hours')

         flare[start+1]=amplitude
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

   #Dropouts
   if "Dropouts" in jf:
      for i in range(0, len(jf["Dropouts"])):
         if "t0" not in jf["Dropouts"][i]:
            print("Dropout", i+1, "time not specified, randomly generating...")
            dropt0=(round(random.random()*max(time)*2))/2
         elif jf["Dropouts"][i]["t0"]>max(time):
            print("Dropout", i+1, "time outwith observation length, randomly generating one that is within...")
            dropt0=(round(random.random()*max(time)*2))/2
         else:
            dropt0=(round(jf["Dropouts"][i]["t0"]*2))/2

         if "Amp" not in jf["Dropouts"][i]:
            print("Dropout", i+1, "amplitude not specified, randomly generating...")
            dropamp=random.random()*4
         else:
            dropamp=jf["Dropouts"][i]["Amp"]
               
            
         for j in range(int(dropt0*2), len(time)):
            flare[j]=flare[j]+dropamp
            

         
   for i in range(0, len(time)):
      time[i]=time[i]/24
   if graph_true==1:
      plt.plot(time, flare)
      plt.plot(zerolinex, zeroliney, 'k:')
      plt.xlabel('Time (Days)')              #Plots curve(s) for human confirmation
      plt.ylabel('Intensity')
      if NumFlares>1:
         plt.title('Stellar Flares')
      else:
         plt.title('Stellar Flare')
      plt.show()

      
   goodinput='false'                                      
   while goodinput=='false': #Allows program to get the answer it requires to continue
      if graph_true==1:
         use=input('Do you wish to save this/these flare(s)? (y/n) ')
      else:
         use='y'
         
      if use=='n':
         goodinput='true'
         print('\nYou selected no, generating new curve(s)...\n')
      elif use=='exit':
         exit()
      elif use=='y':
         goodinput='true'
         goodcurve='true'
         file_name='flare-'+str(NumFlares)+'f-'+str(observation_length)+'h.txt'
         
         name_exists='true' #Avoiding overwriting previously saved same name files
         i=1
         while name_exists=='true':  
            if path.exists(file_name)==1:
               file_name='flare-'+str(NumFlares)+'f-'+str(observation_length)+'h('+str(i)+').txt'
               i=i+1
            else:
               name_exists='false'
            
         output=open(file_name, 'w') #Writing data to .txt file
         print(">> Writing data to file")
         output.write('#Time (Days)\tSignal\n')
         for i in range(0, len(time)):
            out=str(time[i])+'\t'+str(flare[i])+'\n'
            output.write(out)
         output.close()
         if path.exists(file_name)==1:
            print('\tSuccess!')
            fileoutname=open('filename.txt', 'w')
            fileoutname.write(file_name)
            output.close()
         else:
            print("Unexpected error, file somehow not present in current path.")
      else:
         print('Error: Please input y or n only. Or to exit without txt file, type "exit"')
