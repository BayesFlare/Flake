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

const DNest4::Cauchy WaveDistribution::cauchy(0.0, 5.0);

// Constructor
WaveDistribution::WaveDistribution()
{

}


// function to draw sinusoid amplitude prior hyperparameter mu from its prior
void WaveDistribution::from_prior(RNG& rng)
{
  mu_log_P = cauchy.generate(rng);
  scale_log_P = 5.*rng.rand();

  mu_log_amp = cauchy.generate(rng);
  scale_log_amp = 5.*rng.rand();
}


// function to increment the amplitude prior hyperparameter (mu) using a proposal distribution
double WaveDistribution::perturb_hyperparameters(RNG& rng)
{
  double logH = 0.; // proposal ratio

  int which = rng.rand_int(4);

  if(which == 0){
    logH += cauchy.perturb(mu_log_P, rng);
  }
  else if(which == 1){
    scale_log_P += 5.0*rng.randh();
    wrap(scale_log_P, 0.0, 5.0);
  }
  else if(which == 2){
    logH += cauchy.perturb(mu_log_amp, rng);
  }
  else{
    scale_log_amp += 5.0*rng.randh();
    wrap(scale_log_amp, 0.0, 5.0);
  }

  return logH;
}

// vec[0] = log-period
// vec[1] = log-amplitude
// vec[2] = phase


// function to return the log prior pdf for mu
double WaveDistribution::log_pdf(const std::vector<double>& vec) const
{
  double logp = 0.;

  Laplace l1(mu_log_P, scale_log_P);
  Laplace l2(mu_log_amp, scale_log_amp);

  logp += l1.log_pdf(vec[0]);
  logp += l2.log_pdf(vec[1]);

  // check parameters are within prior ranges
  if(vec[2] < 0. || vec[2] > 2.*M_PI) { return -1E300; } // if not log(prior) = -inf
  else { logp += -log(2.*M_PI); }

  // prior pdf for mu
  return logp;
}


// function to convert the sinusoid parameters from a unit hypercube into the true values
void WaveDistribution::from_uniform(std::vector<double>& vec) const
{
  Laplace l1(mu_log_P, scale_log_P);
  Laplace l2(mu_log_amp, scale_log_amp);

  vec[0] = l1.cdf_inverse(vec[0]);
  vec[1] = l2.cdf_inverse(vec[1]);
  vec[2] = 2.*M_PI*vec[2];
}


// function to convert the sinusoid parameters into a uniform unit hypercube
void WaveDistribution::to_uniform(std::vector<double>& vec) const
{
  Laplace l1(mu_log_P, scale_log_P);
  Laplace l2(mu_log_amp, scale_log_amp);

  vec[0] = l1.cdf(vec[0]);
  vec[1] = l2.cdf(vec[1]);
  vec[2] = vec[2]/(2.*M_PI);
}


// output the sinusoid amplitude prior hyperparameter (mu)
void WaveDistribution::print(std::ostream& out) const
{
  out<<mu_log_P<<' '<<scale_log_P<<' '<<mu_log_amp<<' '<<scale_log_amp<<' ';
}

