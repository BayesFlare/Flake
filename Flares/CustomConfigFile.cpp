#include "CustomConfigFile.h"
#include <sstream>
#include <cstdlib>

using namespace std;

using boost::property_tree::ptree;
using boost::property_tree::read_json;

/* This class expects a JSON file containing the maximum number of sinusoids
 * required in the model, otherwise the default is set to 10, e.g.
 * {
 *   "SinusoidModel":
 *   {
 *     "MaxSinusoids": 20
 *   }
 * }
 */

// an instance
CustomConfigFile CustomConfigFile::instance;

// constructor 
CustomConfigFile::CustomConfigFile()
{

}

void CustomConfigFile::load(string configFileInput)
{
  configFile = configFileInput; // set the config file

  // read in the JSON configuration file
  try{
    read_json(configFile, pt); // put json file into property tree
  }
  catch( ... ){
    cout << "Could not read in JSON file " << configFile << endl;
    cout << "Reverting to default values." << endl;
  }

  // set custom options
  try{
    string maxSinusoidsString = pt.get<string> ("SinusoidModel.MaxSinusoids");
    maxSinusoids = atoi(maxSinusoidsString.c_str());
  }
  catch( ... ){
    // set default value
    maxSinusoids = 10;
  }
}

