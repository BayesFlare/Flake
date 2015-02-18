## Flare estimation in Kepler data

This is a fork of Brendon Brewer's RJObject code (which allows birth/death MCMC moves within the DNest3 code) with the specific aim of performing parameter estimation for stellar flares in data from the _Kepler_ satellite.

The `Data` class in this example will attempt to read in the `TIME` and [`PDCSAP_FLUX`](http://archive.stsci.edu/kepler/manuals/archive_manual.pdf) light curve data from a _Kepler_ FITS file. It will set the epoch of the time stamps (read in as days) to be the first time stamp value and remove any data containing NaNs or infinities. The light curve data will have the median value removed from it.

#### Requirements
 * [DNest3](https://github.com/eggplantbren/DNest3) - the current Makefile assumes the static library is installed in `/usr/local/lib`
 * [CFITSIO](http://heasarc.gsfc.nasa.gov/docs/software/fitsio/fitsio.html) - on Debian/Ubuntu install with e.g. `sudo apt-get install libcfitsio3-dev` (on 64bit systems the libraries will be installed in `/usr/lib/x86_64-linux-gnu`, so this is included in the Makefile)
 * [CCfits](http://heasarc.gsfc.nasa.gov/docs/software/fitsio/ccfits/index.html) - on Debian/Ubuntu install with e.g. `sudo apt-get install libccfits-dev`

&copy; 2013 Brendon J. Brewer, 2015 Matthew Pitkin

Licence: GNU GPL v3 for software.
