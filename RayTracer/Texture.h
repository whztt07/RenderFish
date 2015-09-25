#pragma once

template <typename T>
class Texture : public ReferenceCounted
{
public:
	Texture();
	virtual ~Texture();

	virtual T evaluate(const DifferectialGeometry &) const = 0;
};

template <typename T>
class ConstantTexture : public Texture<T>
{
public:
	ConstantTexture(const T &v) { value = v; }

	T evaluate(const DifferectialGeometry &) const {
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

	T evaluate(const DifferectialGeometry & diff_geo) const {
		return tex1->evaluate(diff_geo) * tex2->evaluate(diff_geo);
	}

private:
	Reference<Texture<T1>> tex1;
	Reference<Texture<T2>> tex2;
};