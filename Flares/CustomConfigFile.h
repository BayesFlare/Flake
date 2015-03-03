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
    int maxSinusoids;    // the maximum number of sinudoid models
    double minLogPeriod; // the minimum natural log of sinusoid periods in days
    double maxLogPeriod; // the maximum natural log of sinusoid periods in days
    double minWaveMu;    // the minimum of the mean of the exponential amplitude distribution
    double maxWaveMu;    // the maximum of the mean of the exponential amplitude distribution

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

  // Singleton
  private:
    static CustomConfigFile instance;
  public:
    static CustomConfigFile& get_instance() { return instance; }
};

#endif

