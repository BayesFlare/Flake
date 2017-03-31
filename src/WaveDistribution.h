#ifndef _WaveDistribution_
#define _WaveDistribution_

#include <DNest4.h>

// Based on ClassicMassInf1D from RJObject
class WaveDistribution:public DNest4::ConditionalPrior
{
  private:
    static const DNest4::Cauchy cauchy;

    // amplitude and period distribution location and scale hyperparameters
    double mu_log_amp, scale_log_amp, mu_log_P, scale_log_P;

    double perturb_hyperparameters(DNest4::RNG& rng);

  public:
    WaveDistribution(); // constructor

    void from_prior(DNest4::RNG& rng);

    double log_pdf(const std::vector<double>& vec) const;
    void from_uniform(std::vector<double>& vec) const;
    void to_uniform(std::vector<double>& vec) const;

    void print(std::ostream& out) const;
    static const int weight_parameter = 1;
};

#endif

