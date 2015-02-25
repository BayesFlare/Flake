#include <iostream>
#include <Start.h>
#include "MyModel.h"
#include "Data.h"
#include "CustomConfigFile.h"

using namespace std;
using namespace DNest3;

int main(int argc, char** argv)
{
  // Load the data
  CommandLineOptions options(argc, argv);
  string dataFile = options.get_dataFile();

  // custom options file
  string configFile = options.get_configFile();

  // load custom configuration file (if config file is "" load will set defaults)
  CustomConfigFile::get_instance().load(configFile);

  if(dataFile.compare("") == 0){
    cerr << "# ERROR: Kepler FITS filename required with -d argument." <<endl;
    return 1;
  }
  else
    Data::get_instance().load(dataFile.c_str());
        
  MTSampler<MyModel> sampler = setup_mt<MyModel>(options);
  sampler.run();

  return 0;
}

