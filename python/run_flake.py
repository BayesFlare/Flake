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

from matplotlib import pyplot as plt

# get postprocess from DNest4/python/classic.py
def logsumexp(values):
  biggest = np.max(values)
  x = values - biggest
  result = np.log(np.sum(np.exp(x))) + biggest
  return result


def logdiffexp(x1, x2):
  biggest = x1
  xx1 = x1 - biggest
  xx2 = x2 - biggest
  result = np.log(np.exp(xx1) - np.exp(xx2)) + biggest
  return result


def loadtxt_rows(filename, rows, single_precision=False):
  """
  Load only certain rows
  """
  # Open the file
  f = open(filename, "r")

  # Storage
  results = {}

  # Row number
  i = 0

  # Number of columns
  ncol = None

  while(True):
    # Read the line and split by whitespace
    line = f.readline()
    cells = line.split()

    # Quit when you see a different number of columns
    if ncol is not None and len(cells) != ncol:
      break

    # Non-comment lines
    if cells[0] != "#":
      # If it's the first one, get the number of columns
      if ncol is None:
        ncol = len(cells)

      # Otherwise, include in results
      if i in rows:
        if single_precision:
          results[i] = np.array([float(cell) for cell in cells], dtype="float32")
        else:
          results[i] = np.array([float(cell) for cell in cells])
      i += 1

  results["ncol"] = ncol
  return results


# postprocess from DNest4/python/classic.py, with minor edits to data loading
def postprocess(temperature=1., numResampleLogX=1, plot=True, rundir=".", \
                cut=0., save=True, zoom_in=True, compression_bias_min=1., verbose=True,\
                compression_scatter=0., moreSamples=1., compression_assert=None, single_precision=False):
  levels_orig = np.atleast_2d(np.loadtxt(os.path.join(rundir, "levels.txt"), comments="#"))
  sample_info = np.atleast_2d(np.loadtxt(os.path.join(rundir, "sample_info.txt"), comments="#"))

  # Remove regularisation from levels_orig if we asked for it
  if compression_assert is not None:
    levels_orig[1:,0] = -np.cumsum(compression_assert*np.ones(levels_orig.shape[0] - 1))

  cut = int(cut*sample_info.shape[0])
  sample_info = sample_info[cut:, :]

  if plot:
    plt.figure(1)
    plt.plot(sample_info[:,0], "k")
    plt.xlabel("Iteration")
    plt.ylabel("Level")

    plt.figure(2)
    plt.subplot(2,1,1)
    plt.plot(np.diff(levels_orig[:,0]), "k")
    plt.ylabel("Compression")
    plt.xlabel("Level")
    xlim = plt.gca().get_xlim()
    plt.axhline(-1., color='g')
    plt.axhline(-np.log(10.), color='g', linestyle="--")
    plt.ylim(ymax=0.05)

    plt.subplot(2,1,2)
    good = np.nonzero(levels_orig[:,4] > 0)[0]
    plt.plot(levels_orig[good,3]/levels_orig[good,4], "ko-")
    plt.xlim(xlim)
    plt.ylim([0., 1.])
    plt.xlabel("Level")
    plt.ylabel("MH Acceptance")

  # Convert to lists of tuples
  logl_levels = [(levels_orig[i,1], levels_orig[i, 2]) for i in range(0, levels_orig.shape[0])] # logl, tiebreaker
  logl_samples = [(sample_info[i, 1], sample_info[i, 2], i) for i in range(0, sample_info.shape[0])] # logl, tiebreaker, id
  logx_samples = np.zeros((sample_info.shape[0], numResampleLogX))
  logp_samples = np.zeros((sample_info.shape[0], numResampleLogX))
  logP_samples = np.zeros((sample_info.shape[0], numResampleLogX))
  P_samples = np.zeros((sample_info.shape[0], numResampleLogX))
  logz_estimates = np.zeros((numResampleLogX, 1))
  H_estimates = np.zeros((numResampleLogX, 1))

  # Find sandwiching level for each sample
  sandwich = sample_info[:,0].copy().astype('int')
  for i in range(0, sample_info.shape[0]):
    while sandwich[i] < levels_orig.shape[0]-1 and logl_samples[i] > logl_levels[sandwich[i] + 1]:
      sandwich[i] += 1

  for z in range(0, numResampleLogX):
    # Make a monte carlo perturbation of the level compressions
    levels = levels_orig.copy()
    compressions = -np.diff(levels[:,0])
    compressions *= compression_bias_min + (1. - compression_bias_min)*np.random.rand()
    compressions *= np.exp(compression_scatter*np.random.randn(compressions.size))
    levels[1:, 0] = -compressions
    levels[:, 0] = np.cumsum(levels[:,0])

    # For each level
    for i in range(0, levels.shape[0]):
      # Find the samples sandwiched by this level
      which = np.nonzero(sandwich == i)[0]
      logl_samples_thisLevel = [] # (logl, tieBreaker, ID)
      for j in range(0, len(which)):
        logl_samples_thisLevel.append(copy.deepcopy(logl_samples[which[j]]))
      logl_samples_thisLevel = sorted(logl_samples_thisLevel)
      N = len(logl_samples_thisLevel)

      # Generate intermediate logx values
      logx_max = levels[i, 0]
      if i == levels.shape[0]-1:
        logx_min = -1E300
      else:
        logx_min = levels[i+1, 0]
      Umin = np.exp(logx_min - logx_max)

      if N == 0 or numResampleLogX > 1:
        U = Umin + (1. - Umin)*np.random.rand(len(which))
      else:
        U = Umin + (1. - Umin)*np.linspace(1./(N+1), 1. - 1./(N+1), N)
      logx_samples_thisLevel = np.sort(logx_max + np.log(U))[::-1]

      for j in range(0, which.size):
        logx_samples[logl_samples_thisLevel[j][2]][z] = logx_samples_thisLevel[j]

        if j != which.size - 1:
          left = logx_samples_thisLevel[j+1]
        elif i == levels.shape[0]-1:
          left = -1E300
        else:
          left = levels[i+1][0]

        if j != 0:
          right = logx_samples_thisLevel[j-1]
        else:
          right = levels[i][0]

        logp_samples[logl_samples_thisLevel[j][2]][z] = np.log(0.5) + logdiffexp(right, left)

    logl = sample_info[:,1]/temperature

    logp_samples[:,z] = logp_samples[:,z] - logsumexp(logp_samples[:,z])
    logP_samples[:,z] = logp_samples[:,z] + logl
    logz_estimates[z] = logsumexp(logP_samples[:,z])
    logP_samples[:,z] -= logz_estimates[z]
    P_samples[:,z] = np.exp(logP_samples[:,z])
    H_estimates[z] = -logz_estimates[z] + np.sum(P_samples[:,z]*logl)

    if plot:
      plt.figure(3)

      plt.subplot(2,1,1)
      plt.plot(logx_samples[:,z], sample_info[:,1], 'k.', label='Samples')
      plt.plot(levels[1:,0], levels[1:,1], 'g.', label='Levels')
      plt.legend(numpoints=1, loc='lower left')
      plt.ylabel('log(L)')
      plt.title(str(z+1) + "/" + str(numResampleLogX) + ", log(Z) = " + str(logz_estimates[z][0]))
      # Use all plotted logl values to set ylim
      combined_logl = np.hstack([sample_info[:,1], levels[1:, 1]])
      combined_logl = np.sort(combined_logl)
      lower = combined_logl[int(0.1*combined_logl.size)]
      upper = combined_logl[-1]
      diff = upper - lower
      lower -= 0.05*diff
      upper += 0.05*diff
      if zoom_in:
        plt.ylim([lower, upper])
      xlim = plt.gca().get_xlim()

    if plot:
      plt.subplot(2,1,2)
      plt.plot(logx_samples[:,z], P_samples[:,z], 'k.')
      plt.ylabel('Posterior Weights')
      plt.xlabel('log(X)')
      plt.xlim(xlim)

  P_samples = np.mean(P_samples, 1)
  P_samples = P_samples/np.sum(P_samples)
  logz_estimate = np.mean(logz_estimates)
  logz_error = np.std(logz_estimates)
  H_estimate = np.mean(H_estimates)
  H_error = np.std(H_estimates)
  ESS = np.exp(-np.sum(P_samples*np.log(P_samples+1E-300)))

  errorbar1 = ""
  errorbar2 = ""
  if numResampleLogX > 1:
    errorbar1 += " +- " + str(logz_error)
    errorbar2 += " +- " + str(H_error)

  if verbose:
    print("log(Z) = " + str(logz_estimate) + errorbar1)
    print("Information = " + str(H_estimate) + errorbar2 + " nats.")
    print("Effective sample size = " + str(ESS))

  # Resample to uniform weight
  N = int(moreSamples*ESS)
  w = P_samples
  w = w/np.max(w)
  rows = np.empty(N, dtype="int64")
  for i in range(0, N):
    while True:
      which = np.random.randint(sample_info.shape[0])
      if np.random.rand() <= w[which]:
        break
    rows[i] = which + cut

  # Get header row
  f = open(os.path.join(rundir, "sample.txt"), "r")
  line = f.readline()
  if line[0] == "#":
    header = line[1:]
  else:
    header = ""
  f.close()

  sample = loadtxt_rows(os.path.join(rundir, "sample.txt"), set(rows), single_precision)
  posterior_sample = None
  if single_precision:
    posterior_sample = np.empty((N, sample["ncol"]), dtype="float32")
  else:
    posterior_sample = np.empty((N, sample["ncol"]))

  for i in range(0, N):
    posterior_sample[i, :] = sample[rows[i]]

  if save:
    np.savetxt(os.path.join(rundir, 'weights.txt'), w)
    if single_precision:
      np.savetxt(os.path.join(rundir, "posterior_sample.txt"), posterior_sample, fmt="%.7e", header=header)
    else:
      np.savetxt(os.path.join(rundir, "posterior_sample.txt"), posterior_sample, header=header)

  if plot:
    plt.show()

  return [logz_estimate, H_estimate, logx_samples, posterior_sample]


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
      time.sleep(30) # sleep for a minute

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
    
  plt.hist(sinnum)
  plt.show()