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
    int maxSinusoids; // the maximum number of sinudoid models

  public:
    CustomConfigFile();
    void load(std::string configFile);

    const ptree& get_ptree() const
    { return pt; }

    // return the maximum number of sinusoids allowed
    int get_maxSinusoids() const
    { return maxSinusoids; }

  // Singleton
  private:
    static CustomConfigFile instance;
  public:
    static CustomConfigFile& get_instance() { return instance; }
};

#endif

