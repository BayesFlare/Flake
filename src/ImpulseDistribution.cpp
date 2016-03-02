#include "ImpulseDistribution.h"
#include "RNG.h"
#include "Utils.h"
#include "Data.h"
#include <cmath>

using namespace DNest4;

ImpulseDistribution::ImpulseDistribution(double t0_imp_min, double t0_imp_max)
:t0_imp_min(t0_imp_min)
,t0_imp_max(t0_imp_max)
{

}


// function to generate prior hyperparameters from their respective priors
void ImpulseDistribution::fromPrior(RNG& rng)
{
  mu_imp_amp = tan(M_PI*(0.97*rng.rand() - 0.485)); // generate amplitude prior hyperparameter from Cauchy distribution
  mu_imp_amp = exp(mu_imp_amp);
}


// function to increment the various parameter prior hyperparameters using their respective proposals
double ImpulseDistribution::perturb_parameters(RNG& rng)
{
  double logH = 0.;

  // impulse amplitude prior hyperparameter
  mu_imp_amp = log(mu_imp_amp);
  mu_imp_amp = (atan(mu_imp_amp)/M_PI + 0.485)/0.97; // Cauchy distribution proposal
  mu_imp_amp += pow(10., 1.5 - 6.*rng.rand())*rng.randn();
  mu_imp_amp = mod(mu_imp_amp, 1.);
  mu_imp_amp = tan(M_PI*(0.97*mu_imp_amp - 0.485));
  mu_imp_amp = exp(mu_imp_amp);


  return logH;
}


// function to return the log prior pdf for mu
double ImpulseDistribution::log_pdf(const std::vector<double>& vec) const
{
  // check parameters are within prior ranges
  if (vec[1] < 0.0){ return -1E300; }

  return -log(mu_imp_amp) - vec[1]/mu_imp_amp;
}


// function to convert the impulse parameters from a unit hypercube into the true values
void ImpulseDistribution::from_uniform(std::vector<double>& vec) const
{
  // impulse must be on a time bin so return a bin index as vec[0]
  vec[0] = floor((double)(Data::get_instance().get_len()-1)*vec[0]);
  vec[1] = -mu_imp_amp*log(1. - vec[1]);
}


// function to convert the impulse parameters into a uniform unit hypercube
void ImpulseDistribution::to_uniform(std::vector<double>& vec) const
{
  // vec[0] is a bin index rather than a time, so get the actual time
  vec[0] = (Data::get_instance().get_t()[(size_t)vec[0]] - t0_imp_min)/(t0_imp_max - t0_imp_min);
  vec[1] = 1. - exp(-vec[1]/mu_imp_amp);
}


void ImpulseDistribution::print(std::ostream& out) const
{
  out<<mu_imp_amp<<' ';
}
