#pragma once

#include "ReferenceCounted.hpp"
#include "DifferentialGeometry.hpp"

template <typename T>
class Texture : public ReferenceCounted
{
public:
	virtual ~Texture() {};
	virtual T evaluate(const DifferentialGeometry &) const = 0;
};

template <typename T>
class ConstantTexture : public Texture<T>
{
public:
	ConstantTexture(const T &v) { value = v; }

	T evaluate(const DifferentialGeometry &) const {
		return value;
	}

private:
	T value;
};

template <typename T1, typename T2>
class ScaleTexture : public Texture < T2 >
{
public:
	ScaleTexture(Reference<Texture<T1>> t1, Reference<Texture<T2>> t2)
		: tex1(t1), tex2(t2) {
	}

	auto evaluate(const DifferentialGeometry & diff_geo) const -> decltype(T1(1) * T2(1)) {
		return tex1->evaluate(diff_geo) * tex2->evaluate(diff_geo);
	}

private:
	Reference<Texture<T1>> tex1;
	Reference<Texture<T2>> tex2;
};
