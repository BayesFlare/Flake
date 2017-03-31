#ifndef _ImpulseDistribution_
#define _ImpulseDistribution_

#include <DNest4.h>

// This distribution is based on the ClassicMassInf1D distribution
// in the magnetron code https://bitbucket.org/dhuppenkothen/magnetron
// (Copyright, 2013, Brewer, Frean, Hogg, Huppenkothen & Murray)
// as described in Huppenkothen et al, http://arxiv.org/abs/1501.05251

class ImpulseDistribution:public DNest4::ConditionalPrior
{
  private:
    static const DNest4::Cauchy cauchy;

    // Limits
    double t0_imp_min, t0_imp_max; // limits on impulse time
    
    // Mean of impulse amplitude hyperparameters
    double mu_log_imp_amp, scale_log_imp_amp;

    double perturb_hyperparameters(DNest4::RNG& rng);

  public:
    ImpulseDistribution(double t0_imp_min, double t0_imp_max);

    void from_prior(DNest4::RNG& rng);

    double log_pdf(const std::vector<double>& vec) const;
    void from_uniform(std::vector<double>& vec) const;
    void to_uniform(std::vector<double>& vec) const;

    void print(std::ostream& out) const;

    static const int weight_parameter = 1;
};

#endif

