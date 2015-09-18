#ifndef _ChangepointDistribution_
#define _ChangepointDistribution_

#include <rjobject/Distribution.h>

// This distribution is based on the ClassicMassInf1D distribution
// in the magnetron code https://bitbucket.org/dhuppenkothen/magnetron
// (Copyright, 2013, Brewer, Frean, Hogg, Huppenkothen & Murray)
// as described in Huppenkothen et al, http://arxiv.org/abs/1501.05251

class ChangepointDistribution:public Distribution
{
  private:
    // Limits
    double tcp_min, tcp_max; // limits on when the change points can be
    
    // Mean of background level amplitude hyperparameter
    double mu_back_amp;

    double perturb_parameters();

  public:
    ChangepointDistribution(double tcp_min, double tcp_max);

    void fromPrior();

    double log_pdf(const std::vector<double>& vec) const;
    void from_uniform(std::vector<double>& vec) const;
    void to_uniform(std::vector<double>& vec) const;

    void print(std::ostream& out) const;

    static const int weight_parameter = 1;
};

#endif