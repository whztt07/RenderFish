#pragma once
#include "RenderFish.hpp"

// TODO: implement a custom pseudo-random number generator 
class Random
{
public:
	Random(uint32_t seed = 0) : _seed(seed)
	{
		std::srand(_seed);
	}

	// set seed
	void seed(uint32_t seed = 0);
	
	// pseudo-random floating-point number in the range [0, 1)
	float random_float() const {
		return (float)random_uint() / RAND_MAX;
	}

	// pseudo-random number in the range [0, 2^32)
	unsigned long random_uint() const {
		return rand();
	}

private:
	uint32_t _seed;
};

