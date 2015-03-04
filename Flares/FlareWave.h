#ifndef _FlareWave_
#define _FlareWave_

#include "Model.h"
#include <vector>
#include <RJObject.h>
#include "WaveDistribution.h"

class FlareWave:public DNest3::Model
{
  private:
    RJObject<WaveDistribution> waves;
    //RJObject<FlareDistribution> flares; // leave out for now

    double sigma; // Noise standard deviation

    // The signal
    std::vector<long double> mu;

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

