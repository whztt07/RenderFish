#pragma once
#include "Math.hpp"
#include "Spectrum.hpp"

class AreaLight
{
public:
	AreaLight();
	~AreaLight();

	virtual Spectrum L(const Point &p, const Normal &n, const Vec3 &w) const = 0;
};

