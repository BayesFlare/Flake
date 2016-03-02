#include "WaveDistribution.h"
#include "RNG.h"
#include "Utils.h"
#include <cmath>


/* Functions for the sinusoid RJObject's (i.e. objects for which there can be a
 * variable number). These define the prior and proposal for any parameter prior
 * hyperparameters (in this case the mu parameter defining the prior on the
 * sinusoid amplitudes), the prior pdf, and the parameter conversions to and from
 * the unit hypercube.
 */

using namespace DNest4;

// Constructor
WaveDistribution::WaveDistribution(double logP_min, double logP_max, double mu_min, double mu_max)
:logP_min(logP_min)
,logP_max(logP_max)
,mu_min(mu_min)
,mu_max(mu_max)
{

}


// function to draw sinusoid amplitude prior hyperparameter mu from its prior
void WaveDistribution::fromPrior(RNG& rng)
{
  mu = exp(log(mu_min) + log(mu_max/mu_min)*rng.rand());
}


// function to increment the amplitude prior hyperparameter (mu) using a proposal distribution
double WaveDistribution::perturb_parameters(RNG& rng)
{
  double logH = 0.; // proposal ratio

  mu = log(mu); // convert mu back into log-space
  
  // draw new value from proposal and increment
  mu += log(mu_max/mu_min)*pow(10., 1.5 - 6.*rng.rand())*rng.randn();
  mu = mod(mu - log(mu_min), log(mu_max/mu_min)) + log(mu_min);
  
  mu = exp(mu); // convert back to mu value

  return logH;
}

// vec[0] = log-period
// vec[1] = amplitude
// vec[2] = phase


// function to return the log prior pdf for mu
double WaveDistribution::log_pdf(const std::vector<double>& vec) const
{
  // check parameters are within prior ranges
  if(vec[0] < logP_min || vec[0] > logP_max || vec[1] < 0. || vec[2] < 0. || vec[2] > 2.*M_PI)
    return -1E300; // if not log(prior) = -inf

  // prior pdf for mu
  return -log(mu) - vec[1]/mu;
}


// function to convert the sinusoid parameters from a unit hypercube into the true values
void WaveDistribution::from_uniform(std::vector<double>& vec) const
{
  vec[0] = logP_min + (logP_max - logP_min)*vec[0];
  vec[1] = -mu*log(1. - vec[1]);
  vec[2] = 2.*M_PI*vec[2];
}


// function to convert the sinusoid parameters into a uniform unit hypercube
void WaveDistribution::to_uniform(std::vector<double>& vec) const
{
  vec[0] = (vec[0] - logP_min)/(logP_max - logP_min);
  vec[1] = 1. - exp(-vec[1]/mu);
  vec[2] = vec[2]/(2.*M_PI);
}


// output the sinusoid amplitude prior hyperparameter (mu)
void WaveDistribution::print(std::ostream& out) const
{
  out<<mu<<' ';
}
