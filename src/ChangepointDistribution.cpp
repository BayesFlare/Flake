#include "ChangepointDistribution.h"
#include "RNG.h"
#include "Utils.h"
#include "Data.h"
#include <cmath>

using namespace DNest4;

ChangepointDistribution::ChangepointDistribution(double tcp_min, double tcp_max)
:tcp_min(tcp_min)
,tcp_max(tcp_max)
{

}


// function to generate prior hyperparameters from their respective priors
void ChangepointDistribution::from_prior(RNG& rng)
{
  mu_back_amp = tan(M_PI*(0.97*rng.rand() - 0.485)); // generate amplitude prior hyperparameter from Cauchy distribution
  mu_back_amp = exp(mu_back_amp);
}


// function to increment the various parameter prior hyperparameters using their respective proposals
double ChangepointDistribution::perturb_hyperparameters(RNG& rng)
{
  double logH = 0.;

  // background amplitude prior hyperparameter
  mu_back_amp = log(mu_back_amp);
  mu_back_amp = (atan(mu_back_amp)/M_PI + 0.485)/0.97; // Cauchy distribution proposal
  mu_back_amp += pow(10., 1.5 - 6.*rng.rand())*rng.randn();
  mu_back_amp = mod(mu_back_amp, 1.);
  mu_back_amp = tan(M_PI*(0.97*mu_back_amp - 0.485));
  mu_back_amp = exp(mu_back_amp);

  return logH;
}


// function to return the log prior pdf for mu
double ChangepointDistribution::log_pdf(const std::vector<double>& vec) const
{
  // check parameters are within prior ranges
  if (vec[1] < 0.0) { return -1E300; }

  return -log(mu_back_amp) - vec[1]/mu_back_amp;
}


// function to convert the background changepoint parameters from a unit hypercube into the true values
void ChangepointDistribution::from_uniform(std::vector<double>& vec) const
{
  // change point must be on a time bin so return a bin index as vec[0]
  vec[0] = floor((double)(Data::get_instance().get_len()-1)*vec[0]);
  vec[1] = -mu_back_amp*log(1. - vec[1]);
}


// function to convert the background changepoint parameters into a uniform unit hypercube
void ChangepointDistribution::to_uniform(std::vector<double>& vec) const
{
  // vec[0] is a bin index rather than a time, so get the actual time
  vec[0] = (Data::get_instance().get_t()[(size_t)vec[0]] - tcp_min)/(tcp_max - tcp_min);
  vec[1] = 1. - exp(-vec[1]/mu_back_amp);
}


void ChangepointDistribution::print(std::ostream& out) const
{
  out<<mu_back_amp<<' ';
}
