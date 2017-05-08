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

import matplotlib as mpl
from matplotlib import pyplot as pl
import matplotlib.gridspec as gridspec
from matplotlib.ticker import ScalarFormatter

mplparams = {
  'backend': 'Agg',
  'text.usetex': True, # use LaTeX for all text
  'axes.linewidth': 0.5, # set axes linewidths to 0.5
  'axes.grid': True, # add a grid
  'grid.linewidth': 0.5,
  'font.family': 'sans-serif',
  'font.sans-serif': 'Avant Garde, Helvetica, Computer Modern Sans serif',
  'font.size': 11 }

mpl.rcParams.update(mplparams)

# Bayesflare for models
import bayesflare as bf

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
  parser.add_argument("-n", "--plot-nsamples", dest="plotsamples", default=20, type=int, help="Set the number of sample light curves to plot (default: %(default)s)")
  parser.add_argument("-x", "--time-it", dest="timeit", action="store_true", default=False, help="Output the run time of 'flake' and time per posterior sample")
  parser.add_argument("-m", "--output-plot", dest="outname", help="The output file name for the plot")

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

  if opts.timeit:
    t0 = time.time()

  # run code and end when at least minpost posterior samples have been generated
  npsamps = 0
  flakerun = [flake, "-t", "%i" % opts.numthreads, "-d", datafile, "-o", options, "-f", configfile]
  print("Running flake:\n$ %s\n" % " ".join(flakerun))
  flake_process = sp.Popen(flakerun)
  print("process ID: %s\n" % flake_process.pid)
  while npsamps < minpost:
    try:
      time.sleep(10) # sleep for a minute

      output = postprocess(temperature=1., numResampleLogX=1, plot=False, rundir=rundir,
                                                cut=0., save=True, zoom_in=True, compression_bias_min=1., verbose=False,
                                                compression_scatter=0., moreSamples=1., compression_assert=None, single_precision=False)
      if output is not None:
        logz, Hs, lxs, post_samples = output
        npsamps = len(post_samples)
        print("Current number of posterior samples: %d\n" % npsamps)
    except KeyboardInterrupt:
      break

  flake_process.kill()

  if opts.timeit:
    t1 = time.time()
    dt = t1-t0
    print("Total runtime for 'flake': %.2f\n" % dt, file=sys.stdout)
    print("Run time per posterior sample: %.5f" % (dt/npsamps), file=sys.stdout)

  # parse the posterior samples into model components
  noisestddev = post_samples[:,0]      # data noise standard deviation
  backgroundoffset = post_samples[:,1] # data background offset value
  nmodels = 0 # number of models used
  
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
    nmodels += 1
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
    nmodels += 1
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
    nmodels += 1
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
    nmodels += 1
  else:
    impnum = None

  # read in the data file
  ext = os.path.splitext(datafile)[-1].lower()
  if ext == '.fits':
    flarelc = bf.Lightcurve(curve=kfile)
    times = flarelc.cts-flarelc.cts[0]
    lc = flarelc.clc - np.median(flare.clc)
  elif ext == '.txt':
    data = np.loadtxt(datafile)
    times = data[:,0]-data[0,0] # set times to have epoch at first time stamp
    lc = data[:,1] - np.median(data[:,1]) # remove median from "light curve"
  else:
    print("Data file extension not recognised")
    sys.exit(1)

  # check number of samples to plot
  plotsamples = min([opts.plotsamples, npsamps])

  # randomly pick several samples and plot over the data 
  sampidx = np.random.choice(npsamps, plotsamples, replace=False)
  modelcurves = []
  
  for i in range(plotsamples):
    thisidx = sampidx[i]
    thiscurve = np.zeros(len(times)) + backgroundoffset[thisidx]
    
    # create flare model
    if flarenum is not None:
      for j in range(int(flarenum[thisidx])):
        Mf = bf.Flare(times, amp=1.)

        # create flare
        pdict = {'t0': flaretime[thisidx, j], 'amp': np.exp(flarelogamp[thisidx,j]), 'taugauss': flarerisetimes[thisidx,j], 'tauexp': flaredecaytimes[thisidx,j]}
        thiscurve += Mf.model(pdict)

    # create sinusoid model
    if sinnum is not None:
      for j in range(int(sinnum[thisidx])):
        thiscurve += np.exp(sinlogamp[thisidx,j])*np.sin((2.*np.pi*times/np.exp(sinlogperiod[thisidx,j])) + sinphase[thisidx,j])

    # create impulse model
    if impnum is not None:
      for j in range(int(impnum[thisidx])):
        Mi = bf.Impulse(times, amp=1.)

        # create impulse
        pdict = {'t0': imptime[thisidx,j], 'amp': np.exp(implogamp[thisidx,j])}
        thiscurve += Mi.model(pdict)
        
    # create change point (step) model
    if cpnum is not None:
      for j in range(int(cpnum[thisidx])):
        Ms = bf.Step(times, amp=1.)
        
        # create step
        pdict = {'t0': cptimes[thisidx,j], 'amp': cpoffsets[thisidx,j]}
        thiscurve += Ms.model(pdict)
        
    modelcurves.append(thiscurve)

  fig = pl.figure(figsize=(16,6+nmodels*3))
  gs = gridspec.GridSpec(4 + nmodels*3, 5 + 12)

  # set the plot for the data
  ax = pl.subplot(gs[0:4,:])

  # plot data and overlay posterior model curve samples
  ax.plot(times, lc, 'b', lw=2)
  ax.set_xlabel('Time')
  ax.set_ylabel('Amplitude')

  for mc in modelcurves:
    ax.plot(times, mc, 'r', alpha=0.2, lw=1.5)

  # plot and overlay injected signal if given
  inj = None
  if opts.injfile is not None:
    if os.path.isfile(opts.injfile):
      try:
        fp = open(opts.injfile, "r")
        inj = json.load(fp)
        fp.close()
      except:
        print("Could not read in injection file")
        sys.exit(1)
    
      # create injected signal
      dm = np.median(data[:,1])
      if 'backgroundoffset' not in inj:
        backgroundoffset = -dm
      else:
        backgroundoffset = inj['backgroundoffset'] - dm
      injsig = np.ones(len(data))*backgroundoffset
      
      # set sinusoids
      if 'sinusoids' in inj:
        sinusoidsinj = inj['sinusoids']

        if not isinstance(sinusoidsinj, list):
          print("Error... 'sinusoids' must be a list")
          sys.exit(1)

        nsinsinj = len(sinusoidsinj)
        for i in range(nsinsinj):
          thissin = sinusoidsinj[i]['amplitude']*np.sin((2.*np.pi*times/sinusoidsinj[i]['period']) + sinusoidsinj[i]['phase'])
          injsig += thissin

      # set flares
      if 'flares' in inj:
        flaresinj = inj['flares']

        if not isinstance(flaresinj, list):
          print("Error... 'flares' must be a list")
          sys.exit(1)

        nflaresinj = len(flaresinj)
        for i in range(nflaresinj):
          Mf = bf.Flare(times, amp=1.)

          # create flare
          pdict = {'t0': flaresinj[i]['time'], 'amp': flaresinj[i]['amplitude'], 'taugauss': flaresinj[i]['risetime'], 'tauexp': flaresinj[i]['decaytime']}
          injsig += Mf.model(pdict)

      ax.plot(times, injsig, 'k--', lw=0.5)

  # plot the sinusoid model parameters
  if sinnum is not None:
    axsinnum = pl.subplot(gs[4:7, 0:5])
    nbins = np.arange(0., np.max(sinnum)+1)
    axsinnum.hist(sinnum, bins=nbins, histtype='stepfilled', normed=True)
    if inj is not None:
      if 'sinusoids' in inj:
        axsinnum.axvline(nsinsinj+0.5, color='k', ls='--', lw=2) # 0.5 addition (here and in xticks) is to set the histogram bins labels correctly
    axsinnum.set_xlabel('No. sinusoids')
    axsinnum.set_xticks(nbins[:-1]+0.5)
    axsinnum.set_xticklabels(nbins[:-1])

    # get median number of sinusoids
    #medsins = int(np.median(sinnum))
    # get the mode number of sinusoids
    slist = sinnum.tolist()
    modesins = int(max(set(slist), key=slist.count))

    axsinamp = pl.subplot(gs[4:7, 5:9])
    axsinperiod = pl.subplot(gs[4:7, 9:13])
    axsinphase = pl.subplot(gs[4:7, 13:])
    #for j in range(medsins):
    for j in range(modesins):
      axsinamp.hist(np.exp(sinlogamp[:,j]), histtype='stepfilled', normed=True, alpha=0.3)
      axsinperiod.hist(np.exp(sinlogperiod[:,j]), histtype='stepfilled', normed=True, alpha=0.3)
      axsinphase.hist(sinphase[:,j], histtype='stepfilled', normed=True, alpha=0.3)

    # plot true values of parameters in injection is provided
    if inj is not None:
      if 'sinusoids' in inj:
        for j in range(nsinsinj):
          axsinamp.axvline(sinusoidsinj[j]['amplitude'], ls='--', lw=2)
          axsinperiod.axvline(sinusoidsinj[j]['period'], ls='--', lw=2)
          axsinphase.axvline(sinusoidsinj[j]['phase'], ls='--', lw=2)

    axsinamp.set_xlabel('$A$')
    axsinperiod.set_xlabel('$P$')
    axsinphase.set_xlabel('$\phi$ (rads)')
    axsinphase.set_xlim([0., 2.*np.pi])
    
  # plot the flare model parameters
  if flarenum is not None:
    axflarenum = pl.subplot(gs[7:10, 0:5])
    nbins = np.arange(0., np.max(flarenum)+1)
    axflarenum.hist(flarenum, bins=nbins, histtype='stepfilled', normed=True)
    if inj is not None:
      if 'flares' in inj:
        axflarenum.axvline(nflaresinj+0.5, color='k', ls='--', lw=2) # 0.5 addition (here and in xticks) is to set the histogram bins labels correctly
    axflarenum.set_xlabel('No. flares')
    axflarenum.set_xticks(nbins[:-1]+0.5)
    axflarenum.set_xticklabels(nbins[:-1])
    
    # get median number of flares
    #medflares = int(np.median(flarenum))
    # get the mode number of flares
    flist = flarenum.tolist()
    modeflares = int(max(set(flist), key=flist.count))
    
    axflareamp = pl.subplot(gs[7:10, 5:8])
    axflaret0 = pl.subplot(gs[7:10, 8:11])
    axflarerise = pl.subplot(gs[7:10, 11:14])
    axflaredec = pl.subplot(gs[7:10, 14:])

    xfmt = ScalarFormatter()
    xfmt.set_powerlimits((-2,2))

    #for j in range(medflares):
    for j in range(modeflares):
      axflareamp.hist(np.exp(flarelogamp[:,j]), histtype='stepfilled', normed=True, alpha=0.3)
      axflaret0.hist(flaretime[:,j], histtype='stepfilled', normed=True, alpha=0.3)
      axflarerise.hist(flarerisetimes[:,j], histtype='stepfilled', normed=True, alpha=0.3)
      axflarerise.xaxis.set_major_formatter(xfmt)
      axflaredec.hist(flaredecaytimes[:,j], histtype='stepfilled', normed=True, alpha=0.3)
      axflaredec.xaxis.set_major_formatter(xfmt)

      # on the time series plot add vertical lines marking each found flare (median peak time)
      if j == 0:
        prange = ax.get_ylim()
        ax.axvline(np.median(flaretime[:,j]), prange[0], prange[1], color='m', lw=3, alpha=0.3)

    # plot true values of parameters in injection is provided
    if inj is not None:
      if 'flares' in inj:
        for j in range(nflaresinj):
          axflareamp.axvline(flaresinj[j]['amplitude'], ls='--', lw=2)
          axflaret0.axvline(flaresinj[j]['time'], ls='--', lw=2)
          axflarerise.axvline(flaresinj[j]['risetime'], ls='--', lw=2)
          axflaredec.axvline(flaresinj[j]['decaytime'], ls='--', lw=2)
    
    axflareamp.set_xlabel('$A$')
    axflaret0.set_xlabel('$t_0$')
    axflarerise.set_xlabel(r'$\tau_g$')
    axflaredec.set_xlabel(r'$\tau_e$')

  fig.tight_layout()
  if opts.outname is None:
    pl.show()
  else:
    outname = opts.outname
    try:
      if 'png' in outname:
        fig.savefig(outname, dpi=300)
      else:
        fig.savefig(outname)
    except:
      print("Error... problem outputing plot to '%s'" % outname, file=sys.stderr)
      sys.exit(1)
