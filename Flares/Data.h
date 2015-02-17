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
    double y_mean, ymedian;
    double dt
                
    // Getters
    const std::vector<double>& get_t() const { return t; }
    const std::vector<double>& get_y() const { return y; }
    double get_mean() const { return y_mean; }
    double get_median() const { return y_median; }
    double get_dt() const { return dt; }

  // Singleton
  private:
    static Data instance;
  public:
    static Data& get_instance() { return instance; }
};

#endif

