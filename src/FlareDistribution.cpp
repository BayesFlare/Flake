#include "FlareDistribution.h"
#include "RNG.h"
#include "Utils.h"
#include "Data.h"
#include <cmath>

using namespace DNest4;

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
  mu_amp = tan(M_PI*(0.97*rng.rand() - 0.485)); // generate amplitude prior hyperparameter from Cauchy distribution
  mu_amp = exp(mu_amp);
  mu_rise_width = exp(log(1E-3*(t0_max - t0_min)) + log(1E3)*rng.rand());  // generate rise time hyperparameter from log uniform distribution
  mu_decay_width = exp(log(1E-3*(t0_max - t0_min)) + log(1E3)*rng.rand()); // generate decay time hyperparameter from log uniform distribution
}


// function to increment the various parameter prior hyperparameters using their respective proposals
double FlareDistribution::perturb_hyperparameters(RNG& rng)
{
  double logH = 0.;

  int which = rng.rand_int(3); // set equal probability for incrementing each hyperparameter

  if(which == 0){
    // flare amplitude prior hyperparameter
    mu_amp = log(mu_amp);
    mu_amp = (atan(mu_amp)/M_PI + 0.485)/0.97; // Cauchy distribution proposal
    mu_amp += pow(10., 1.5 - 6.*rng.rand())*rng.randn();
    mu_amp = mod(mu_amp, 1.);
    mu_amp = tan(M_PI*(0.97*mu_amp - 0.485));
    mu_amp = exp(mu_amp);
  }
  if(which == 1){
    mu_rise_width = log(mu_rise_width/(t0_max - t0_min));
    mu_rise_width += log(1E3)*pow(10., 1.5 - 6.*rng.rand())*rng.randn();
    mu_rise_width = mod(mu_rise_width - log(1E-3), log(1E3)) + log(1E-3);
    mu_rise_width = (t0_max - t0_min)*exp(mu_rise_width);
  }
  if(which == 2){
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
  // check parameters are within prior ranges
  if(vec[0] < t0_min || vec[0] > t0_max || vec[1] < 0.0 || vec[2] < min_rise_width || vec[3] < min_decay_width){ return -1E300; }

  return -log(mu_amp) - vec[1]/mu_amp - log(mu_rise_width) - (vec[2] - min_rise_width)/mu_rise_width - log(mu_decay_width) - (vec[3] - min_decay_width)/mu_decay_width;
}


// function to convert the flare parameters from a unit hypercube into the true values
void FlareDistribution::from_uniform(std::vector<double>& vec) const
{
  vec[0] = t0_min + (t0_max - t0_min)*vec[0];
  vec[1] = -mu_amp*log(1. - vec[1]);
  vec[2] = min_rise_width - mu_rise_width*log(1. - vec[2]);
  vec[3] = min_decay_width - mu_decay_width*log(1. - vec[3]);
}


// function to convert the flare parameters into a uniform unit hypercube
void FlareDistribution::to_uniform(std::vector<double>& vec) const
{
  vec[0] = (vec[0] - t0_min)/(t0_max - t0_min);
  vec[1] = 1. - exp(-vec[1]/mu_amp);
  vec[2] = 1. - exp(-(vec[2] - min_rise_width)/mu_rise_width);
  vec[3] = 1. - exp(-(vec[3] - min_decay_width)/mu_decay_width);
}


void FlareDistribution::print(std::ostream& out) const
{
  out<<mu_amp<<' '<<mu_rise_width<<' '<<mu_decay_width<<' ';
}
