#include <iostream>
#include <Start.h>
#include "FlareWave.h"
#include "Data.h"
#include "CustomConfigFile.h"

using namespace std;
using namespace DNest4;

int main(int argc, char** argv)
{
  // parse the command line options
  CommandLineOptions options(argc, argv);

  // load a Kepler light curve file
  string dataFile = options.get_data_file();
  if(dataFile.compare("") == 0){
    cerr << "# ERROR: Kepler FITS file or ascii text file required with -d argument." <<endl;
    return 1;
  }
  else{ Data::get_instance().load(dataFile.c_str()); }

  /* load custom configuration file (if config file is "" load will set defaults)
   *  this file should set up the flare and sinusoid parameter ranges.
   */ 
  string configFile = options.get_config_file();
  CustomConfigFile::get_instance().load(configFile);
  start<FlareWave>(options); // run the code

  return 0;
}

