#include "FlareWave.h"
#include "RandomNumberGenerator.h"
#include "Utils.h"
#include "Data.h"
#include "CustomConfigFile.h"
#include <cmath>
#include <algorithm>
#include <utility>
#include <iterator>

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
       FlareDistribution(CustomConfigFile::get_instance().get_minFlareT0(),           // minimum flare peak time scale
                         CustomConfigFile::get_instance().get_maxFlareT0(),           // maximum flare peak time scale
                         CustomConfigFile::get_instance().get_minFlareRiseWidth(),    // minimum rise width of a flare
                         CustomConfigFile::get_instance().get_minFlareDecayWidth())), // minimum decay width of a flare
impulse(2,                                                     // number of impulse parameters
        CustomConfigFile::get_instance().get_maxImpulses(),    // max. number of impulses (single bin spikes)
        false,
        ImpulseDistribution(Data::get_instance().get_tstart(), // the lower end of allowed impulse times is the start of the data
                            Data::get_instance().get_tend())), // the upper end of allowed impulse times is the end of the data  
changepoint(2,                                                         // number of background change point parameters
           CustomConfigFile::get_instance().get_maxChangepoints(),     // max. number of background change points
           false,
           ChangepointDistribution(Data::get_instance().get_tstart(),  // the lower end of allowed change point times is the start of the data
                                   Data::get_instance().get_tend()))   // the upper end of allowed change point times is the end of the data
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

  impulse.fromPrior();
  impulse.consolidate_diff();
  
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
  else if(randval < 0.4){ // perturb flares 20% of time
    logH += flares.perturb();
    flares.consolidate_diff();
  }
  else if(randval < 0.6){ // perturb impulses 20% of time
    logH += impulse.perturb();
    impulse.consolidate_diff();
  }
  else if(randval < 0.7){ // perturb the change point background offset value 10% of time
    logH += changepoint.perturb();
    changepoint.consolidate_diff();
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
  const vector< vector<double> >& componentsImpulse = impulse.get_components();
  const vector< vector<double> >& componentsChangepoints = changepoint.get_components();

  // Get the data
  const vector<double>& t = Data::get_instance().get_t(); // times
  const vector<double>& y = Data::get_instance().get_y(); // light curve

  double var = (sigma*sigma);
  double halfinvvar = 0.5/var;
  double freq, A, phi;
  double Af, trise, tdecay, t0, tscale;
  double dm;
  double lmv = -0.5*log(2.*M_PI*var);
  double logL = (double)y.size()*lmv;

  vector<double> model(Data::get_instance().get_len(),background); // allocate model vector

  // add background change points
  if ( componentsChangepoints.size() > 0 ){
    vector<int> cpcopy(componentsChangepoints.size());
    vector<int> cpsorted(componentsChangepoints.size());

    for (size_t k=0; k<componentsChangepoints.size(); k++){
      cpcopy[k] = (int)componentsChangepoints[k][0];
    }

    // sort indices (last one first)
    for (size_t k=0; k<componentsChangepoints.size(); k++){
      int thisidx = distance(cpcopy.begin(), max_element(cpcopy.begin(), cpcopy.end())); // find position of max value
      cpsorted[k] = thisidx;
      cpcopy[thisidx] = -1; // set element to negative number (so other values will always be bigger)
    }

    int istart = y.size()-1;
    for (size_t k=0; k<componentsChangepoints.size(); k++){
      double thisbackground = componentsChangepoints[cpsorted[k]][1]; // background level for the current change point
      double cpback = thisbackground-background; // change point background offset
      for(int i=istart; i>-1; i--){
        if ( i > componentsChangepoints[cpsorted[k]][0] ){
          model[i] += cpback;
        }
        else{
          istart = i;
          break;
        }
      }
    }
  }

  // add impulses (single bin transients)
  if ( componentsImpulse.size() > 0 ){
    for ( size_t j=0; j<componentsImpulse.size(); j++ ){
      int impidx = (int)componentsImpulse[j][0]; // impulse time index
      double impamp = componentsImpulse[j][1];   // impulse amplitude
      model[impidx] = impamp;
    }
  }

  // add sinusoids
  if ( componentsWave.size() > 0 ){
    for(size_t j=0; j<componentsWave.size(); j++){
      freq = 2.*M_PI/exp(componentsWave[j][0]); // sinusoid angular frequency (2pi/period)
      A = componentsWave[j][1];                 // sinusoid amplitude
      phi = componentsWave[j][2];               // sinusoid initial phase

      for(size_t i=0; i<y.size(); i++){
        model[i] += A*sin(t[i]*freq + phi);
      }
    }
  }

  // add flares
  if ( componentsFlare.size() > 0 ){
    for(size_t j=0; j<componentsFlare.size(); j++){
      t0 = componentsFlare[j][0];     // flare t0
      Af = componentsFlare[j][1];     // flare amplitude
      trise = componentsFlare[j][2];  // flare rise timescale
      tdecay = componentsFlare[j][3]; // flare decay timescale

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
  out<<sigma<<' ';                  // output sigma level
  out<<background<<' ';             // output background value
  changepoint.print(out); out<<' '; // output background change point values
  waves.print(out); out<<' ';       // output sinusoid values
  flares.print(out); out<<' ';      // output flare values
  impulse.print(out); out<<' ';     // output impulse values
}


string FlareWave::description() const
{
  return string("objects, sigma");
}
