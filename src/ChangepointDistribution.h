#ifndef _ChangepointDistribution_
#define _ChangepointDistribution_

#include <DNest4.h>

// This distribution is based on the ClassicMassInf1D distribution
// in the magnetron code https://bitbucket.org/dhuppenkothen/magnetron
// (Copyright, 2013, Brewer, Frean, Hogg, Huppenkothen & Murray)
// as described in Huppenkothen et al, http://arxiv.org/abs/1501.05251

// convert value, r, between 0->1 into Gaussian with given mean and standard devaition (uses gsl_cdf_gaussian_Pinv function) 
double unit_to_gaussian(double r, double meanv, double sigmav);

class ChangepointDistribution:public DNest4::ConditionalPrior
{
  private:
    // Limits
    double tcp_min, tcp_max; // limits on when the change points can be
    
    // sigma of background level amplitude hyperparameter
    double sigma_back_amp;

    double perturb_hyperparameters(DNest4::RNG& rng);

  public:
    ChangepointDistribution(double tcp_min, double tcp_max);

    void from_prior(DNest4::RNG& rng);

    double log_pdf(const std::vector<double>& vec) const;
    void from_uniform(std::vector<double>& vec) const;
    void to_uniform(std::vector<double>& vec) const;

    void print(std::ostream& out) const;

    static const int weight_parameter = 1;
};

#endif
