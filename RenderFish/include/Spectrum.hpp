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
		Assert(!has_NaNs());
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

	bool operator==(const CoefficientSpectrum &s) const {
		for (int i = 0; i < n_samples; ++i)
			if (!equal(c[i], s.c[i]))
				return false;
		return true;
	}
	bool operator!=(const CoefficientSpectrum &s) const {
		return !((*this) == s)
	}

	void operator+=(const CoefficientSpectrum &s2) {
		for (int i = 0; i < n_samples; ++i)
			c[i] += s2.c[i];
	}
	void operator-=(const CoefficientSpectrum &s2) {
		for (int i = 0; i < n_samples; ++i)
			c[i] -= s2.c[i];
	}
	void operator*=(const CoefficientSpectrum &s2) {
		for (int i = 0; i < n_samples; ++i)
			c[i] *= s2.c[i];
	}
	void operator/=(const CoefficientSpectrum &s2) {
		for (int i = 0; i < n_samples; ++i)
			c[i] /= s2.c[i];
		Assert(!has_NaNs());
	}
	CoefficientSpectrum operator+(const CoefficientSpectrum &s2) const {
		CoefficientSpectrum ret = *this;
		for (int i = 0; i < n_samples; ++i)
			ret.c[i] += s2.c[i];
		return ret;
	}
	CoefficientSpectrum operator-(const CoefficientSpectrum &s2) const {
		CoefficientSpectrum ret = *this;
		for (int i = 0; i < n_samples; ++i)
			ret.c[i] -= s2.c[i];
		return ret;
	}
	CoefficientSpectrum operator*(const CoefficientSpectrum &s2) const {
		CoefficientSpectrum ret = *this;
		for (int i = 0; i < n_samples; ++i)
			ret.c[i] *= s2.c[i];
		return ret;
	}
	CoefficientSpectrum operator/(const CoefficientSpectrum &s2) const {
		CoefficientSpectrum ret = *this;
		for (int i = 0; i < n_samples; ++i)
			ret.c[i] /= s2.c[i];
		Assert(!ret.has_NaNs());
		return ret;
	}
	CoefficientSpectrum operator+(float f) const {
		CoefficientSpectrum ret = *this;
		for (int i = 0; i < n_samples; ++i)
			ret.c[i] += f;
		return ret;
	}
	CoefficientSpectrum operator-(float f) const {
		CoefficientSpectrum ret = *this;
		for (int i = 0; i < n_samples; ++i)
			ret.c[i] -= f;
		return ret;
	}
	CoefficientSpectrum operator*(float f) const {
		CoefficientSpectrum ret = *this;
		for (int i = 0; i < n_samples; ++i)
			ret.c[i] *= f;
		return ret;
	}
	CoefficientSpectrum operator/(float f) const {
		Assert(!zero(f));
		CoefficientSpectrum ret = *this;
		for (int i = 0; i < n_samples; ++i)
			ret.c[i] /= f;
		//Assert(!ret.has_NaNs());
		return ret;
	}
	friend CoefficientSpectrum operator+(float f, const CoefficientSpectrum& s) {
		CoefficientSpectrum ret = s;
		for (int i = 0; i < n_samples; ++i)
			ret.c[i] += f;
		return ret;
	}
	friend CoefficientSpectrum operator-(float f, const CoefficientSpectrum& s) {
		CoefficientSpectrum ret = s;
		for (int i = 0; i < n_samples; ++i)
			ret.c[i] -= f;
		return ret;
	}
	friend CoefficientSpectrum operator*(float f, const CoefficientSpectrum& s) {
		CoefficientSpectrum ret = s;
		for (int i = 0; i < n_samples; ++i)
			ret.c[i] *= f;
		return ret;
	}
	friend CoefficientSpectrum operator/(float f, const CoefficientSpectrum& s) {
		Assert(!zero(f));
		CoefficientSpectrum ret = s;
		for (int i = 0; i < n_samples; ++i)
			ret.c[i] /= f;
		return ret;
	}
};

enum SpectrumType { SPECTRUM_REFLECTANCE, SPECTRUM_ILLUMINANT };

inline void XYZToRGB(const float xyz[3], float rgb[3]) {
	rgb[0] = 3.240479f*xyz[0] - 1.537150f*xyz[1] - 0.498535f*xyz[2];
	rgb[1] = -0.969256f*xyz[0] + 1.875991f*xyz[1] + 0.041556f*xyz[2];
	rgb[2] = 0.055648f*xyz[0] - 0.204043f*xyz[1] + 1.057311f*xyz[2];
}

inline void RGBToXYZ(const float rgb[3], float xyz[3]) {
	xyz[0] = 0.412453f*rgb[0] + 0.357580f*rgb[1] + 0.180423f*rgb[2];
	xyz[1] = 0.212671f*rgb[0] + 0.715160f*rgb[1] + 0.072169f*rgb[2];
	xyz[2] = 0.019334f*rgb[0] + 0.119193f*rgb[1] + 0.950227f*rgb[2];
}

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
		Assert(!s.has_NaNs());
		return s;
	}

	void to_rgb(float rgb[3]) const {
		rgb[0] = c[0];
		rgb[1] = c[1];
		rgb[2] = c[2];
	}

	const RGBSpectrum& to_rgb_spectrum() const {
		return *this;
	}

	void to_XYZ(float xyz[3]) const {
		RGBToXYZ(c, xyz);
	}

	static RGBSpectrum from_xyz(const float xyz[3]) {
		RGBSpectrum s;
		XYZToRGB(xyz, s.c);
		return s;
	}

	float y() const {
		const float y_weights[3] = { 0.212671f, 0.715160f, 0.072169f };
		return y_weights[0] * c[0] + y_weights[1] * c[1] * y_weights[2] * c[2];
	}

	//static RGBSpectrum from_sampled(const float *lambda, const float *v, int n);
};

typedef RGBSpectrum Spectrum;

template Spectrum lerp(float t, const Spectrum &s1, const Spectrum &s2);