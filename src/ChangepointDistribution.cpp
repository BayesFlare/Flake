#include "ChangepointDistribution.h"
#include "RNG.h"
#include "Utils.h"
#include "Data.h"
#include <cmath>
#include <gsl/gsl_cdf.h>

using namespace DNest4;

const DNest4::Cauchy ChangepointDistribution::cauchy(0.0, 5.0);

ChangepointDistribution::ChangepointDistribution(double tcp_min, double tcp_max)
:tcp_min(tcp_min)
,tcp_max(tcp_max)
{

}

// function to convert a unit range to a Gaussian
double unit_to_gaussian(double r, double meanv, double sigmav)
{
  return gsl_cdf_gaussian_Pinv(r, sigmav) + meanv;
}

// function to generate prior hyperparameters from their respective priors
void ChangepointDistribution::from_prior(RNG& rng)
{
  log_sigma_back_amp = cauchy.generate(rng); // generate log sigma from prior (Cauchy distribution)
  sigma_back_amp = exp(log_sigma_back_amp);
}


// function to increment the various parameter prior hyperparameters using their respective proposals
double ChangepointDistribution::perturb_hyperparameters(RNG& rng)
{
  double logH = 0.;

  // background change point amplitude prior hyperparameter (sigma for Gaussian prior distribution)
  logH += cauchy.perturb(log_sigma_back_amp, rng);
  sigma_back_amp = exp(log_sigma_back_amp);

  return logH;
}


// function to return the log prior pdf for mu
double ChangepointDistribution::log_pdf(const std::vector<double>& vec) const
{

  // check parameters are within prior ranges
  if (vec[0]*Data::get_instance().get_dt() + Data::get_instance().get_tstart() < tcp_min ||
      vec[0]*Data::get_instance().get_dt() + Data::get_instance().get_tstart() > tcp_max) { return -1E300; }

  return -0.5*pow(vec[1]/sigma_back_amp, 2);
}


// function to convert the background changepoint parameters from a unit hypercube into the true values
void ChangepointDistribution::from_uniform(std::vector<double>& vec) const
{
  // change point must be on a time bin so return a bin index as vec[0]
  vec[0] = floor((double)(Data::get_instance().get_len()-1)*vec[0]);
  vec[1] = unit_to_gaussian(vec[1], 0., sigma_back_amp);
}


// function to convert the background changepoint parameters into a uniform unit hypercube
void ChangepointDistribution::to_uniform(std::vector<double>& vec) const
{
  // vec[0] is a bin index rather than a time, so get the actual time
  vec[0] = (Data::get_instance().get_t()[(size_t)vec[0]] - tcp_min)/(tcp_max - tcp_min);
  vec[1] = gsl_cdf_gaussian_P(vec[1], sigma_back_amp);
}


void ChangepointDistribution::print(std::ostream& out) const
{
  out<<sigma_back_amp<<' ';
}
