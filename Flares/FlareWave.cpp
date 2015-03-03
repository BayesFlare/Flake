#include "FlareWave.h"
#include "RandomNumberGenerator.h"
#include "Utils.h"
#include "Data.h"
#include "CustomConfigFile.h"
#include <cmath>

using namespace std;
using namespace DNest3;

// FlareWaves contructor
FlareWave::FlareWave()
:waves(3,                                                   // number of parameters for each sinusoid (amplitude, phase and period)
       CustomConfigFile::get_instance().get_maxSinusoids(), // maximum number of sinusoids
       false,
       WaveDistribution(CustomConfigFile::get_instance().get_minLogPeriod(), // minimum log period for sinusoids
                        CustomConfigFile::get_instance().get_maxLogPeriod(), // maximum log period for sinusoids
                        CustomConfigFile::get_instance().get_minWaveMu(), // minumun of mu (mean of exponetial distribution for amplitudes)
                        CustomConfigFile::get_instance().get_maxWaveMu())) // maximum of mu
,mu(Data::get_instance().get_t().size())                    // initialise the model vector
{

}

void FlareWave::fromPrior()
{
	waves.fromPrior();
	waves.consolidate_diff();
	sigma = exp(log(1E-3) + log(1E6)*randomU());
	calculate_mu();
}

void FlareWave::calculate_mu()
{
	// Get the times from the data
	const vector<double>& t = Data::get_instance().get_t();

	// Update or from scratch?
	bool update = (waves.get_added().size() < waves.get_components().size());

	// Get the components
	const vector< vector<double> >& components = (update)?(waves.get_added()):
				(waves.get_components());

	// Zero the signal
	if(!update)
		mu.assign(mu.size(), 0.);

	double T, A, phi;
	for(size_t j=0; j<components.size(); j++)
	{
		T = exp(components[j][0]);
		A = components[j][1];
		phi = components[j][2];
		for(size_t i=0; i<t.size(); i++)
			mu[i] += A*sin(2.*M_PI*t[i]/T + phi);
	}
}

double FlareWave::perturb()
{
	double logH = 0.;

	if(randomU() <= 0.75)
	{
		logH += waves.perturb();
		waves.consolidate_diff();
		calculate_mu();
	}
	else
	{
		sigma = log(sigma);
		sigma += log(1E6)*randh();
		sigma = mod(sigma - log(1E-3), log(1E6)) + log(1E-3);
		sigma = exp(sigma);
	}

	return logH;
}

double FlareWave::logLikelihood() const
{
	// Get the data
	const vector<double>& y = Data::get_instance().get_y();

	double logL = 0.;
	double var = sigma*sigma;
	for(size_t i=0; i<y.size(); i++)
		logL += -0.5*log(2.*M_PI*var) - 0.5*pow(y[i] - mu[i], 2)/var;

	return logL;
}

void FlareWave::print(std::ostream& out) const
{
	for(size_t i=0; i<mu.size(); i++)
		out<<mu[i]<<' ';
	out<<sigma<<' ';
	waves.print(out); out<<' ';
}

string FlareWave::description() const
{
	return string("objects, sigma");
}

