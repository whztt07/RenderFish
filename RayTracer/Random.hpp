#pragma once
#include "RenderFish.hpp"

// TODO: implement a custom pseudo-random number generator 
class Random
{
public:
	Random(uint32_t seed = 0) : _seed(seed) {
		mti = N + 1;
		this->seed(seed);
	}

	// set seed
	void seed(uint32_t seed = 0) const;

	// pseudo-random floating-point number in the range [0, 1)
	float random_float() const;

	// pseudo-random number in the range [0, 2^32)
	uint32_t random_uint() const;

private:
	static const int N = 624;
	mutable unsigned long mt[N]; /* the array for the state vector  */
	mutable int mti;
	uint32_t _seed;
};

typedef Random RNG;