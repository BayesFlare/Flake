#ifndef _FlareDistribution_
#define _FlareDistribution_

#include <DNest4.h>

// This distribution is based on the ClassicMassInf1D distribution
// in the magnetron code https://bitbucket.org/dhuppenkothen/magnetron
// (Copyright, 2013, Brewer, Frean, Hogg, Huppenkothen & Murray)
// as described in Huppenkothen et al, http://arxiv.org/abs/1501.05251

class FlareDistribution:public DNest4::ConditionalPrior
{
  private:
    // Limits
    double t0_min, t0_max; // limits on flare peak time
    double min_rise_width, min_decay_width;
    
    // Mean of amplitudes and width prior hyperparameters
    double mu_amp, mu_rise_width, mu_decay_width;

    double perturb_hyperparameters(DNest4::RNG& rng);

  public:
    FlareDistribution(double t0_min, double t0_max, double min_rise_width, double min_decay_width);

    void from_prior(DNest4::RNG& rng);

    double log_pdf(const std::vector<double>& vec) const;
    void from_uniform(std::vector<double>& vec) const;
    void to_uniform(std::vector<double>& vec) const;

    void print(std::ostream& out) const;

    static const int weight_parameter = 1;
};

#endif

