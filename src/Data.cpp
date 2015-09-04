#include "Data.h"

#include <CCfits/CCfits>
#include <cmath>

using namespace CCfits;

Data Data::instance;

Data::Data()
{

}

// A function for reading in Kepler light curve data from a FITS file.
// This uses the C++ CFITSIO interface CCfits. 
// See http://heasarc.gsfc.nasa.gov/docs/software/fitsio/ccfits/html/readtable.html for an example

void Data::load(const char* filename)
{
  // use the verbose mode to output any errors in reading files
  FITS::setVerboseMode(true);
  
  // Empty the vectors
  t.clear(); // the light curve time stamps in days from the first value
  y.clear(); // the light curve flux
  
  // set the names of the Kepler FITS file HDUs
  std::vector<string> hdus(3);
  hdus[0] = "PRIMARY";
  hdus[1] = "LIGHTCURVE"; // contains the light curve data
  hdus[2] = "APERTURE";
  
  // create the FITS class (false means that all data is *not* read on construction)
  std::auto_ptr<FITS> pInfile(new FITS(filename, Read, hdus, false));
  
  // create a table for the LIGHTCURVE HDU 
  ExtHDU& table = pInfile->extension(hdus[1]);
  
  long nrows = table.rows(); // number of values in the light curve 
  
  // read in the times (in days in the light curve file) and make epoch from first time stamp
  table.column("TIME").read( t, 1, nrows ); // read from first row (1) to last
  double t0 = t[0];
  for ( std::vector<double>::iterator i=t.begin(); i != t.end(); ++i ) { *i -= t0;  }
  
  // data times step
  dt = t[1];
  
  // read in the light curve data from the PDCSAP_FLUX data channel
  //  - there are other channels available, see http://archive.stsci.edu/kepler/manuals/archive_manual.pdf
  table.column("PDCSAP_FLUX").read( y, 1, nrows );
  
  // loop through light curve and remove any NaN or inf entries
  std::vector<double>::iterator ity, itt;
  for (ity = y.begin(), itt = t.begin(); ity != y.end(); ){
    if ( isnan(*ity) || isinf(*ity) ){
      ity = y.erase(ity);
      itt = t.erase(itt);
    }
    else {
      ++ity;
      ++itt;
    }
  }

  // calculate median of the data
  std::vector<double> yc;
  yc = y; // copy of y
  size_t newsize = yc.size();
  std::sort(yc.begin(), yc.end()); // sort for median
  if ( newsize % 2 == 0 ){ y_median = (yc[newsize/2 - 1] + yc[newsize/2]) / 2.; }
  else{ y_median = yc[newsize/2]; }

  // calculate the mean of the data
  y_mean = 0.;
  for ( int i=0; i < (int)newsize; i++ ){ y_mean += y[i]; }
  y_mean /= (double)newsize;

  // remove the median from the data to rescale
  for ( int i=0; i < (int)newsize; i++ ){ y[i] -= y_median; }
}

