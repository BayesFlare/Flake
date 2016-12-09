Variables in the .JSON files are as follows

*****************
*FlareParameters*
*****************
Parameters Unique to each Flare

GSTD- Gaussian Standard Deviation (GRED Type Only, see FlareType)
      The standard deviation of the Gaussian Rise.
      Measured in lots of 0.5 hours
      *If field not in json file, variable will be randomly generated
     
EDTC- Exponential Decay Time Constant (GRED Type Only, see FlareType)
      The constant characterising the decay of the flare.
      Measured in lots of 0.5 hours
      *If field not in json file, variable will be randomly generated

Amp- Amplitude
     Amplitude of the flare
     *If field not in json file, variable will be randomly generated

t0- Flare peak time
    The time at which the flare is at it's largest
    Measured in lots of 0.5 hours
    *If field not in json file, variable will be randomly generated

FlareType- Flare Type
	   "Impulse" are Impulse flares
	   "GRED" are Gaussian Rises and Exponential Decays
	   "N/A" No flare will be produced. Use this if you wish to produce only noise and other signal artefacts.
	   *If field not in json file, GRED will be run by default
	   
******************
*GlobalParameters*
******************
General Paramaters used Globally by all Flares

Noise- Noise
       Standard devatiation of noise in data. Std=0 will return no noise.

ObsLen- Observation Length
	The time over which the flare(s) are observed
	Measured in lots of 1 hour.
	*If field not in json file, variable will be randomly generated

Graph- Output graph to check that correct plot is created?
       1 - Yes
       Any other number - No

***********
*Sinusoids*
***********
Sinusoidal variations in the signal

Period- The period of the sinusoid in hours

Phase- The phase offset of the sinusoid in hours

Amp- The amplitude of the sinusoid

***************
*Change Points*
***************
Change Points in the signal.
Setting t0=0 for a Change Point will apply an offset of that Change Point's amplitude from 0 over the whole flare signal

t0- When the Change Point occurs in hours

Amp- The amplitude of the drop

**********
*Transits*
**********
Simulates a planet transit.
If Transit entry is in json file, with no contents, the star will consist of default Solar values and the planet will be a Jupiter in a 1AU orbit, and t0 will be randomly generated.

Rs- Stellar radius

Rp- Planetary radius

Ms- Mass of the star

Mp- Mass of the planet

Ro- Radius of planetary orbit

t0- Time which transit begins
