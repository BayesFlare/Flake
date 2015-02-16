#include <iostream>
#include <Start.h>
#include "MyModel.h"
#include "Data.h"

using namespace std;
using namespace DNest3;

int main(int argc, char** argv)
{
  // Load the data
  CommandLineOptions options(argc, argv);
  string dataFile = options.get_dataFile();
  if(dataFile.compare("") == 0)
    cerr << "# ERROR: Kepler FITS filename required with -d argument." <<endl;
  else
    Data::get_instance().load(dataFile.c_str());
        
  MTSampler<MyModel> sampler = setup_mt<MyModel>(argc, argv);
  sampler.run();

  return 0;
}

