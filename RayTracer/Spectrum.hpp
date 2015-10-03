#pragma once
#include "Math.hpp"

template <int n_samples>
class CoefficientSpectrum
{
protected:
	float c[n_samples];

public:
	CoefficientSpectrum(float v = 0.f) {
		for (int i = 0; i < n_samples; ++i) {
			c[i] = v;
		}
	}

	bool is_black() const {
		for (int i = 0; i < n_samples; ++i)
			if (c[i] != 0)
				return false;
		return true;
	}

	friend CoefficientSpectrum sqrt(const CoefficientSpectrum &s) {
		CoefficientSpectrum ret;
		for (int i = 0; i < n_samples; ++i)
			ret.c[i] = sqrtf(s.c[i]);
		return ret;
	}
	friend CoefficientSpectrum pow(const CoefficientSpectrum &s, float e) {
		CoefficientSpectrum ret;
		for (int i = 0; i < n_samples; ++i)
			ret.c[i] = powf(s.c[i], e);
		return ret;
	}
	friend CoefficientSpectrum exp(const CoefficientSpectrum &s) {
		CoefficientSpectrum ret;
		for (int i = 0; i < n_samples; ++i)
			ret.c[i] = expf(s.c[i]);
		return ret;
	}

	CoefficientSpectrum clamp(float low = 0, float high = INFINITY) const {
		CoefficientSpectrum ret;
		for (int i = 0; i < n_samples; ++i)
			ret.c[i] = ::clamp(c[i], low, high);
		return ret;
	}

	bool has_NaNs() const {
		for (int i = 0; i < n_samples; ++i)
			if (isnan(c[i]))
				return true;
		return false;
	}

	void operator+=(const CoefficientSpectrum &s2) {
		for (int i = 0; i < n_samples; ++i)
			c[i] += s2.c[i];
	}
	CoefficientSpectrum operator+(const CoefficientSpectrum &s2) const {
		CoefficientSpectrum ret = *this;
		for (int i = 0; i < n_samples; ++i)
			ret.c[i] += s2.c[i];
		return ret;
	}
};

enum SpectrumType { SPECTRUM_REFLECTANCE, SPECTRUM_ILLUMINANT };

class RGBSpectrum : public CoefficientSpectrum<3> {
public:
	RGBSpectrum(float v = 0.f) : CoefficientSpectrum<3>(v) { }
	RGBSpectrum(const CoefficientSpectrum<3> &v)
		: CoefficientSpectrum<3>(v) { }

	static RGBSpectrum from_rgb(const float rgb[3]) {
		RGBSpectrum s;
		s.c[0] = rgb[0];
		s.c[1] = rgb[1];
		s.c[2] = rgb[2];
		return s;
	}

	void to_rgb(float *rgb) const {
		rgb[0] = c[0];
		rgb[1] = c[1];
		rgb[2] = c[2];
	}
};

typedef RGBSpectrum Spectrum;