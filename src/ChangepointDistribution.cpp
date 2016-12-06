#include "ChangepointDistribution.h"
#include "RNG.h"
#include "Utils.h"
#include "Data.h"
#include <cmath>
#include <gsl/gsl_cdf.h>

using namespace DNest4;

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
  sigma_back_amp = tan(M_PI*(0.97*rng.rand() - 0.485)); // generate amplitude prior hyperparameter from Cauchy distribution
  sigma_back_amp = exp(sigma_back_amp); 
}


// function to increment the various parameter prior hyperparameters using their respective proposals
double ChangepointDistribution::perturb_hyperparameters(RNG& rng)
{
  double logH = 0.;

  // background change point amplitude prior hyperparameter (sigma for Gaussian prior distribution)
  sigma_back_amp = log(sigma_back_amp);
  sigma_back_amp = (atan(sigma_back_amp)/M_PI + 0.485)/0.97; // Cauchy distribution proposal
  sigma_back_amp += pow(10., 1.5 - 6.*rng.rand())*rng.randn();
  sigma_back_amp = mod(sigma_back_amp, 1.);
  sigma_back_amp = tan(M_PI*(0.97*sigma_back_amp - 0.485));
  sigma_back_amp = exp(sigma_back_amp);

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
