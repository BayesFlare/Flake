#include "FlareDistribution.h"
#include "RNG.h"
#include "Utils.h"
#include "Data.h"
#include <cmath>

using namespace DNest4;

const DNest4::Cauchy FlareDistribution::cauchy(0.0, 5.0);

FlareDistribution::FlareDistribution(double t0_min, double t0_max, double min_rise_width, double min_decay_width)
:t0_min(t0_min)
,t0_max(t0_max)
,min_rise_width(min_rise_width)
,min_decay_width(min_decay_width)
{

}


// function to generate prior hyperparameters from their respective priors
void FlareDistribution::from_prior(RNG& rng)
{
  mu_log_amp = cauchy.generate(rng); // generate log amplitude prior hyperparameter from Cauchy distribution
  scale_log_amp = 5.*rng.rand();

  mu_rise_width = exp(log(1E-3*(t0_max - t0_min)) + log(1E3)*rng.rand());  // generate rise time hyperparameter from log uniform distribution
  mu_decay_width = exp(log(1E-3*(t0_max - t0_min)) + log(1E3)*rng.rand()); // generate decay time hyperparameter from log uniform distribution
}


// function to increment the various parameter prior hyperparameters using their respective proposals
double FlareDistribution::perturb_hyperparameters(RNG& rng)
{
  double logH = 0.;

  int which = rng.rand_int(4); // set equal probability for incrementing each hyperparameter

  if(which == 0){
    // flare amplitude location prior hyperparameter
    logH += cauchy.perturb(mu_log_amp, rng);
  }
  else if( which == 1){
    // flare amplitude scale prior hyperparameter
    scale_log_amp += 5.0*rng.randh();
    wrap(scale_log_amp, 0., 5.);
  } 
  else if(which == 2){
    mu_rise_width = log(mu_rise_width/(t0_max - t0_min));
    mu_rise_width += log(1E3)*pow(10., 1.5 - 6.*rng.rand())*rng.randn();
    mu_rise_width = mod(mu_rise_width - log(1E-3), log(1E3)) + log(1E-3);
    mu_rise_width = (t0_max - t0_min)*exp(mu_rise_width);
  }
  else{
    mu_decay_width = log(mu_decay_width/(t0_max - t0_min));
    mu_decay_width += log(1E3)*pow(10., 1.5 - 6.*rng.rand())*rng.randn();
    mu_decay_width = mod(mu_decay_width - log(1E-3), log(1E3)) + log(1E-3);
    mu_decay_width = (t0_max - t0_min)*exp(mu_decay_width);
  }

  return logH;
}


// function to return the log prior pdf for mu
double FlareDistribution::log_pdf(const std::vector<double>& vec) const
{
  double logp = 0.0;

  Laplace l1(mu_log_amp, scale_log_amp);

  logp += l1.log_pdf(vec[1]);

  // check parameters are within prior ranges
  if(vec[0] < t0_min || vec[0] > t0_max || vec[1] < 0.0 || vec[2] < min_rise_width || vec[3] < min_decay_width){ return -1E300; }

  logp += -log(mu_rise_width)-(vec[2] - min_rise_width)/mu_rise_width-log(mu_decay_width)-(vec[3] - min_decay_width)/mu_decay_width;

  return logp;
}


// function to convert the flare parameters from a unit hypercube into the true values
void FlareDistribution::from_uniform(std::vector<double>& vec) const
{
  Laplace l1(mu_log_amp, scale_log_amp);

  vec[0] = t0_min + (t0_max - t0_min)*vec[0];
  vec[1] = l1.cdf_inverse(vec[1]);
  vec[2] = min_rise_width - mu_rise_width*log(1. - vec[2]);
  vec[3] = min_decay_width - mu_decay_width*log(1. - vec[3]);
}


// function to convert the flare parameters into a uniform unit hypercube
void FlareDistribution::to_uniform(std::vector<double>& vec) const
{
  Laplace l1(mu_log_amp, scale_log_amp);

  vec[0] = (vec[0] - t0_min)/(t0_max - t0_min);
  vec[1] = l1.cdf(vec[1]);
  vec[2] = 1. - exp(-(vec[2] - min_rise_width)/mu_rise_width);
  vec[3] = 1. - exp(-(vec[3] - min_decay_width)/mu_decay_width);
}


void FlareDistribution::print(std::ostream& out) const
{
  out<<mu_log_amp<<' '<<scale_log_amp<<' '<<mu_rise_width<<' '<<mu_decay_width<<' ';
}
