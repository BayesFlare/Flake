import matplotlib.pyplot as plt
import numpy as np
import random
import os.path as path

observation_length=int(input('Over how many hours would you like the "observation" to be? ')) #in hours
NumFlares=int(input('How many flares would you like to inject? '))

#Initial variable initialisation and assignment

#Assumes half gaussian half exp decay

time=np.arange(0.0, observation_length+0.5, 0.5) #Time axis for flare
flare=[0.0]*len(time)                            #Light curve
zeroliney=[0,0]                                  #Used to generate 0 line
zerolinex=[0, observation_length]                #As above
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

goodcurve='false' #Allow new curves to be generated if one is rejected

while goodcurve=='false': 

   for i in range(0, len(flare)): #Generate Gaussian noise
      flare[i]=(2*random.random())-1

   amplitude=random.randint(40,60) #Generate flare amplitude

   #Generate half Gauss  
   gauss_sig=300
   for i in range(0, len(fxg)):
       fxg[i]=amplitude*(np.exp(((x[i]-999)**2)/(-2*((gauss_sig)**2))))
       
   #Generate half decay
   a=(np.log(amplitude)-np.log(1))/(len(x))
                  
   for i in x:
       fxed[i]=amplitude*np.exp(-a*x[i])

      #Flare injector v3: Imperfect (realistic), half gaussian, half exponential decay, injector
       
   for i in range(0, NumFlares):

      identical='true'
      while identical=='true':     #Avoids creation of identical flares
 
         a=random.randint(0, len(time)-1) #Generates flare duration
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




         if g_start not in starts or mid not in mids or ed_end not in ends:
               identical='false'
               starts[i]=g_start             #Avoids creation of identical flares 
               mids[i]=mid
               ends[i]=ed_end

      sample_start=random.randint(0, int(np.floor(len(fxg)/(mid-g_start)))) #Generates gaussian sample start  


      if sample_start==0:    #Samples smooth dists and injects them as flare(s)
         for i in range(g_start, mid+1):
            flare[i]=flare[i]+fxg[int((np.floor(len(fxg)/(mid-g_start)))*(i-g_start))]
         for i in range(mid+1, ed_end):
            flare[i]=flare[i]+fxed[int(np.floor(len(fxed)/(ed_end-mid-1)))*(i-mid-1)]
      else:
         for i in range(g_start, mid):
            flare[i]=flare[i]+fxg[sample_start+int((np.floor(len(fxg)/(mid-g_start)))*(i-g_start))]
         for i in range(mid, ed_end-1):
            flare[i]=flare[i]+fxed[sample_start+int(np.floor(len(fxed)/(ed_end-mid)))*(i-mid)]

            
      
       
   plt.plot(time, flare)
   plt.plot(zerolinex, zeroliney, 'k:')
   plt.xlabel('Time (hours)')              #Plots curve(s) for human confirmation
   plt.ylabel('Intensity')
   plt.title('Stellar Flare(s)')
   plt.show()

   goodinput='false'                                      
   while goodinput=='false': #Allows program to get the answer it requires to continue
      use=input('Do you wish to use this flare? (y/n) ')

      if use=='n':
         goodinput='true'
         print('You selected no, generating new curve...')
      elif use=='exit':
         goodinput='true'
         goodcurve='true'
         #Used to exit without .txt file
      elif use=='y':
         goodinput='true'
         goodcurve='true'
         file_name='mb-flare-'+str(NumFlares)+'f-'+str(observation_length)+'h.txt'
         
         name_exists='true' #Avoiding overwriting previously saved same name files
         i=1
         while name_exists=='true':  
            if path.exists(file_name)==1:
               file_name='mb-flare-'+str(NumFlares)+'f-'+str(observation_length)+'h('+str(i)+').txt'
               i=i+1
            else:
               name_exists='false'
            
         output=open(file_name, 'w') #Writing data to .txt file
         output.write('#Time (hours)    Signal\n')
         for i in range(0, len(time)):
            if time[i]<10:
               out=' '+str(time[i])+'            '+str(flare[i])+'\n'
            else:
               out=str(time[i])+'            '+str(flare[i])+'\n'
            output.write(out)
         output.close()
      else:
         print('Error: Please input y or n only, or to exit without txt file, type "exit"')
