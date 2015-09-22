#ifndef _FlareWave_
#define _FlareWave_

#include <dnest3/Model.h>
#include <vector>
#include <rjobject/RJObject.h>
#include "WaveDistribution.h"
#include "FlareDistribution.h"
#include "ImpulseDistribution.h"
#include "ChangepointDistribution.h"

class FlareWave:public DNest3::Model
{
  private:
    RJObject<WaveDistribution> waves;              // sinusoid distribution
    RJObject<FlareDistribution> flares;            // flare distribution
    RJObject<ImpulseDistribution> impulse;         // impulse distribution
    RJObject<ChangepointDistribution> changepoint; // background change point distribution
    
    double sigma; // Noise standard deviation
    double background; // A flat background offset level
    
  public:
    FlareWave(); // constructor

    // Generate the point from the prior
    void fromPrior();

    // Metropolis-Hastings proposals
    double perturb();

    // Likelihood function (this also generates the model)
    double logLikelihood() const;
    
    // Print to stream
    void print(std::ostream& out) const;

    // Return string with column information
    std::string description() const;
};

#endif

