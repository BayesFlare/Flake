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
                        CustomConfigFile::get_instance().get_minWaveMu(),    // minumun of mu (mean of exponetial distribution for amplitudes)
                        CustomConfigFile::get_instance().get_maxWaveMu())),  // maximum of mu
flares(4,
       CustomConfigFile::get_instance().get_maxFlares(),
       false,
       FlareDistribution(CustomConfigFile::get_instance().get_minFlareT0(),          // minimum flare peak time scale
                         CustomConfigFile::get_instance().get_maxFlareT0(),          // maximum flare peak time scale
                         CustomConfigFile::get_instance().get_minFlareRiseWidth(),   // minimum rise width of a flare
                         CustomConfigFile::get_instance().get_minFlareDecayWidth())), // minimum decay width of a flare
changepoint(2,                                                      // number of background change point parameters
           CustomConfigFile::get_instance().get_maxChangepoints(), // max. number of background change points
           false,
           ChangepointDistribution())
{

}


// function to generate the sinusoid, flare and background change point hyperparameters from their distributions
// and the data noise standard devaition and overall background level
void FlareWave::fromPrior()
{
  waves.fromPrior();
  waves.consolidate_diff();

  flares.fromPrior();
  flares.consolidate_diff();

  changepoint.fromPrior();
  changepoint.consolidate_diff();
  
  sigma = exp(log(1E-3) + log(1E6)*randomU());      // generate sigma from prior (uniform in log space between 1e-3 and 1e6)
  background = tan(M_PI*(0.97*randomU() - 0.485));  // generate background from Cauchy prior distribution
  background = exp(background);
}


double FlareWave::perturb()
{
  double logH = 0.;
  double randval = randomU();
  
  if(randval <= 0.2){ // perturb background sinusoids 20% of time
    logH += waves.perturb();
    waves.consolidate_diff();
  }
  else if(randval < 0.6){ // perturb flares 40% of time
    logH += flares.perturb();
    flares.consolidate_diff();
  }
  else if(randval < 0.8){ // perturb noise sigma 20% of time
    sigma = log(sigma);
    sigma += log(1E6)*randh();
    sigma = mod(sigma - log(1E-3), log(1E6)) + log(1E-3);
    sigma = exp(sigma);
  }
  else{ // perturb the overall background offset value 10% of time
    background = log(background);
    background = (atan(background)/M_PI + 0.485)/0.97;
    background += pow(10., 1.5 - 6.*randomU())*randn();
    background = mod(background, 1.);
    background = tan(M_PI*(0.97*background - 0.485));
    background = exp(background);
  }
  else{ // perturb the change point background offset value 10% of time
    logH += changepoint.perturb();
    changepoint.consolidate_diff();
  }
  
  return logH;
}


// the log likelihood function - this function generates the signal model and then
// calculates the the log likelihood function using it:
//  - the sinusoid model is based on the RJObject SineWaves example
//  - the flare model is based on the magnetron code
//    https://bitbucket.org/dhuppenkothen/magnetron/ described in Hupperkothen et al,
//    http://arxiv.org/abs/1501.05251
double FlareWave::logLikelihood() const
{
  // Get the model components
  const vector< vector<double> >& componentsWave = waves.get_components();
  const vector< vector<double> >& componentsFlare = flares.get_components();
  const vector< vector<double> >& componentsChangepoints = changepoints.get_components();
  
  // Get the data
  const vector<double>& t = Data::get_instance().get_t(); // times
  const vector<double>& y = Data::get_instance().get_y(); // light curve

  double var = (sigma*sigma);
  double halfinvvar = 0.5/var;
  double P, A, phi;
  double Af, trise, tdecay, t0, tscale;
  double cpt0, cpback; // change point time and background offset
  double dm;
  double lmv = -0.5*log(2.*M_PI*var);
  double logL = (double)y.size()*lmv;

  std::vector<double> model(Data::get_instance().get_y().size(),background); // allocate model vector
  
  for(size_t j=0; j<(componentsWave.size()+componentsFlare.size()+componentsChangepoints.size()); j++){
    if ( j < componentsChangepoints.size() ){
      cpt0 = componentsChangepoints[j][0];
      cpback = componentsChangepoints[j][1];
      
      for(size_t i=0; i<y.size(); i++){
        if ( t[i] > cpt0 ){
          model[i] += (cpback-background);
        }
      }
    }
    else if ( j < componentsWave.size()+componentsChangepoints.size() ){
      P = exp(componentsWave[j-componentsChangepoints.size()][0]); // sinusoid period
      A = componentsWave[j-componentsChangepoints.size()][1];      // sinusoid amplitude
      phi = componentsWave[j-componentsChangepoints.size()][2];    // sinusoid initial phase
    
      for(size_t i=0; i<y.size(); i++){
        model[i] += A*sin(2.*M_PI*(t[i]/P) + phi);
      }
    }
    else{
      t0 = componentsFlare[j-(componentsWave.size()+componentsChangepoints.size())][0];     // flare t0
      Af = componentsFlare[j-(componentsWave.size()+componentsChangepoints.size())][1];     // flare amplitude
      trise = componentsFlare[j-(componentsWave.size()+componentsChangepoints.size())][2];  // flare rise timescale
      tdecay = componentsFlare[j-(componentsWave.size()+componentsChangepoints.size())][3]; // flare decay timescale

      for(size_t i=0; i<y.size(); i++){
        if ( t[i] < t0 ){ tscale = trise; }
        else { tscale = tdecay; }

        model[i] += Af*exp(-abs(t[i] - t0)/tscale);
      }
    }
  }

  for( size_t i=0; i<y.size(); i++ ){
    dm = y[i]-model[i];
    logL -= dm*dm*halfinvvar;
  }

  return logL;
}


void FlareWave::print(std::ostream& out) const
{
  out<<sigma<<' ';             // output sigma level
  out<<background<<' ';        // output background value
  waves.print(out); out<<' ';  // output sinusoid values
  flares.print(out); out<<' '; // output flare values
}


string FlareWave::description() const
{
  return string("objects, sigma");
}

