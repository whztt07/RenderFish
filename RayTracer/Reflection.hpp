#pragma once
#include "Math.hpp"
#include "Spectrum.hpp"
#include "Ray.hpp"
#include "Material.hpp"
#include "Renderer.hpp"

// i: incident directions
// t: transmitted directions
// eta: indices of refraction (wavelength-dependent)
Spectrum fresnel_dielectric(float cosi, float cost, const Spectrum &etai,
	const Spectrum &etat);

// i:	incident directions
// t:	transmitted directions
// eta: indices of refraction (wavelength-dependent)
// k:	absorption coefficient (wavelength-dependent)
Spectrum fresnel_conductor(float cosi, const Spectrum &eta, const Spectrum &k);

//Spectrum specular_reflect(const RayDifferential &ray, BSDF *bsdf,
//	RNG &rng, const Intersection &isect, const Renderer *renderer,
//	const Scene *scene, const Sample *sample, MemoryArena &arena) {
//
//	Vec3 wo = -ray.d, wi;
//	float pdf;
//	const Point &p = bsdf->shading_diff_geom.p;
//	const Normal &n = bsdf->shading_diff_geom.normal;
//	Spectrum f = bsdf->sample_f(wo, )
//}

#include "Spectrum.hpp"
#define MAX_BxDFS 8

class BxDF {
public:
	enum BxDFType {
		BSDF_REFLECTION = 1 << 0,
		BSDF_TRANSMISSION = 1 << 1,
		BSDF_DIFFUSE = 1 << 2,
		BSDF_GLOSSY = 1 << 3,
		BSDF_SPECULAR = 1 << 4,
		BSDF_ALL_TYPES = BSDF_DIFFUSE | BSDF_GLOSSY | BSDF_SPECULAR,
		BSDF_ALL_REFLECTION = BSDF_REFLECTION | BSDF_ALL_TYPES,
		BSDF_ALL_TRANSMISSION = BSDF_TRANSMISSION | BSDF_ALL_TYPES,
		BSDF_ALL = BSDF_ALL_REFLECTION | BSDF_ALL_TRANSMISSION
	};

	const BxDFType type;

	BxDF(BxDFType type) : type(type) {}

	bool matches_flags(BxDFType flags) const {
		return (type & flags) == type;
	}

	// wo: normalized outgoing direction
	// wi: normalized incident light direction
	virtual Spectrum f(const Vec3 &wo, const Vec3 &wi) const = 0;
	virtual Spectrum sample_f(const Vec3 &wo, Vec3 *wi, float u1, float u2, float *pdf) const { return Spectrum(0.f); }
	virtual Spectrum rho(const Vec3 &wo, int n_samples, const float *samples) const { return Spectrum(0.f); }
	virtual Spectrum rho(int nSamples, const float *samples1, const float *samples2) const { return Spectrum(0.f); }
};

class Fresnel {
public:
	// cosi: cos of incident direction
	virtual Spectrum evaluate(float cosi) const = 0;
};

class FresnelConductor : public Fresnel {
private:
	Spectrum _eta;	// eta: indices of refraction (wavelength-dependent)
	Spectrum _k;		// k:	absorption coefficient (wavelength-dependent)

public:
	FresnelConductor(const Spectrum &e, const Spectrum &kk)
		: _eta(e), _k(kk) {
	}

	virtual Spectrum evaluate(float cosi) const override {
		return fresnel_conductor(fabsf(cosi), _eta, _k);
	}
};

class FresnelDielectric : public Fresnel {
private:
	// indices of refraction on the two sides of the surface
	float _eta_i, _eta_t;
public:
	FresnelDielectric(float etai, float etat)
		: _eta_i(etai), _eta_t(etat) {
	}

	virtual Spectrum evaluate(float cosi) const override;
};

class FresnelNoOp : public Fresnel {
public:
	virtual Spectrum evaluate(float cosi) const override {
		return Spectrum(1.f);
	}
};

class Lambertian : public BxDF {
private:
	Spectrum R;

public:
	Lambertian(const Spectrum &reflectance)
		: BxDF(BxDFType(BSDF_REFLECTION)), R(reflectance) {

	}

	virtual Spectrum f(const Vec3 &wo, const Vec3 &wi) const override {
		return R * INV_PI;
	}

	virtual Spectrum rho(const Vec3 &wo, int n_samples, const float *samples) const {
		return R;
	}

	virtual Spectrum rho(int nSamples, const float *samples1, const float *samples2) const {
		return R;
	}
};

class BSDF {
public:
	const DifferentialGeometry shading_diff_geom;
	const float eta;	// index of refraction

private:
	Normal _normal_shading, _normal_geom;
	Vec3 _tangent, _bitangent;
	int _n_BxDFs;
	BxDF *_bxdfs[MAX_BxDFS];

public:
	BSDF(const DifferentialGeometry &dg, const Normal &n_geom, float eta = 1.f)
		: shading_diff_geom(dg), eta(eta), _normal_geom(n_geom), _normal_shading(shading_diff_geom.normal),
		_tangent(normalize(shading_diff_geom.dpdu)), _bitangent(cross(_normal_shading, _tangent)), _n_BxDFs(0) {
	}

	inline void add(BxDF *b) {
		Assert(_n_BxDFs < MAX_BxDFS);
		_bxdfs[_n_BxDFs++] = b;
	}
};

#define BSDF_ALLOC(arena, Type) new (arena.alloc(sizeof(Type))) Type