#ifndef _RJObject_
#define _RJObject_

#include <vector>
#include <ostream>

/*
* A class that implements basic birth-death Metropolis-Hastings
* proposals using an exponential prior on the masses and a
* uniform prior on the positions. This is designed for 1, 2,
* or 3-dimensional objects. This class stores masses for
* each of the components it has, but it does not store
* any other properties. If your components have more properties
* you may derive from this class or store them externally.
*/

template<class Distribution>
class RJObject
{
	protected:
		// How many parameters for each component
		int num_dimensions;

		// Maximum number of components allowed (minimum is zero)
		int max_num_components;

		// The hyperparameters that specify the conditional prior
		// for the components
		Distribution dist;

		// The components
		int num_components;
		std::vector< std::vector<double> > components;

		// Transformed into iid U(0, 1) priors
		std::vector< std::vector<double> > u_components;

		// Helper methods -- these do one thing at a time
		double perturb_num_components(double scale);
		double perturb_components(double chance, double scale);

		// Helper methods -- add or remove single component
		double add_component();
		double remove_component();

	public:
		/*
		* num_dimensions: number of dimensions for each object.
		* "mass" etc count. E.g. in StarField problem (x, y, flux)
		* specifies a star, so num_dimensions = 3.
		*
		* max_num_components: obvious
		* fix: if true, doesn't do RJ steps. N will be fixed at
		* max_num_components
		*/
		RJObject(int num_dimensions, int max_num_components, bool fix,
				const Distribution& dist);

		// Generate everything from the prior
		void fromPrior();

		// The top-level perturb method
		double perturb();

		// For output
		void print(std::ostream& out);
};

//#include "RJObjectImpl.h"

#endif

