Variables in the .JSON files are as follows

*****************
*FlareParameters*
*****************
Parameters Unique to each Flare

GRT- Gaussian Rise Time
     Gaussian Rise and Exponential Decay flares' (GREDs) Gaussian
     Rise section duration.
     Measured in lots of 30m.
     *If field not in json file, variable will be randomly generated
     
EDT- Exponential Decay Time
     GREDs Exponential Decays duration.
     Measured in lots of 30m
     *If field not in json file, variable will be randomly generated

Amp- Amplitude
     Amplitude of the flare
     *If field not in json file, variable will be randomly generated

FStart- Flare Start
	The time at which the flare begins
	Measured in lots of 30m
	*If field not in json file, variable will be randomly generated

FlareType- Flare Type
	   "Impulse" are Impulse flares
	   "GRED" are GREDs
	   *If field not in json file, GRED will be run by default
	   
******************
*GlobalParameters*
******************
General Paramaters used Globally by all Flares

Noise- Noise
       Shall random noise be injected?
       1 - Yes
       Any other number - no
       *If field not in json file, yes will be run by default

ObsLen- Observation Length
	The time over which the flare(s) are observed
	Measured in lots of 1 hour.
	*If field not in json file, variable will be randomly generated

Graph- Output graph to check that correct plot is created?
       1 - Yes
       Any other number - no
       If you plan to run flareflake, set this as not 1

***********
*Sinusoids*
***********
Sinusoidal variations in the signal

Period- The period of the sinusoid in hours

Phase- The phase offset of the sinusoid in hours

Amp- The amplitude of the sinusoid

**********
*Dropouts*
**********
Drop outs in the signal.
For a drop out specify a negative amplitude
For a drop on  specify a positive amplitude

t0- When the drop out occurs

Amp- The amplitude of the drop



