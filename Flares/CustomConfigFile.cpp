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
  
  // maximum number of sinusoid components
  try{
    string maxSinusoidsString = pt.get<string> ("SinusoidModel.MaxSinusoids");
    maxSinusoids = atoi(maxSinusoidsString.c_str());
  }
  catch( ... ){
    // set default value
    maxSinusoids = 10;
  }

  // maximum natural log of periods in days
  try{
    string maxLogPeriodString = pt.get<string> ("SinusoidModel.MaxLogPeriod");
    maxLogPeriod = atof(maxLogPeriodString.c_str());
  }
  catch( ... ){
    // set default max log period
    maxLogPeriod = 10.;
  }

  // minimum natural log of periods in days
  try{
    string minLogPeriodString = pt.get<string> ("SinusoidModel.MinLogPeriod");
    minLogPeriod = atof(minLogPeriodString.c_str());
  }
  catch( ... ){
    // set default max log period
    minLogPeriod = -10.;
  }

  // maximum of the mean of the exponential sinusoid amplitude distribution
  try{
    string maxWaveMuString = pt.get<string> ("SinusoidModel.MaxWaveMu");
    maxWaveMu = atof(maxWaveMuString.c_str());
  }
  catch( ... ){
    // set default max value
    maxWaveMu = 1.e3;
  }

  // minimum of the mean of the exponential sinusoid amplitude distribution
  try{
    string minWaveMuString = pt.get<string> ("SinusoidModel.MinWaveMu");
    minWaveMu = atof(minWaveMuString.c_str());
  }
  catch( ... ){
    // set default max value
    minWaveMu = 1.e-3;
  }
}

