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

inline float cos_theta(const Vec3 &w) { return w.z; }
inline float abs_cos_theta(const Vec3 &w) { return fabsf(w.z); }
inline float sin_theta2(const Vec3& w) {
	return max(0.f, 1.f - cos_theta(w) * cos_theta(w));
}
inline float sin_theta(const Vec3& w) {
	return sqrtf(sin_theta2(w));
}
inline float CosPhi(const Vec3 &w) {
	float sintheta = sin_theta(w);
	if (sintheta == 0.f) return 1.f;
	return clamp(w.x / sintheta, -1.f, 1.f);
}
inline float SinPhi(const Vec3 &w) {
	float sintheta = sin_theta(w);
	if (sintheta == 0.f) return 0.f;
	return clamp(w.y / sintheta, -1.f, 1.f);
}


inline bool same_hemisphere(const Vec3 &w, const Vec3 &wp) {
	return w.z * wp.z > 0.f;
}


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
	virtual float pdf(const Vec3 &wi, const Vec3 &wo) const {
		return same_hemisphere(wo, wi) ? abs_cos_theta(wi) * INV_PI : 0.f;
	}
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

struct BSDFSampleOffsets {
	int n_samples, component_offset, dir_offset;

	BSDFSampleOffsets(int count, Sample *sample)
		: n_samples(count) {
		component_offset = sample->add_1D(n_samples);
		dir_offset = sample->add_2D(n_samples);
	}
};

class BSDFSample {
public:
	float u_dir[2], u_component;

	BSDFSample(float up0, float up1, float ucomp) {
		u_dir[0] = up0;
		u_dir[1] = up1;
		u_component = ucomp;
	}
	BSDFSample(RNG &rng) {
		u_dir[0] = rng.random_float();
		u_dir[1] = rng.random_float();
		u_component = rng.random_float();
	}
	BSDFSample(const Sample *sample, const BSDFSampleOffsets &offsets, uint32_t n) {
		u_dir[0] = sample->twoD[offsets.dir_offset][2 * n];
		u_dir[1] = sample->twoD[offsets.dir_offset][2 * n + 1];
		u_component = sample->oneD[offsets.component_offset][n];
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

	Spectrum sample_f(const Vec3 &wo_w, Vec3 *wi_w, const BSDFSample &bsdf_sample,
		float *pdf, BxDFType flags = BSDF_ALL, BxDFType *sampled_type = nullptr) const {
		// choose which BxDF to sample
		int matching_comps = num_components(flags);
		if (matching_comps == 0) {
			*pdf = 0.f;
			return Spectrum(0.f);
		}
		int which = min(floor2int(bsdf_sample.u_component * matching_comps), matching_comps - 1);

		BxDF *bxdf = nullptr;
		int count = which;
		for (int i = 0; i < _n_BxDFs; ++i) {
			if (_bxdfs[i]->matches_flags(flags) && count-- == 0) {
				bxdf = _bxdfs[i];
				break;
			}
		}
		// sample chosen BxDF
		Vec3 wo = world_to_local(wo_w);
		Vec3 wi;
		*pdf = 0.f;
		Spectrum f = bxdf->sample_f(wo, &wi, bsdf_sample.u_dir[0], bsdf_sample.u_dir[1], pdf);
		if (*pdf == 0.f)
			return 0.f;
		if (sampled_type) *sampled_type = bxdf->type;
		*wi_w = local_to_world(wi);
		// compute overall PDF with all matching BxDFs
		if (!(bxdf->type & BSDF_SPECULAR) && matching_comps > 1)
			for (int i = 0; i < _n_BxDFs; ++i)
				if (_bxdfs[i] != bxdf && _bxdfs[i]->matches_flags(flags))
					*pdf += _bxdfs[i]->pdf(wo, wi);
		if (matching_comps > 1)
			*pdf /= matching_comps;

		// compute value of BSDF for sampled direction
		if (!(bxdf->type & BSDF_SPECULAR)) {
			f = 0.f;
			if (dot(*wi_w, _normal_geom) * dot(wo_w, _normal_geom) > 0) // ignore BTDFs
				flags = BxDFType(flags & ~BSDF_TRANSMISSION);
			else // ignore BRDFs
				flags = BxDFType(flags & ~BSDF_REFLECTION);
			for (int i = 0; i < _n_BxDFs; ++i) {
				if (_bxdfs[i]->matches_flags(flags))
					f += _bxdfs[i]->f(wo, wi);
			}
		}
		return f;
		//info("BSDF::sample_f not implemented\n");
		//return Spectrum(0.f);
	}

	float Pdf(const Vec3 &wo, const Vec3 &wi, BxDFType flags = BSDF_ALL) const;

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