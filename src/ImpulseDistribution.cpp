#include "ImpulseDistribution.h"
#include "RNG.h"
#include "Utils.h"
#include "Data.h"
#include <cmath>

using namespace DNest4;

const DNest4::Cauchy ImpulseDistribution::cauchy(0.0, 5.0);

ImpulseDistribution::ImpulseDistribution(double t0_imp_min, double t0_imp_max)
:t0_imp_min(t0_imp_min)
,t0_imp_max(t0_imp_max)
{

}


// function to generate prior hyperparameters from their respective priors
void ImpulseDistribution::from_prior(RNG& rng)
{
  mu_log_imp_amp = cauchy.generate(rng); // generate log amplitude prior hyperparameter from Cauchy distribution
  scale_log_imp_amp = 5.*rng.rand();
}


// function to increment the various parameter prior hyperparameters using their respective proposals
double ImpulseDistribution::perturb_hyperparameters(RNG& rng)
{
  double logH = 0.;

  int which = rng.rand_int(2); // set equal probability for incrementing each hyperparameter
  if ( which == 0 ){
    logH += cauchy.perturb(mu_log_imp_amp, rng);
  }
  else {
    // flare amplitude scale prior hyperparameter
    scale_log_imp_amp += 5.0*rng.randh();
    wrap(scale_log_imp_amp, 0., 5.);
  } 

  return logH;
}


// function to return the log prior pdf for mu
double ImpulseDistribution::log_pdf(const std::vector<double>& vec) const
{
  double logp = 0.0;

  Laplace l1(mu_log_imp_amp, scale_log_imp_amp);
  logp += l1.log_pdf(vec[1]);

  // check parameters are within prior ranges
  if (vec[0] < t0_imp_min || vec[0] > t0_imp_max){ return -1E300; }

  return logp;
}


// function to convert the impulse parameters from a unit hypercube into the true values
void ImpulseDistribution::from_uniform(std::vector<double>& vec) const
{
  Laplace l1(mu_log_imp_amp, scale_log_imp_amp);

  // impulse must be on a time bin so return a bin index as vec[0]
  vec[0] = floor((double)(Data::get_instance().get_len()-1)*vec[0]);
  vec[1] = l1.cdf_inverse(vec[1]);
}


// function to convert the impulse parameters into a uniform unit hypercube
void ImpulseDistribution::to_uniform(std::vector<double>& vec) const
{
  Laplace l1(mu_log_imp_amp, scale_log_imp_amp);

  // vec[0] is a bin index rather than a time, so get the actual time
  vec[0] = (Data::get_instance().get_t()[(size_t)vec[0]] - t0_imp_min)/(t0_imp_max - t0_imp_min);
  vec[1] = l1.cdf(vec[1]);
}


void ImpulseDistribution::print(std::ostream& out) const
{
  out<<mu_log_imp_amp<<' '<<scale_log_imp_amp<<' ';
}

