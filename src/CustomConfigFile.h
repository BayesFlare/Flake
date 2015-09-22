#ifndef _CustomConfigFile_h
#define _CustomConfigFile_h

/* file to read in a custom JSON configuration file */

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <string>

using boost::property_tree::ptree;
using boost::property_tree::read_json;

class CustomConfigFile{
  private:
    std::string configFile;
    ptree pt;

    // custom parameters
    
    // sinusoid parameters
    int maxSinusoids;     // the maximum number of sinusoids
    double minLogPeriod;  // the minimum natural log of sinusoid periods in days
    double maxLogPeriod;  // the maximum natural log of sinusoid periods in days
    double minWaveMu;     // the minimum of the mean of the exponential amplitude distribution
    double maxWaveMu;     // the maximum of the mean of the exponential amplitude distribution

    // flare parameters
    int maxFlares;             // the maximum number of flares
    double minFlareT0;         // the minimum peak time of the flares
    double maxFlareT0;         // the maximum peak time of the flares
    double minFlareRiseWidth;  // the minimum width of a single flare's rise time
    double minFlareDecayWidth; // the minimum width of a single flare's decay time
    
    // impulse parameters
    int maxImpulses; // the maximum number of impulses
    
    // change point parameters
    int maxChangepoints;  // the maximum number of background change points
    
  public:
    CustomConfigFile();
    void load(std::string configFile);

    const ptree& get_ptree() const
    { return pt; }

    // return the maximum number of sinusoids allowed
    int get_maxSinusoids() const
    { return maxSinusoids; }

    // return the maximum log period
    double get_maxLogPeriod() const
    { return maxLogPeriod; }

    // return the minimum log period
    double get_minLogPeriod() const
    { return minLogPeriod; }

    // return the maximum wave mu
    double get_maxWaveMu() const
    { return maxWaveMu; }

    // return the minimum wave mu
    double get_minWaveMu() const
    { return minWaveMu; } 

    // return the maximum number of flares allowed
    int get_maxFlares() const
    { return maxFlares; }

    // return the maximum central time of the flare (in days)
    double get_maxFlareT0() const
    { return maxFlareT0; }

    // return the minimum central time of the flare (in days)
    double get_minFlareT0() const
    { return minFlareT0; }
    
    // return the minimum flare rise width (in days)
    double get_minFlareRiseWidth() const
    { return minFlareRiseWidth; }
    
    // return the minimum flare decay width (in days)
    double get_minFlareDecayWidth() const
    { return minFlareDecayWidth; }
    
    // return the maximum number of allowed impulses
    int get_maxImpulses() const
    { return maxImpulses; }
    
    // return the maximum number of allowed change points
    int get_maxChangepoints() const
    { return maxChangepoints; }
    
  // Singleton
  private:
    static CustomConfigFile instance;
  public:
    static CustomConfigFile& get_instance() { return instance; }
};

#endif

