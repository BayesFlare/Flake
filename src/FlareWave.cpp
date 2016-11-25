#include "FlareWave.h"
#include "RNG.h"
#include "Utils.h"
#include "Data.h"
#include "CustomConfigFile.h"
#include <cmath>
#include <algorithm>
#include <utility>
#include <iterator>

using namespace std;
using namespace DNest4;

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
                                   Data::get_instance().get_tend())),  // the upper end of allowed change point times is the end of the data
mu(Data::get_instance().get_len()),           // the model vector
muwaves(Data::get_instance().get_len()),      // the sinusoidal models
muflares(Data::get_instance().get_len()),     // the flare models
muimpulse(Data::get_instance().get_len()),    // the impulse models
muchangepoint(Data::get_instance().get_len()),// the background change point models
firstiter(true)
{

}


// function to generate the sinusoid, flare and background change point hyperparameters from their distributions
// and the data noise standard devaition and overall background level
void FlareWave::from_prior(RNG& rng)
{
  waves.from_prior(rng);
  waves.consolidate_diff();

  flares.from_prior(rng);
  flares.consolidate_diff();

  impulse.from_prior(rng);
  impulse.consolidate_diff();
  
  changepoint.from_prior(rng);
  changepoint.consolidate_diff();

  sigma = exp(log(1E-3) + log(1E6)*rng.rand());      // generate sigma from prior (uniform in log space between 1e-3 and 1e6)
  background = tan(M_PI*(0.97*rng.rand() - 0.485));  // generate background from Cauchy prior distribution
  background = exp(background);
  calculate_mu(); // calculate model
}


double FlareWave::perturb(RNG& rng)
{
  double logH = 0.;
  double randval = rng.rand();

  if(randval <= 0.25){ // perturb background sinusoids 25% of time
    logH += waves.perturb(rng);
    waves.consolidate_diff();
  }
  else if(randval < 0.55){ // perturb flares 30% of time
    logH += flares.perturb(rng);
    flares.consolidate_diff();
  }
  else if(randval < 0.75){ // perturb impulses 20% of time
    logH += impulse.perturb(rng);
    impulse.consolidate_diff();
  }
  else if(randval < 0.80){ // perturb the change point background offset value 5% of time
    logH += changepoint.perturb(rng);
    changepoint.consolidate_diff();
  }
  else if(randval < 0.9){ // perturb noise sigma 10% of time
    sigma = log(sigma);
    sigma += log(1E6)*rng.randh();
    sigma = mod(sigma - log(1E-3), log(1E6)) + log(1E-3);
    sigma = exp(sigma);
  }
  else{ // perturb the overall background offset value 10% of time
    background = log(background);
    background = (atan(background)/M_PI + 0.485)/0.97;
    background += pow(10., 1.5 - 6.*rng.rand())*rng.randn();
    background = mod(background, 1.);
    background = tan(M_PI*(0.97*background - 0.485));
    background = exp(background);
  }
  
  // (re-)calculate model in all cases (even when perturbing background or sigma, so that the mu value is assigned)
  calculate_mu();
  
  return logH;
}


// This function generates the model:
//  - the sinusoid model is based on the RJObject SineWaves example
//  - the flare model is based on the magnetron code
//    https://bitbucket.org/dhuppenkothen/magnetron/ described in Hupperkothen et al,
//    http://arxiv.org/abs/1501.05251
void FlareWave::calculate_mu()
{
  // Update or from scratch?
  bool updateWaves = (waves.get_removed().size() == 0);
  bool updateFlares = (flares.get_removed().size() == 0);
  bool updateImpulse = (impulse.get_removed().size() == 0);
  bool updateChangepoint = (changepoint.get_removed().size() == 0);
  
  // Get the model components
  const vector< vector<double> >& componentsWave = (updateWaves)?(waves.get_added()):(waves.get_components());
  const vector< vector<double> >& componentsFlare = (updateFlares)?(flares.get_added()):(flares.get_components());
  const vector< vector<double> >& componentsImpulse = (updateImpulse)?(impulse.get_added()):(impulse.get_components());
  const vector< vector<double> >& componentsChangepoints = changepoint.get_components(); // always re-add all change points if update required (this is different to other model components)

  double freq, A, phi;
  double Af, trise, trise2, tdecay, t0;
  
  // Get the data
  const vector<double>& t = Data::get_instance().get_t(); // times
  
  // compute different components separately and add to main model at the end
  if (!updateWaves){
    muwaves.assign(Data::get_instance().get_len(), 0); // allocate model vector
  }
  if (!updateFlares){
    muflares.assign(Data::get_instance().get_len(), 0); // allocate model vector
  }
  if (!updateImpulse){
    muimpulse.assign(Data::get_instance().get_len(), 0); // allocate model vector
  }

  if (updateChangepoint || firstiter){ // re-add everything if updating (or initialise if start of code)
    muchangepoint.assign(Data::get_instance().get_len(), 0); // allocate model vector
    firstiter = false;
    
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

      int istart = t.size()-1;
      for (size_t k=0; k<componentsChangepoints.size(); k++){
        double thisbackground = componentsChangepoints[cpsorted[k]][1]; // background level for the current change point
        double cpback = thisbackground-background; // change point background offset
        for(int i=istart; i>-1; i--){
          if ( i > componentsChangepoints[cpsorted[k]][0] ){
            muchangepoint[i] += cpback;
          }
          else{
            istart = i;
            break;
          }
        }
      }
    }
  }
  
  // add impulses (single bin transients)
  if ( componentsImpulse.size() > 0 ){
    for ( size_t j=0; j<componentsImpulse.size(); j++ ){
      int impidx = (int)componentsImpulse[j][0]; // impulse time index
      double impamp = componentsImpulse[j][1];   // impulse amplitude
      muimpulse[impidx] = impamp;
    }
  }
  
  // add sinusoids
  if ( componentsWave.size() > 0 ){
    for(size_t j=0; j<componentsWave.size(); j++){
      freq = 2.*M_PI/exp(componentsWave[j][0]); // sinusoid angular frequency (2pi/period)
      A = componentsWave[j][1];                 // sinusoid amplitude
      phi = componentsWave[j][2];               // sinusoid initial phase

      for(size_t i=0; i<t.size(); i++){
        muwaves[i] += A*sin(t[i]*freq + phi);
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
      trise2 = trise*trise; // rise time squared
           
      for(size_t i=0; i<t.size(); i++){
        if ( t[i] < t0 ){ muflares[i] += Af*exp(-0.5*(t[i]-t0)*(t[i]-t0)/trise2); }
        else { muflares[i] += Af*exp(-(t[i] - t0)/tdecay); }
      }
    }
  }

  // combine all models
  for (size_t j=0; j<t.size(); j++){
    mu[j] = background + muflares[j] + muwaves[j] + muimpulse[j] + muchangepoint[j];
  }
}


// the log likelihood function - this function calculates the the log likelihood function:
double FlareWave::log_likelihood() const
{
  const vector<double>& y = Data::get_instance().get_y(); // light curve

  double var = (sigma*sigma);
  double halfinvvar = 0.5/var;
  double dm;
  double lmv = -0.5*log(2.*M_PI*var);
  double logL = (double)y.size()*lmv;

  for( size_t i=0; i<y.size(); i++ ){
    dm = y[i]-mu[i];
    logL -= dm*dm*halfinvvar;
    logL += lmv; // normalisation
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
