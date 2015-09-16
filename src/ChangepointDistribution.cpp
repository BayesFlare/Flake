#include "BackgroundDistribution.h"
#include "RandomNumberGenerator.h"
#include "Utils.h"
#include "Data.h"
#include <cmath>

using namespace DNest3;

BackgroundDistribution::BackgroundDistribution()
{

}


// function to generate prior hyperparameters from their respective priors
void BackgroundDistribution::fromPrior()
{
  mu_back_amp = tan(M_PI*(0.97*randomU() - 0.485)); // generate amplitude prior hyperparameter from Cauchy distribution
  mu_back_amp = exp(mu_back_amp);
}


// function to increment the various parameter prior hyperparameters using their respective proposals
double BackgroundDistribution::perturb_parameters()
{
  double logH = 0.;

  // background amplitude prior hyperparameter
  mu_back_amp = log(mu_back_amp);
  mu_back_amp = (atan(mu_back_amp)/M_PI + 0.485)/0.97; // Cauchy distribution proposal
  mu_back_amp += pow(10., 1.5 - 6.*randomU())*randn();
  mu_back_amp = mod(mu_back_amp, 1.);
  mu_back_amp = tan(M_PI*(0.97*mu_back_amp - 0.485));
  mu_back_amp = exp(mu_back_amp);

  return logH;
}


// function to return the log prior pdf for mu
double FlareDistribution::log_pdf(const std::vector<double>& vec) const
{
  // check parameters are within prior ranges
  if(vec[0] < t0_min || vec[0] > t0_max || vec[1] < 0.0 || vec[2] < min_rise_width || vec[3] < min_decay_width)
    return -1E300;

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