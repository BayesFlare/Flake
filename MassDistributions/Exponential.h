#ifndef _Exponential_
#define _Exponential_

#include "MassDistribution.h"

class Exponential:public MassDistribution
{
	private:
		// Range allowed for mu
		const double mu_min, mu_max;

		double mu;

		double perturb_parameters();

	public:
		Exponential();

		void fromPrior();
		double mass_log_pdf(double x) const;
		double mass_cdf(double x) const;
		double mass_cdf_inv(double u) const;
		void print(std::ostream& out) const;
};

#endif

