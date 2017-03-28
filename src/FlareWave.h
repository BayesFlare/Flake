#ifndef _FlareWave_
#define _FlareWave_

#include <DNest4.h>
#include <vector>
#include <RJObject.h>
#include "WaveDistribution.h"
#include "FlareDistribution.h"
#include "ImpulseDistribution.h"
#include "ChangepointDistribution.h"
#include "VectorMath_sse_mathfun.h"

class FlareWave
{
  private:
    DNest4::RJObject<WaveDistribution> waves;              // sinusoid distribution
    DNest4::RJObject<FlareDistribution> flares;            // flare distribution
    DNest4::RJObject<ImpulseDistribution> impulse;         // impulse distribution
    DNest4::RJObject<ChangepointDistribution> changepoint; // background change point distribution

#ifdef USE_SSE2
    // if using SSE2 use floats rather than double
    std::vector<float> mu; // the model vector
    std::vector<float> muwaves;
    std::vector<float> muflares;
    std::vector<float> muimpulse;
    std::vector<float> muchangepoint;
#else
    std::vector<double> mu; // the model vector
    std::vector<double> muwaves;
    std::vector<double> muflares;
    std::vector<double> muimpulse;
    std::vector<double> muchangepoint;
#endif
    
    bool firstiter;          // Set if first iteration of code
    double sigma;            // Noise standard deviation
    double background;       // A flat background offset level

    void calculate_mu(bool updateWaves=false, bool updateFlares=false, bool updateImpulse=false, bool updateChangepoints=false); // calculate the model
    
  public:
    FlareWave(); // constructor

    // Generate the point from the prior
    void from_prior(DNest4::RNG& rng);

    // Metropolis-Hastings proposals
    double perturb(DNest4::RNG& rng);

    // Likelihood function (this also generates the model)
    double log_likelihood() const;
    
    // Print to stream
    void print(std::ostream& out) const;

    // Return string with column information
    std::string description() const;
};

#endif

