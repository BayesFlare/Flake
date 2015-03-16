#ifndef _FlareDistribution_
#define _FlareDistribution_

#include "Distributions/Distribution.h"

class FlareDistribution:public Distribution
{
  private:
    // Limits
    double t0_min, t0_max; // limits on flare peak time
    double mu_min, mu_max;
    double min_width;
    
    // Mean of amplitudes and widths
    double mu, mu_widths;

    // Uniform for log-skews
    double a, b; // Midpoint and half-width

    double perturb_parameters();

  public:
    FlareDistribution(double t0_min, double t0_max, double mu_min, double mu_max);

    void fromPrior();

    double log_pdf(const std::vector<double>& vec) const;
    void from_uniform(std::vector<double>& vec) const;
    void to_uniform(std::vector<double>& vec) const;

    void print(std::ostream& out) const;

    static const int weight_parameter = 1;
};

#endif

