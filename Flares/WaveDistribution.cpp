#include "WaveDistribution.h"
#include "RandomNumberGenerator.h"
#include "Utils.h"
#include <cmath>

using namespace DNest3;

WaveDistribution::WaveDistribution(double logP_min, double logP_max, double mu_min, double mu_max)
:logP_min(logP_min)
,logP_max(logP_max)
,mu_min(mu_min)
,mu_max(mu_max)
{

}

void WaveDistribution::fromPrior()
{
	mu = exp(log(mu_min) + log(mu_max/mu_min)*randomU());
}

double WaveDistribution::perturb_parameters()
{
	double logH = 0.;

	mu = log(mu);
	mu += log(mu_max/mu_min)*pow(10., 1.5 - 6.*randomU())*randn();
	mu = mod(mu - log(mu_min), log(mu_max/mu_min)) + log(mu_min);
	mu = exp(mu);

	return logH;
}

// vec[0] = log-period
// vec[1] = amplitude
// vec[2] = phase

double WaveDistribution::log_pdf(const std::vector<double>& vec) const
{
	if(vec[0] < logP_min || vec[0] > logP_max || vec[1] < 0. ||
			vec[2] < 0. || vec[2] > 2.*M_PI)
		return -1E300;

	return -log(mu) - vec[1]/mu;
}

void WaveDistribution::from_uniform(std::vector<double>& vec) const
{
	vec[0] = logP_min + (logP_max - logP_min)*vec[0];
	vec[1] = -mu*log(1. - vec[1]);
	vec[2] = 2.*M_PI*vec[2];
}

void WaveDistribution::to_uniform(std::vector<double>& vec) const
{
	vec[0] = (vec[0] - logP_min)/(logP_max - logP_min);
	vec[1] = 1. - exp(-vec[1]/mu);
	vec[2] = vec[2]/(2.*M_PI);
}

void WaveDistribution::print(std::ostream& out) const
{
	out<<mu<<' ';
}

