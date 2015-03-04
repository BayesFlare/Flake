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
  // Get the model components
  const vector< vector<double> >& componentsWave = waves.get_components();
  const vector< vector<double> >& componentsFlare = flares.get_components();

  // Get the data
  const vector<double>& t = Data::get_instance().get_t(); // times
  const vector<double>& y = Data::get_instance().get_y(); // light curve

  mu.assign(mu.size(), 0.);

  double logL = 0.;
  double var = sigma*sigma;
  double P, A, phi;
  double Af, tscale1, tscale2, t0, tscale;
  double dm;
  double lmv = -0.5*log(2.*M_PI*var);
  for(size_t j=0; j<(componentsWave.size()+componentsFlare.size()); j++){
    if ( j < componentsWave.size() ){
      P = exp(componentsWave[j][0]); // sinusoid period
      A = componentsWave[j][1];      // sinusoid amplitude
      phi = componentsWave[j][2];    // sinusoid initial phase
    
      for(size_t i=0; i<y.size(); i++){
        mu[i] += A*sin(2.*M_PI*(t[i]/P) + phi);
        dm = y[i]-mu[i];
        logL += lmv - 0.5*dm*dm/var;
      }
    }
    else{
      Af = componentsFlare[j-componentsWave.size()][1];      // flare amplitude
      t0 = componentsFlare[j-componentsWave.size()][0];      // flare t0
      tscale1 = componentsFlare[j-componentsWave.size()][2]; // flare rise timescale
      tscale2 = componentsFlare[j-componentsWave.size()][3]; // flare rise timescale

      for(size_t i=0; i<y.size(); i++){
        if ( t[i] < t0 ){ tscale = tscale1; }
        else { tscale = tscale2; }

        mu[i] += Af*exp(-abs(t[i] - t0)/tscale);
        dm = y[i]-mu[i];
        logL += lmv - 0.5*dm*dm/var;
      }
    } 
  }

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

