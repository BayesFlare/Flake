## Flare estimation in Kepler data

This is a fork of Brendon Brewer's RJObject code (which allows birth/death MCMC moves within the DNest3 code) with the specific aim of performing parameter estimation for stellar flares in data from the _Kepler_ satellite.

The `Data` class in this example will attempt to read in the `TIME` and [`PDCSAP_FLUX`](http://archive.stsci.edu/kepler/manuals/archive_manual.pdf) light curve data from a _Kepler_ FITS file. It will set the epoch of the time stamps (read in as days) to be the first time stamp value and remove any data containing NaNs or infinities. The light curve data will have the median value removed from it. The aim is to be able to simultaneously fit a variable number of background sinusoids and also a variable number of flares. The maximum number of sinusoids allowed in the model can be set through a custom JSON configuration file, rather than having it hardwired in the code (Note that for this to work the patch to [DNest3](https://github.com/eggplantbren/DNest3) in the `Flare` directory must have been applied and DNest3 recompiled). I have started to implement the flare burst model from the [magnetron](https://bitbucket.org/dhuppenkothen/magnetron/) code (by Huppenkothen, Brewer, Frean, Hogg & Murray) as described in Huppenkothen _et al_, [arXiv:1501.05251](http://arxiv.org/abs/1501.05251). This currently is unlikely to work though!

The DNest3 algorithm is described in Brewer _et al_, [arXiv:0912.2380](http://arxiv.org/abs/0912.2380) and the documentation can be found [within the code repository](https://github.com/eggplantbren/DNest3/tree/master/doc). The additional RJObject code is described in Brewer, [arXiv:1411.3921](http://arxiv.org/abs/1411.3921).

#### Requirements
 * [DNest3](https://github.com/eggplantbren/DNest3) - the current Makefile assumes the static library is installed in `/usr/local/lib` and the patch in the `Flare` directory must have been applied.
 * [CFITSIO](http://heasarc.gsfc.nasa.gov/docs/software/fitsio/fitsio.html) - on Debian/Ubuntu install with e.g. `sudo apt-get install libcfitsio3-dev` (on 64bit systems the libraries will be installed in `/usr/lib/x86_64-linux-gnu`, so this is included in the Makefile)
 * [CCfits](http://heasarc.gsfc.nasa.gov/docs/software/fitsio/ccfits/index.html) - on Debian/Ubuntu install with e.g. `sudo apt-get install libccfits-dev`

&copy; 2013 Brendon J. Brewer, 2015 Matthew Pitkin

Licence: GNU GPL v3 for software.
