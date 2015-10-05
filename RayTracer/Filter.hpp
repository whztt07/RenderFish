#pragma once
#include "Math.hpp"

class Filter
{
public:
	const float x_width, y_width;
	const float inv_x_width, inv_y_width;

public:
	Filter(float xw, float yw)
		: x_width(xw), y_width(yw), inv_x_width(1.f / xw), inv_y_width(1.f / yw) {
	}

	virtual float evaluate(float x, float y) const = 0;
};

class BoxFilter : public Filter {
public:
	BoxFilter(float xw, float yw) : Filter(xw, yw) {}
	virtual float evaluate(float x, float y) const override {
		return 1.f;
	}
};

class TriangleFilter : public Filter {
public:
	TriangleFilter(float xw, float yw) : Filter(xw, yw) {}
	virtual float evaluate(float x, float y) const override {
		return max(0.f, x_width - fabsf(x)) *
			max(0.f, y_width - fabsf(y));
	}
};

class GaussianFilter : public Filter {
private:
	const float alpha;
	const float exp_x, exp_y;
public:
	GaussianFilter(float xw, float yw, float a) : Filter(xw, yw), alpha(a),
		exp_x(expf(-a*x_width*x_width)), exp_y(expf(-a*y_width*y_width)) {}
	
	virtual float evaluate(float x, float y) const override {
		return gaussian(x, exp_x) * gaussian(y, exp_y);
	}

private:
	float gaussian(float d, float expv) const {
		return max(0.f, float(expf(-alpha * d * d) - expv));
	}
};

class MitchellFilter : public Filter {
private:
	const float B, C;

public:
	MitchellFilter(float b, float c, float xw, float yw) : Filter(xw, yw), B(b), C(c) {}
	
	virtual float evaluate(float x, float y) const override {
		return Mitchell1D(x * inv_x_width) * Mitchell1D(y * inv_y_width);
	}

private:
	float Mitchell1D(float x) const {
		x = fabsf(2.f * x);
		if (x > 1.f)
			return (1.f / 6.f) * ((((-B - 6 * C) * x + (6 * B + 30 * C)) * x + (-12 * B - 48 * C)) * x + (8 * B + 24 * C));
		else
			return (1.f / 6.f) * (((12 - 9*B - 6 * C) * x + (-18 + 12*B + 6 * C)) * x * x + (6 - 2 * B));
	}
};

class LanczosFilter : public Filter {
private:
	const float tau;

public:
	LanczosFilter(float xw, float yw, float t) : Filter(xw, yw), tau(t) {}
	virtual float evaluate(float x, float y) const override {
		return sinc_1D(x * inv_x_width) * sinc_1D(y * inv_y_width);
	}

private:
	float sinc_1D(float x) const {
		x = fabsf(x);
		if (x < 1e-5f) return 1.f;
		if (x > 1.f) return 0.f;
		x *= M_PI;
		float sinc = sinf(x * tau) / (x * tau);
		float lanczos = sinf(x) / x;
		return sinc * lanczos;
	}
};