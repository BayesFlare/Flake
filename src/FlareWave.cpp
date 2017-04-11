#include "FlareWave.h"
#include "RNG.h"
#include "Utils.h"
#include "Data.h"
#include "CustomConfigFile.h"
#include <cmath>
#include <algorithm>
#include <utility>
#include <iterator>

//#include <stdio.h>
//int counter = 0;

using namespace std;
using namespace DNest4;

const Cauchy FlareWave::cauchy(0., 5.);

// FlareWaves contructor
FlareWave::FlareWave()
:waves(3,                                                   // number of parameters for each sinusoid (amplitude, phase and period)
       CustomConfigFile::get_instance().get_maxSinusoids(), // maximum number of sinusoids
       false,
       WaveDistribution(),
       PriorType::log_uniform),
flares(4,
       CustomConfigFile::get_instance().get_maxFlares(),
       false,
       FlareDistribution(CustomConfigFile::get_instance().get_minFlareT0(),           // minimum flare peak time scale
                         CustomConfigFile::get_instance().get_maxFlareT0(),           // maximum flare peak time scale
                         CustomConfigFile::get_instance().get_minFlareRiseWidth(),    // minimum rise width of a flare
                         CustomConfigFile::get_instance().get_minFlareDecayWidth()),  // minimum decay width of a flare
                         PriorType::log_uniform),
impulse(2,                                                     // number of impulse parameters
        CustomConfigFile::get_instance().get_maxImpulses(),    // max. number of impulses (single bin spikes)
        false,
        ImpulseDistribution(Data::get_instance().get_tstart(), // the lower end of allowed impulse times is the start of the data
                            Data::get_instance().get_tend()),  // the upper end of allowed impulse times is the end of the data
                            PriorType::log_uniform),
changepoint(2,                                                         // number of background change point parameters
           CustomConfigFile::get_instance().get_maxChangepoints(),     // max. number of background change points
           false,
           ChangepointDistribution(Data::get_instance().get_tstart(),  // the lower end of allowed change point times is the start of the data
                                   Data::get_instance().get_tend()),   // the upper end of allowed change point times is the end of the data
                                   PriorType::log_uniform),
mu(Data::get_instance().get_len()),           // the model vector
muwaves(Data::get_instance().get_len()),      // the sinusoidal models
muflares(Data::get_instance().get_len()),     // the flare models
muimpulse(Data::get_instance().get_len()),    // the impulse models
muchangepoint(Data::get_instance().get_len()),// the background change point models
firstiter(true),
background(0.0), //initialise background to zero
wavesweight(25.),
flaresweight(20.),
impulseweight(20.),
cpweight(5.),
wavesfrac(0.0),
flaresfrac(0.0),
impulsefrac(0.0),
cpfrac(0.0)
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

  log_sigma = cauchy.generate(rng); // generate log sigma from prior (Cauchy distribution)
  sigma = exp(log_sigma);

  // use a naive diffuse (sigme = 1e3) Gaussian prior for the background
  background = 1e3*rng.randn();

  calculate_mu(); // calculate model
}


double FlareWave::perturb(RNG& rng)
{
  double logH = 0.;
  double randval = rng.rand();
  bool updateWaves = false, updateFlares = false, updateImpulse = false, updateChangepoint = false;
  
  // set fraction of time for each model perturbation
  double totweight = 0.;
  double modelsfrac = 0.8;

  if ( !wavesfrac && !flaresfrac && !impulsefrac && !cpfrac ){
    if ( CustomConfigFile::get_instance().get_maxSinusoids() > 0 ){
      wavesfrac = wavesweight;
      totweight += wavesfrac;
    }
    if ( CustomConfigFile::get_instance().get_maxFlares() > 0 ){
      flaresfrac = flaresweight;
      totweight += flaresfrac;
    }
    if ( CustomConfigFile::get_instance().get_maxImpulses() > 0 ){
      impulsefrac = impulseweight;
      totweight += impulsefrac;
    }
    if ( CustomConfigFile::get_instance().get_maxChangepoints() > 0 ){
      cpfrac = cpweight;
      totweight += cpfrac;
    }
    
    wavesfrac /= totweight;
    flaresfrac /= totweight;
    flaresfrac += wavesfrac;
    impulsefrac /= totweight;
    impulsefrac += flaresfrac;
    cpfrac /= totweight;
    cpfrac += impulsefrac;
  }
  
  if ( randval < modelsfrac ){ 
    if(randval <= wavesfrac*modelsfrac){ // perturb background sinusoids 25% of time
      logH += waves.perturb(rng);
      waves.consolidate_diff();
      updateWaves = (waves.get_removed().size() == 0);
    }
    else if(randval < flaresfrac*modelsfrac){ // perturb flares 30% of time
      logH += flares.perturb(rng);
      flares.consolidate_diff();
      updateFlares = (flares.get_removed().size() == 0);
    }
    else if(randval < impulsefrac*modelsfrac){ // perturb impulses 20% of time
      logH += impulse.perturb(rng);
      impulse.consolidate_diff();
      updateImpulse = (impulse.get_removed().size() == 0);
    }
    else if(randval < cpfrac*modelsfrac){ // perturb the change point background offset value 5% of time
      logH += changepoint.perturb(rng);
      changepoint.consolidate_diff();
      updateChangepoint = (changepoint.get_removed().size() == 0);
    }
    calculate_mu(updateWaves, updateFlares, updateImpulse, updateChangepoint);
  }
  else if(randval < 0.9){ // perturb noise sigma 10% of time (no need to re-calculate_mu)
    logH += cauchy.perturb(log_sigma, rng);
    sigma = exp(log_sigma);
  }
  else{ // perturb the overall background offset value 10% of time
    logH -= -0.5*pow(background/1e3, 2);
    background += 1e3*rng.randh(); // see e.g. https://github.com/eggplantbren/DNest4/blob/master/code/Examples/StraightLine/StraightLine.cpp
    logH += -0.5*pow(background/1e3, 2);
    calculate_mu();
  }

  return logH;
}


// This function generates the model:
//  - the sinusoid model is based on the RJObject SineWaves example
//  - the flare model is based on the magnetron code
//    https://bitbucket.org/dhuppenkothen/magnetron/ described in Hupperkothen et al,
//    http://arxiv.org/abs/1501.05251
void FlareWave::calculate_mu(bool updateWaves, bool updateFlares, bool updateImpulse, bool updateChangepoint)
{
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
    muwaves.assign(Data::get_instance().get_len(), 0.); // allocate model vector
  }
  if (!updateFlares){
    muflares.assign(Data::get_instance().get_len(), 0.); // allocate model vector
  }
  if (!updateImpulse){
    muimpulse.assign(Data::get_instance().get_len(), 0.); // allocate model vector
  }

  if (updateChangepoint || firstiter){ // re-add everything if updating (or initialise if start of code)
    muchangepoint.assign(Data::get_instance().get_len(), 0.); // allocate model vector
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
      int impidx = (int)componentsImpulse[j][0];    // impulse time index
      double impamp = exp(componentsImpulse[j][1]); // impulse amplitude
      muimpulse[impidx] = impamp;
    }
  }
  
  // add sinusoids
  if ( componentsWave.size() > 0 ){
#ifdef USE_SSE2
    float wphases[t.size()];  // phase vector
    float thiswave[t.size()]; // wave vector   
#endif
    for(size_t j=0; j<componentsWave.size(); j++){
      freq = 2.*M_PI/exp(componentsWave[j][0]); // sinusoid angular frequency (2pi/period)
      A = exp(componentsWave[j][1]);            // sinusoid amplitude
      phi = componentsWave[j][2];               // sinusoid initial phase

#ifdef USE_SSE2
      for(size_t i=0; i<t.size(); i++){ wphases[i] = t[i]*freq + phi; }
      VectorMathSin( thiswave, wphases, (unsigned int)t.size() );
      VectorMathScale( thiswave, (float)A, thiswave, (unsigned int)t.size() );
      VectorMathAdd( &muwaves[0], &muwaves[0], thiswave, (unsigned int)t.size() );
#else
      for(size_t i=0; i<t.size(); i++){
        muwaves[i] += A*sin(t[i]*freq + phi);
      }
#endif
    }
  }

  // add flares
  if ( componentsFlare.size() > 0 ){
#ifdef USE_SSE2
    float flaretimes[t.size()];
    float thisflare[t.size()];
#endif
    for(size_t j=0; j<componentsFlare.size(); j++){
      t0 = componentsFlare[j][0];      // flare t0
      Af = exp(componentsFlare[j][1]); // flare amplitude
      trise = componentsFlare[j][2];   // flare rise timescale
      tdecay = componentsFlare[j][3];  // flare decay timescale
      trise2 = trise*trise;            // rise time squared
      
#ifdef USE_SSE2
      for(size_t i=0; i<t.size(); i++){
        if ( t[i] < t0 ) { flaretimes[i] = -0.5*(t[i]-t0)*(t[i]-t0)/trise2; }
        else { flaretimes[i] = -(t[i] - t0)/tdecay; }
      }
      VectorMathExp( thisflare, flaretimes, (unsigned int)t.size() );
      VectorMathScale( thisflare, (float)Af, thisflare, (unsigned int)t.size() );
      VectorMathAdd( &muflares[0], &muflares[0], thisflare, (unsigned int)t.size() );
#else
      for(size_t i=0; i<t.size(); i++){
        if ( t[i] < t0 ){ muflares[i] += Af*exp(-0.5*(t[i]-t0)*(t[i]-t0)/trise2); }
        else { muflares[i] += Af*exp(-(t[i] - t0)/tdecay); }
      }
#endif
    }
  }

  // combine all models
  //FILE *fp = NULL;
  //if ( counter == 100000 ){
  //  fp = fopen("model.txt", "w");
  //}
  for (size_t j=0; j<t.size(); j++){
    mu[j] = background + muflares[j] + muwaves[j] + muimpulse[j] + muchangepoint[j];
    //if ( counter == 100000 ){
    //  fprintf(fp, "%.8lf\n", mu[j]);
    //}
  }
  //if ( counter == 100000 ){
  //  fclose(fp);
  //  exit(0);
  //}
  
  counter++;
}


// the log likelihood function - this function calculates the the log likelihood function:
double FlareWave::log_likelihood() const
{
  const vector<double>& y = Data::get_instance().get_y(); // light curve

  double var = (sigma*sigma);
  double halfinvvar = 0.5/var;
  double dm;
  double lmv = -0.5*log(2.*M_PI*var);
  double logL = (double)y.size()*lmv; // normalisation

  for( size_t i=0; i<y.size(); i++ ){
    dm = y[i]-mu[i];
    logL -= dm*dm*halfinvvar;
  }

  logL += lmv;

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
