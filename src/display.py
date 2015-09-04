from pylab import *
import numpy as np
from astropy.io import fits
import os
import sys

# pass Kepler fits file as argument to script
if os.path.isfile(sys.argv[1]):
  f = fits.open(sys.argv[1])
else:
  print >> sys.stderr, "Could not read Kepler fits file"
  sys.exit(0)

# read light curve and time stamps
lc = f[1].data['PDCSAP_FLUX']
ts = f[1].data['TIME']

# remove NaNs
nanidx = np.invert(np.isnan(lc))
lc = lc[nanidx]
ts = ts[nanidx]

# remove median
lc = lc - np.median(lc)

posterior_sample = atleast_2d(loadtxt('posterior_sample.txt'))

saveFrames = False # For making movies
if saveFrames:
  os.system('rm Frames/*.png')

ion()
for i in xrange(0, posterior_sample.shape[0]):
  hold(False)
  plot(ts, lc, 'b.')
  hold(True)
  plot(ts, posterior_sample[i, 0:len(lc)], 'r')
  xlabel('Time', fontsize=16)
  ylabel('y', fontsize=16)
  draw()
  if saveFrames:
    savefig('Frames/' + '%0.4d'%(i+1) + '.png', bbox_inches='tight')
    print('Frames/' + '%0.4d'%(i+1) + '.png')

ioff()
show()
