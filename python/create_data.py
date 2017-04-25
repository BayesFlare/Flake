#!/usr/bin/env python

import os
import sys
import numpy as np
import json
import argparse

import bayesflare as bf

"""
Create a data file potentially with flare, sinusoid, change point and impulse signals
added. The data is generated based on information provided by a JSON configuration
file. Such a file should look like:

{
  "datafile": "data.txt", // the file to output the data to
  "starttime": 0,         // the data start time
  "timestep": 60,         // the time step in the data file
  "duration": 100,        // the number of time steps
  "backgroundoffset": 0., // the background offset value
  "noise": 1.,            // the "Gaussian" noise standard deviation
  "flares": [{"amplitude": 1.,     // flare amplitude
              "time": 212.23,      // flare peak time (in time units from start time)
              "risetime": 63.4,    // flare rise time (in time units)
              "decaytime": 102.3}, // flare decay time (in time units)
             {"amplitude": 1.5,
              "time": 315.53,
              "risetime": 40.4,
              "decaytime": 122.3}],
  "sinusoids": [{"amplitude": 3.4,  // sinusoid amplitude
                 "phase": 0.2,      // sinusoid phase (in rads)
                 "period": 0.04}]   // sinusoid period (in time units)
}

"""

parser = argparse.ArgumentParser()
parser.add_argument("injectfile", help="The injection data JSON file")

# parse input options
opts = parser.parse_args()

# open JSON file
try:
  fp = open(opts.injectfile, "r")
  inj = json.load(fp)
  fp.close()
except:
  print("Error... could not load JSON file.")
  sys.exit(1)

if "datafile" not in inj:
  print("Error... no 'datafile' specified in JSON file")
  sys.exit(1)
else:
  datafile = inj['datafile']
  
# get times for data file
if 'starttime' not in inj or 'timestep' not in inj or 'duration' not in inj:
  print("Error... some time values not specified in JSON file")
  sys.exit(1)
else:
  starttime = inj['starttime']
  timestep = inj['timestep']
  duration = int(inj['duration'])
  
datatimes = np.arange(starttime, starttime+duration*timestep, timestep)

if 'backgroundoffset' not in inj:
  backgroundoffset = 0.
else:
  backgroundoffset = inj['backgroundoffset']
data = np.ones(duration)*backgroundoffset

if 'noise' not in inj:
  print("Error... no noise level set in JSON file")
  sys.exit(1)
else:
  noise = inj['noise']

data += noise*np.random.randn(duration)

# set sinusoids
if 'sinusoids' in inj:
  sinusoids = inj['sinusoids']

  if not isinstance(sinusoids, list):
    print("Error... 'sinusoids' must be a list")
    sys.exit(1)

  nsins = len(sinusoids)
  for i in range(nsins):
    thissin = sinusoids[i]['amplitude']*np.sin((2.*np.pi*datatimes/sinusoids[i]['period']) + sinusoids[i]['phase'])
    data += thissin

# set flares
if 'flares' in inj:
  flares = inj['flares']

  if not isinstance(flares, list):
    print("Error... 'flares' must be a list")
    sys.exit(1)

  nflares = len(flares)
  for i in range(nflares):
    Mf = bf.Flare(datatimes, amp=1.)

    # create flare
    pdict = {'t0': flares[i]['time'], 'amp': flares[i]['amplitude'], 'taugauss': flares[i]['risetime'], 'tauexp': flares[i]['decaytime']}
    data += Mf.model(pdict)
    
# set change points and impulses later

# output data
try:
  np.savetxt(datafile, np.vstack((datatimes.T, data.T)).T)
except:
  print("Error... could not output data file")
  sys.exit(1)