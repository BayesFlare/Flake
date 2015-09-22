#include "CustomConfigFile.h"
#include "Data.h"
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
 *   "FlareModel":
 *   {
 *     "MaxFlares": 100
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
    maxSinusoids = 100;
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

  // maximum number of flare components
  try{
    string maxFlaresString = pt.get<string> ("FlareModel.MaxFlares");
    maxFlares = atoi(maxFlaresString.c_str());
  }
  catch( ... ){
    // set default value
    maxFlares = 100;
  }

  // minimum of the flare central time (days)
  try{
    string minFlareT0String = pt.get<string> ("FlareModel.MinFlareT0");
    minFlareT0 = atof(minFlareT0String.c_str());
  }
  catch( ... ){
    // set default min value (set to the start time from the data)
    minFlareT0 = Data::get_instance().get_tstart();
  }

  // maximum of the flare central time (days)
  try{
    string maxFlareT0String = pt.get<string> ("FlareModel.MaxFlareT0");
    maxFlareT0 = atof(maxFlareT0String.c_str());
  }
  catch( ... ){
    // set default min value (set to the end time from the data)
    maxFlareT0 = Data::get_instance().get_tend();
  }

  // minimum width of an individual flare's rise time
  try{
    string minFlareRiseWidthString = pt.get<string> ("FlareModel.MinFlareRiseWidth");
    minFlareRiseWidth = atof(minFlareRiseWidthString.c_str());
  }
  catch( ... ){
    // set default minimum flare rise width (days)
    minFlareRiseWidth = 0.25/24.; // quarter of an hour in days
  }

  // minimum width of an individual flare's decay time
  try{
    string minFlareDecayWidthString = pt.get<string> ("FlareModel.MinFlareDecayWidth");
    minFlareDecayWidth = atof(minFlareDecayWidthString.c_str());
  }
  catch( ... ){
    // set default minimum flare decay width (days)
    minFlareDecayWidth = 1./24.; // one hour in days
  }

  // maximum number of background changepoint components
  try{
    string maxImpulsesString = pt.get<string> ("Impulses.MaxImpulses");
    maxImpulses = atoi(maxImpulsesString.c_str());
  }
  catch( ... ){
    // set default value
    maxImpulses = 50;
  }

  // maximum number of background changepoint components
  try{
    string maxChangepointsString = pt.get<string> ("Changepoints.MaxChangepoints");
    maxChangepoints = atoi(maxChangepointsString.c_str());
  }
  catch( ... ){
    // set default value
    maxChangepoints = 5;
  }
}
