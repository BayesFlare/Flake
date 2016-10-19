Variables in the .JSON files are as follows

******************
*FlareParameters*
******************
Parameters Unique to each Flare

GRT- Gaussian Rise Time
     Gaussian Rise and Exponential Decay flares' (GREDs) Gaussian
     Rise section duration.
     Measured in lots of 30m. 
     
EDT- Exponential Decay Time
     GREDs Exponential Decays duration.
     Measured in lots of 30m

Amp- Amplitude
     Amplitude of the flare

FStart- Flare Start
	The time at which the flare begins
	Measured in lots of 30m

FlareType- Flare Type
	   >(Temporary until a better variable is implemented)
	   Type of flare these parameters describe
	   Type 1 are Impulse flares
	   Type 2 are GREDs
	   
*******************
*GlobalParameters*
*******************
General Paramaters used Globally by all Flares

Noise- Noise
       Shall random noise be injected?
       1 - Yes
       Any other number - no

ObsLen- Observation Length
	The time over which the flare(s) are observed
	Measured in lots of 1 hour.
