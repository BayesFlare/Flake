#include "Data.h"

#include <CCfits/CCfits>

using namespace CCfits;

Data Data::instance;

Data::Data()
{

}

// A function for reading in Kepler light curve data from a FITS file.
// This uses the C++ CFITSIO interface CCfits. 
// See http://heasarc.gsfc.nasa.gov/docs/software/fitsio/ccfits/html/readtable.html for an example

// If the file name has a ".fits" extension then it will be assumed to be a Kepler light curve
// stored in the FITS format. If the file name has the ".txt" extension then it will be
// assumed to be an ascii text file containing two columns: time stamps and data points.

void Data::load(const char* filename)
{
  // use the verbose mode to output any errors in reading files
  FITS::setVerboseMode(true);
  
  std::string filenamestr(filename); // convert file name to string

  // get file extension
  std::string fileext  = boost::filesystem::extension(filenamestr);

  // Empty the vectors
  t.clear(); // the light curve time stamps in days from the first value
  y.clear(); // the light curve flux

  if ( fileext == ".fits" ){ // a Kepler light curve
    // set the names of the Kepler FITS file HDUs
    std::vector<string> hdus(3);
    hdus[0] = "PRIMARY";
    hdus[1] = "LIGHTCURVE"; // contains the light curve data
    hdus[2] = "APERTURE";

    // create the FITS class (false means that all data is *not* read on construction)
    std::unique_ptr<FITS> pInfile(new FITS(filename, Read, hdus, false));

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
      if ( std::isnan(*ity) || std::isinf(*ity) ){
        ity = y.erase(ity);
        itt = t.erase(itt);
      }
      else {
        ++ity;
        ++itt;
      }
    }
  }
  else if ( fileext == ".txt" ){ // an ascii text file with two columns (# or % denote comment lines)
    std::ifstream infile(filenamestr);
    double timestamp = 0., dataval = 0.;

    std::string line; // lines from file
    // read in file line-by-line
    while( std::getline(infile, line) ){
      // check if line starts with '#' or '%'
      if ( line.substr(0, 1) == "#" || line.substr(0, 1) == "%" ){ // line is a comment
        continue;
      }
      std::istringstream iss(line);
      if (!(iss >> timestamp >> dataval)) {
        std::cerr << "Error... problem reading in ascii text file \"" << filenamestr << "\"" << std::endl;
        exit(1);
      }
      else{
        t.push_back(timestamp);
        y.push_back(dataval);
      }
    }
  }
  else{
    std::cerr << "Error... \"" << fileext << "\" file extension not recognised" << std::endl;
    exit(1);
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

