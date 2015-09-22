#ifndef _Data_
#define _Data_

#include <vector>

class Data
{
  private:
    std::vector<double> t, y;

  public:
    Data();
    void load(const char* filename);

    // some useful information
    double y_mean, y_median;
    double dt;
                
    // Getters
    const std::vector<double>& get_t() const { return t; }  // get vector of time stamps
    const std::vector<double>& get_y() const { return y; }  // get light curve vector
    double get_mean() const { return y_mean; }              // get mean of light curve
    double get_median() const { return y_median; }          // get median of light curve
    double get_dt() const { return dt; }                    // get time interval between light curve point
    double get_tstart() const { return t[0]; }              // start time of data
    double get_tend() const { return t[t.size()-1]; }       // end time of data
    unsigned int get_len() const { return t.size(); }                  // number of data points (length)
    
  // Singleton
  private:
    static Data instance;
  public:
    static Data& get_instance() { return instance; }
};

#endif

