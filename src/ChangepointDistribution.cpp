#include "ChangepointDistribution.h"
#include "RandomNumberGenerator.h"
#include "Utils.h"
#include "Data.h"
#include <cmath>

using namespace DNest3;

ChangepointDistribution::ChangepointDistribution(double tcp_min, double tcp_max)
:tcp_min(tcp_min)
,tcp_max(tcp_max)
{

}


// function to generate prior hyperparameters from their respective priors
void ChangepointDistribution::fromPrior()
{
  mu_back_amp = tan(M_PI*(0.97*randomU() - 0.485)); // generate amplitude prior hyperparameter from Cauchy distribution
  mu_back_amp = exp(mu_back_amp);
}


// function to increment the various parameter prior hyperparameters using their respective proposals
double ChangepointDistribution::perturb_parameters()
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
double ChangepointDistribution::log_pdf(const std::vector<double>& vec) const
{
  // check parameters are within prior ranges
  if(vec[0] < tcp_min || vec[0] > tcp_max || vec[1] < 0.0)
    return -1E300;

  return -log(mu_back_amp) - vec[1]/mu_back_amp;
}


// function to convert the background changepoint parameters from a unit hypercube into the true values
void ChangepointDistribution::from_uniform(std::vector<double>& vec) const
{
  vec[0] = tcp_min + (tcp_max - tcp_min)*vec[0];
  vec[1] = -mu_back_amp*log(1. - vec[1]);
}


// function to convert the background changepoint parameters into a uniform unit hypercube
void ChangepointDistribution::to_uniform(std::vector<double>& vec) const
{
  vec[0] = (vec[0] - tcp_min)/(tcp_max - tcp_min);
  vec[1] = 1. - exp(-vec[1]/mu_back_amp);
}


void ChangepointDistribution::print(std::ostream& out) const
{
  out<<mu_back_amp<<' ';
}
