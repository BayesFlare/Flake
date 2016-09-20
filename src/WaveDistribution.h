#ifndef _WaveDistribution_
#define _WaveDistribution_

#include <DNest4.h>

// Based on ClassicMassInf1D from RJObject
class WaveDistribution:public DNest4::ConditionalPrior
{
  private:
    // Limits
    double logP_min, logP_max; // upper and lower ranges of log(period)
    double mu_min, mu_max;     // upper and lower ranges for amplitude prior hyperparameter mu

    // Mean of exponential distribution for amplitudes
    double mu;

    double perturb_hyperparameters(DNest4::RNG& rng);

  public:
    WaveDistribution(double logP_min, double logP_max, double mu_min, double mu_max); // constructor

    void from_prior(DNest4::RNG& rng);

    double log_pdf(const std::vector<double>& vec) const;
    void from_uniform(std::vector<double>& vec) const;
    void to_uniform(std::vector<double>& vec) const;

    void print(std::ostream& out) const;
    static const int weight_parameter = 1;
};

#endif

