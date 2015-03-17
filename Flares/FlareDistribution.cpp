#include "FlareDistribution.h"
#include "RandomNumberGenerator.h"
#include "Utils.h"
#include "Data.h"
#include <cmath>

using namespace DNest3;

FlareDistribution::FlareDistribution(double t0_min, double t0_max)
:t0_min(t0_min)
,t0_max(t0_max)
,min_width(0.3333*Data::get_instance().get_dt())
{

}


// function to generate prior hyperparameters from their respective priors
void FlareDistribution::fromPrior()
{
  mu_amp = tan(M_PI*(0.97*randomU() - 0.485)); // generate amplitude prior hyperparameter from Cauchy distribution
  mu_amp = exp(mu_amp);
  mu_widths = exp(log(1E-3*(t0_max - t0_min)) + log(1E3)*randomU()); // generate rise time hyperparameter from log uniform distribution

  a = -10. + 20.*randomU();
  b = 2.*randomU();
}


// function to increment the various parameter prior hyperparameters using their respective proposals
double FlareDistribution::perturb_parameters()
{
  double logH = 0.;

  int which = randInt(4); // set equal probability for incrementing each hyperparameter

  if(which == 0){
    // flare amplitude prior hyperparameter
    mu_amp = log(mu_amp);
    mu_amp = (atan(mu_amp)/M_PI + 0.485)/0.97; // Cauchy distribution proposal
    mu_amp += pow(10., 1.5 - 6.*randomU())*randn();
    mu_amp = mod(mu_amp, 1.);
    mu_amp = tan(M_PI*(0.97*mu_amp - 0.485));
    mu_amp = exp(mu_amp);
  }
  if(which == 1){
    mu_widths = log(mu_widths/(t0_max - t0_min));
    mu_widths += log(1E3)*pow(10., 1.5 - 6.*randomU())*randn();
    mu_widths = mod(mu_widths - log(1E-3), log(1E3)) + log(1E-3);
    mu_widths = (t0_max - t0_min)*exp(mu_widths);
  }
  if(which == 2){
    a += 20.*randh();
    a = mod(a + 10., 20.) - 10.;
  }
  if(which == 3){
    b += 2.*randh();
    b = mod(b, 2.);
  }

  return logH;
}


// function to return the log prior pdf for mu
double FlareDistribution::log_pdf(const std::vector<double>& vec) const
{
  // check parameters are within prior ranges
  if(vec[0] < t0_min || vec[0] > t0_max || vec[1] < 0.0 || vec[2] < min_width || log(vec[3]) < (a-b) || log(vec[3]) > (a + b))
    return -1E300;

  return -log(mu_amp) - vec[1]/mu_amp - log(mu_widths) - (vec[2] - min_width)/mu_widths - log(2.*b*vec[3]);
}


// function to convert the flare parameters from a unit hypercube into the true values
void FlareDistribution::from_uniform(std::vector<double>& vec) const
{
  vec[0] = t0_min + (t0_max - t0_min)*vec[0];
  vec[1] = -mu_amp*log(1. - vec[1]);
  vec[2] = min_width - mu_widths*log(1. - vec[2]);
  vec[3] = exp(a - b + 2.*b*vec[3]);
}


// function to convert the flare parameters into a uniform unit hypercube
void FlareDistribution::to_uniform(std::vector<double>& vec) const
{
  vec[0] = (vec[0] - t0_min)/(t0_max - t0_min);
  vec[1] = 1. - exp(-vec[1]/mu_amp);
  vec[2] = 1. - exp(-(vec[2] - min_width)/mu_widths);
  vec[3] = (log(vec[3]) + b - a)/(2.*b);
}


void FlareDistribution::print(std::ostream& out) const
{
  out<<mu_amp<<' '<<mu_widths<<' '<<a<<' '<<b<<' ';
}
