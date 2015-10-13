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

class BSDF;

Spectrum specular_reflect(const RayDifferential &ray, BSDF *bsdf,
	RNG &rng, const Intersection &isect, const Renderer *renderer,
	const Scene *scene, const Sample *sample, MemoryArena &arena);


#define MAX_BxDFS 8

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

class BxDF {
public:
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

	Spectrum f(const Vec3 &wo_world, const Vec3 &wi_world, BxDFType flags = BSDF_ALL) const {
		
		Vec3 wi = world_to_local(wi_world), wo = world_to_local(wo_world);
		if (dot(wi_world, _normal_geom) * dot(wo_world, _normal_geom) > 0) // ignore BTDFs
			flags = BxDFType(flags & ~BSDF_ALL_TRANSMISSION);
		else // ignore BRDFs
			flags = BxDFType(flags & ~BSDF_REFLECTION);
		Spectrum f = 0.f;
		for (int i = 0; i < _n_BxDFs; ++i) {
			if (_bxdfs[i]->matches_flags(flags))
				f += _bxdfs[i]->f(wo, wi);
		}
		return f;
	}

	//Spectrum rho(RNG &rng, BxDFType flags = BSDF_ALL, int sqrt_samples = 6) const;
	//Spectrum rho(const Vec3 &wo, RNG &rng, BxDFType flags = BSDF_ALL,
	//	int sqrt_samples = 6) const;

	inline void add(BxDF *b) {
		Assert(_n_BxDFs < MAX_BxDFS);
		_bxdfs[_n_BxDFs++] = b;
	}

	int num_components() const { return _n_BxDFs; }
	int num_components(BxDFType flags) const {
		int count = 0;
		for (int i = 0; i < _n_BxDFs; i++) {
			if (_bxdfs[i]->type == flags)
				++count;
		}
		return count;
	}

	Vec3 world_to_local(const Vec3 &v) const {
		return Vec3(dot(v, _tangent), dot(v, _bitangent), dot(v, _normal_shading));
	}

	Vec3 local_to_world(const Vec3 &v) const {
		return Vec3(
			_tangent.x * v.x + _bitangent.x * v.y + _normal_shading.x * v.z,
			_tangent.y * v.x + _bitangent.y * v.y + _normal_shading.y * v.z,
			_tangent.z * v.x + _bitangent.z * v.y + _normal_shading.z * v.z);
	}
};

#define BSDF_ALLOC(arena, Type) new (arena.alloc(sizeof(Type))) Type