#include "FlareDistribution.h"
#include "RandomNumberGenerator.h"
#include "Utils.h"
#include "Data.h"
#include <cmath>

using namespace DNest3;

FlareDistribution::FlareDistribution(double t0_min, double t0_max, double mu_min, double mu_max)
:t0_min(t0_min)
,t0_max(t0_max)
,mu_min(mu_min)
,mu_max(mu_max)
,min_width(0.3333*Data::get_instance().get_dt())
{

}

void FlareDistribution::fromPrior()
{
	mu = tan(M_PI*(0.97*randomU() - 0.485)); // generate amplitude prior hyperparameter from Cauchy distribution
	mu = exp(mu);
	mu_widths = exp(log(1E-3*(t0_max - t0_min)) + log(1E3)*randomU()); // generate rise time hyperparameter from log uniform distribution

	a = -10. + 20.*randomU();
	b = 2.*randomU();
}

double FlareDistribution::perturb_parameters()
{
	double logH = 0.;

	int which = randInt(4);

	if(which == 0)
	{
		mu = log(mu);
		mu = (atan(mu)/M_PI + 0.485)/0.97;
		mu += pow(10., 1.5 - 6.*randomU())*randn();
		mu = mod(mu, 1.);
		mu = tan(M_PI*(0.97*mu - 0.485));
		mu = exp(mu);
	}
	if(which == 1)
	{
		mu_widths = log(mu_widths/(t0_max - t0_min));
		mu_widths += log(1E3)*pow(10., 1.5 - 6.*randomU())*randn();
		mu_widths = mod(mu_widths - log(1E-3), log(1E3)) + log(1E-3);
		mu_widths = (t0_max - t0_min)*exp(mu_widths);
	}
	if(which == 2)
	{
		a += 20.*randh();
		a = mod(a + 10., 20.) - 10.;
	}
	if(which == 3)
	{
		b += 2.*randh();
		b = mod(b, 2.);
	}

	return logH;
}

double FlareDistribution::log_pdf(const std::vector<double>& vec) const
{
	if(vec[0] < t0_min || vec[0] > t0_max || vec[1] < 0.0 || vec[2] < min_width
		|| log(vec[3]) < (a-b) || log(vec[3]) > (a + b))
		return -1E300;

	return -log(mu) - vec[1]/mu - log(mu_widths)
			- (vec[2] - min_width)/mu_widths - log(2.*b*vec[3]);
}

void FlareDistribution::from_uniform(std::vector<double>& vec) const
{
	vec[0] = t0_min + (t0_max - t0_min)*vec[0];
	vec[1] = -mu*log(1. - vec[1]);
	vec[2] = min_width - mu_widths*log(1. - vec[2]);
	vec[3] = exp(a - b + 2.*b*vec[3]);
}

void FlareDistribution::to_uniform(std::vector<double>& vec) const
{
	vec[0] = (vec[0] - t0_min)/(t0_max - t0_min);
	vec[1] = 1. - exp(-vec[1]/mu);
	vec[2] = 1. - exp(-(vec[2] - min_width)/mu_widths);
	vec[3] = (log(vec[3]) + b - a)/(2.*b);
}

void FlareDistribution::print(std::ostream& out) const
{
	out<<mu<<' '<<mu_widths<<' '<<a<<' '<<b<<' ';
}

