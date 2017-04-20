#!/usr/bin/env python

"""
Python script to run flake
"""

from __future__ import division, print_function

import os
import sys
import argparse
import json
import time
import copy
import subprocess as sp
import numpy as np

from matplotlib import pyplot as pl

from postprocess import *

if __name__=='__main__':
  parser = argparse.ArgumentParser()
  parser.add_argument("datafile", help="The data file")
  parser.add_argument("-e", "--executable", dest="execu", default="flake", help="Set the \"flake\" executable to run")
  parser.add_argument("-r", "--run-dir", dest="rundir", default=".", help="Set the run directory (default: %(default)s)")
  parser.add_argument("-c", "--config-file", dest="configfile", help="Set the JSON flake configuration file")
  parser.add_argument("-i", "--inject-file", dest="injfile", help="Set a JSON file containing any signal parameters injected into the data file")
  parser.add_argument("-t", "--num-threads", dest="numthreads", default=1, type=int, help="Set the number of CPU cores to use (default: %(default)s)")
  parser.add_argument("-p", "--min-psamples", dest="minpost", default=1000, type=int, help="Set the minimum number of posterior samples at which to terminate flake (default: %(default)s)")

  # parse input options
  opts = parser.parse_args()

  # check executable exists and is executable
  flake = opts.execu
  if not os.path.isfile(flake) or not os.access(flake, os.X_OK):
    print("Error... executable \"%s\" does not exist" % flake)
    sys.exit(1)

  # create run directory if it does not exist
  rundir = opts.rundir
  if not os.path.isdir(rundir):
    try:
      os.makedirs(rundir)
    except:
      print("Error... could not create run directory \"%s\"" % rundir)
      sys.exit(1)

  # set options file (if it does not exist in the run directory)
  options = os.path.join(rundir, "OPTIONS")
  if not os.path.isfile(options):
    fp = open(options, "w")
    flakeoptions = """# File containing parameters for flake
# Put comments at the top, or at the end of the line.
5       # Number of particles
10000   # new level interval
10000   # save interval
100     # threadSteps - how many steps each thread should do independently before communication
100     # maximum number of levels
10      # Backtracking scale length (lambda in the paper)
100     # Strength of effect to force histogram to equal push (beta in the paper)
10000   # Maximum number of saves (0 = infinite)
{rundir}/sample.txt
{rundir}/sample_info.txt
{rundir}/levels.txt
"""
    fp.write(flakeoptions.format(**{"rundir": rundir}))
    fp.close()

  minpost = opts.minpost
  if minpost < 1:
    print("Error... minimum required number of posterior samples must be greater than 1")
    sys.exit(1)

  # check data file exists
  datafile = opts.datafile
  if not os.path.isfile(datafile):
    print("Error... data file \"%s\" does not exist" % datafile)
    sys.exit(1)

  # check the configuration file exists
  configfile = ""
  if opts.configfile is not None:
    configfile = opts.configfile
    if not os.path.isfile(configfile):
      configfile = ""

  # run code and end when at least minpost posterior samples have been generated
  npsamps = 0
  flakerun = [flake, "-t", "%i" % opts.numthreads, "-d", datafile, "-o", options, "-f", configfile]
  print("Running flake:\n$ %s\n" % " ".join(flakerun))
  flake_process = sp.Popen(flakerun)
  print("process ID: %s\n" % flake_process.pid)
  while npsamps < minpost:
    try:
      time.sleep(10) # sleep for a minute

      logz, Hs, lxs, post_samples = postprocess(temperature=1., numResampleLogX=1, plot=False, rundir=rundir,
                                                cut=0., save=True, zoom_in=True, compression_bias_min=1., verbose=False,
                                                compression_scatter=0., moreSamples=1., compression_assert=None, single_precision=False)
      npsamps = len(post_samples)
      print("Current number of posterior samples: %d\n" % npsamps)
    except KeyboardInterrupt:
      break

  flake_process.kill()

  # parse the posterior samples into model components
  noisestddev = post_samples[:,0]      # data noise standard deviation
  backgroundoffset = post_samples[:,1] # data background offset value
  
  # change point model
  cpparams = post_samples[0,2]         # number of parameters in change point model (just need first value)
  cpnmax = int(post_samples[0,3])      # maximum number of change point components
  cpamploc = post_samples[:,4]         # change point amplitude hyperparameter
  idj = 5
  if cpnmax > 0:
    cpnum = post_samples[:,idj]               # number of change point models
    idj += 1
    cptimes = post_samples[:,idj:idj+cpnmax]  # change point times
    idj += cpnmax
    cpoffsets = post_samples[:,idj:idj+cpmax] # change point offsets
    idj += cpnmax
  else:
    cpnum = None
    idj += 1

  # sinusoid model
  sinparams = post_samples[0,idj]        # number of parameters in sinusoid model
  sinnmax = int(post_samples[0,idj+1])   # maximum number of sinusoid components
  sinperiodloc = post_samples[:,idj+2]   # sinusoid period location hyperparameter
  sinperiodscale = post_samples[:,idj+3] # sinusoid period scale hyperparameter
  sinamploc = post_samples[:,idj+4]      # sinusoid amplitude location hyperparameter
  sinampscale = post_samples[:,idj+5]    # sinusoid amplitude scale hyperparameter
  idj += 6
  if sinnmax > 0:
    sinnum = post_samples[:,idj]                   # number of sinusoid components
    idj += 1
    sinlogperiod = post_samples[:,idj:idj+sinnmax] # sinusoid log periods
    idj += sinnmax
    sinlogamp = post_samples[:,idj:idj+sinnmax]    # sinusoid log amplitudes
    idj += sinnmax
    sinphase = post_samples[:,idj:idj+sinnmax]     # sinusoid phases
    idj += sinnmax
  else:
    sinnum = None
    idj += 1
    
  # flare model
  flareparams = post_samples[0,idj]      # number of parameters in flare model
  flarenmax = int(post_samples[0,idj+1]) # maximum number of flare components
  flareamploc = post_samples[:,idj+2]    # flare amplitude location hyperparameter
  flareampscale = post_samples[:,idj+3]  # flare amplitude scale parameter
  flareriseloc = post_samples[:,idj+4]   # flare rise time hyperparameter
  flaredecayloc = post_samples[:,idj+5]  # flare decay time hyperparameter
  idj += 6
  if flarenmax > 0:
    flarenum = post_samples[:,idj]                      # number of flare components
    idj += 1
    flaretime = post_samples[:,idj:idj+flarenmax]       # flare peak times
    idj += flarenmax
    flarelogamp = post_samples[:,idj:idj+flarenmax]     # flare log amplitudes
    idj += flarenmax
    flarerisetimes = post_samples[:,idj:idj+flarenmax]  # flare rise times
    idj += flarenmax
    flaredecaytimes = post_samples[:,idj:idj+flarenmax] # flare decay times
    idj += flarenmax
  else:
    flarenum = None
    idj += 1
    
  # impulse model
  impparams = post_samples[0,idj]      # number of parameters in impulse model
  impnmax = int(post_samples[0,idj+1]) # maximum number of impulse components
  impamploc = post_samples[:,idj+2]    # impulse amplitude location hyperparameter
  impampscale = post_samples[:,idj+3]  # impulse amplitude scale hyperparameter
  idj += 4
  if impnmax > 0:
    impnum = post_samples[:,idj]                # number of impulse components
    idj += 1
    imptime = post_samples[:,idj:idj+impnmax]   # impulse times
    idj += impnmax
    implogamp = post_samples[:,idj:idj+impnmax] # impulse log amplitudes
  else:
    impnum = None
    
  pl.hist(sinnum)
  pl.show()