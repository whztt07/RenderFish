#pragma once
#include "ReferenceCounted.hpp"
#include "DifferentialGeometry.hpp"
#include "MemoryArena.hpp"
#include "Spectrum.hpp"
#define MAX_BxDFS 8

class BxDF {
public:
	enum BxDFType {
		BSDF_REFLECTION		= 1 << 0,
		BSDF_TRANSMISSION	= 1 << 1,
		BSDF_DIFFUSE		= 1 << 2,
		BSDF_GLOSSY			= 1 << 3,
		BSDF_SPECULAR		= 1 << 4,
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
	const DifferentialGeometry dg_shading;
	const float eta;	// index of refraction

private:
	Normal nn, n_geom;
	Vec3 sn, tn;
	int n_BxDFs;
	BxDF *bxdfs[MAX_BxDFS];

public:
	BSDF(const DifferentialGeometry &dg, const Normal &n_geom, float eta = 1.f) 
		: dg_shading(dg), eta(eta), n_geom(n_geom), nn(dg_shading.normal),
		 sn(normalize(dg_shading.dpdu)), tn(cross(nn, sn)), n_BxDFs(0) {
	}

	inline void add(BxDF *b) {
		Assert(n_BxDFs < MAX_BxDFS);
		bxdfs[n_BxDFs++] = b;
	}
};

#define BSDF_ALLOC(arena, Type) new (arena.alloc(sizeof(Type))) Type

class Material : public ReferenceCounted
{
public:
	Material();
	virtual ~Material();

	virtual BSDF *get_BSDF(const DifferentialGeometry &dg_geom, const DifferentialGeometry &dg_shading,
		MemoryArena &arena) const = 0;
};