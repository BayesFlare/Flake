## Flare estimation in Kepler data

This code's aim is to performing parameter estimation for stellar flares in data from the _Kepler_ satellite.

The `Data` class in this example will attempt to read in the `TIME` and
[`PDCSAP_FLUX`](http://archive.stsci.edu/kepler/manuals/archive_manual.pdf) light curve data from a _Kepler_
FITS file. It will set the epoch of the time stamps (read in as days) to be the first time stamp value and
remove any data containing NaNs or infinities. The light curve data will have the median value removed from it.
The aim is to be able to simultaneously fit a variable number of background/noise components (in this case
sinusoids, single-bin impulses and step changes in the underlying background level) and also a variable
number of flares. The maximum number of the various allowed model components (and some of their ranges)
can be set through a custom JSON configuration file, rather than having it hardwired in the code.
I have started to implement the flare burst model from the [magnetron](https://bitbucket.org/dhuppenkothen/magnetron/)
code (by Huppenkothen, Brewer, Frean, Hogg & Murray) as described in Huppenkothen _et al_,
[arXiv:1501.05251](http://arxiv.org/abs/1501.05251). This currently is unlikely to work though!

The DNest4 algorithm is described in Brewer _et al_, [arXiv:0912.2380](http://arxiv.org/abs/0912.2380) and the documentation can be found [within the code repository](https://github.com/eggplantbren/DNest4/tree/master/doc). The additional RJObject code is described in Brewer, [arXiv:1411.3921](http://arxiv.org/abs/1411.3921).

#### Requirements
 * [DNest4](https://github.com/eggplantbren/DNest4)
 * [CFITSIO](http://heasarc.gsfc.nasa.gov/docs/software/fitsio/fitsio.html) - on Debian/Ubuntu install with e.g. `sudo apt-get install libcfitsio3-dev`
 * [CCfits](http://heasarc.gsfc.nasa.gov/docs/software/fitsio/ccfits/index.html) - on Debian/Ubuntu install with e.g. `sudo apt-get install libccfits-dev`
 * [Boost](http://www.boost.org/) - in particulate the boost IOStreams and System libraries, on Debian/Ubuntu install with e.g. `sudo apt-get install libboost-dev`

#### Compilation

Compilation of the code makes use of [scons](http://scons.org). To compile the code just run `scons` in the base directory

&copy; 2013 Brendon J. Brewer, 2015 Matthew Pitkin

Licence: [MIT License](https://opensource.org/licenses/MIT) for software.
