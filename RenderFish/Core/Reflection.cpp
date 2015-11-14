#include "Reflection.hpp"
#include "MonteCarlo.hpp"


Spectrum fresnel_dielectric(float cosi, float cost, const Spectrum &etai, const Spectrum &etat)
{
	Spectrum r_parallel = ((etat * cosi) - (etai * cost))
		/ ((etat * cosi) + (etai * cost));
	Spectrum r_perpendicular = ((etai * cosi) - (etat * cost))
		/ ((etai * cosi) + (etat * cost));
	return (r_parallel * r_parallel + r_perpendicular * r_perpendicular) * 0.5f;
}

Spectrum fresnel_conductor(float cosi, const Spectrum &eta, const Spectrum &k)
{
	Spectrum tmp_f = eta*eta + k*k;
	Spectrum tmp = tmp_f * cosi*cosi;
	Spectrum r_paral_2 = (tmp - (2.f * eta * cosi) + 1) /
		(tmp + (2.f * eta * cosi) + 1);
	Spectrum r_perp_2 = (tmp_f - (2.f * eta * cosi) + cosi*cosi) /
		(tmp_f + (2.f * eta * cosi) + cosi*cosi);
	return (r_paral_2 + r_perp_2) * 0.f;
}

//Spectrum specular_reflect(const RayDifferential & ray, BSDF * bsdf, RNG & rng, const Intersection & isect, const Renderer * renderer, const Scene * scene, const Sample * sample, MemoryArena & arena) {
//
//	Vec3 wo = -ray.d, wi;
//	float pdf;
//	const Point &p = bsdf->shading_diff_geom.p;
//	const Normal &n = bsdf->shading_diff_geom.normal;
//	Spectrum f = bsdf->sample_f(wo, &wi, BSDFSample(rng), &pdf,
//								BxDFType(BSDF_REFLECTION | BSDF_SPECULAR));
//
//	Spectrum L = 0.f;
//	float cos_item = abs_dot(wi, n);
//	if (pdf > 0.f && !f.is_black() && cos_item != 0.f) {
//		// compute ray differential rd for specular reflection
//		RayDifferential rd(p, wi, ray, isect.ray_epsilon);
//		if (ray.has_differentials) {
//			rd.has_differentials = true;
//			rd.rx_origin = p + isect.dg.dpdx;
//			rd.ry_origin = p + isect.dg.dpdy;
//			Normal dndx =   bsdf->shading_diff_geom.dndu * bsdf->shading_diff_geom.dudx +
//							bsdf->shading_diff_geom.dndv * bsdf->shading_diff_geom.dvdx;
//			Normal dndy =   bsdf->shading_diff_geom.dndu * bsdf->shading_diff_geom.dudy +
//							bsdf->shading_diff_geom.dndv * bsdf->shading_diff_geom.dvdy;
//			Vec3 dwodx = -ray.rx_direction - wo, dwody = -ray.ry_direction - wo;
//			float dDNdx = dot(dwodx, n) + dot(wo, dndx);
//			float dDNdy = dot(dwody, n) + dot(wo, dndy);
//			rd.rx_direction = wi - dwodx + 2 * Vec3(dot(wo, n) * dndx + dDNdx * n);
//			rd.ry_direction = wi - dwody + 2 * Vec3(dot(wo, n) * dndy + dDNdy * n);
//		}
//
//		Spectrum Li = renderer->Li(scene, rd, sample, rng, arena);
//		float rgb[3];
//		Li.to_rgb(rgb);
//		if (rgb[0] != 0.f) {
//			info("here");
//		}
//
//		L = f * Li * cos_item / pdf;
//	}
//
//	return L;
//}

Spectrum specular_reflect(const RayDifferential &ray, BSDF *bsdf,
	RNG &rng, const Intersection &isect, const Renderer *renderer,
	const Scene *scene, const Sample *sample, MemoryArena &arena) 
{
	Vec3 wo = -ray.d, wi;
	float pdf;
	const Point &p = bsdf->shading_diff_geom.p;
	const Normal &n = bsdf->shading_diff_geom.normal;
	Spectrum f = bsdf->sample_f(wo, &wi, BSDFSample(rng), &pdf,
		BxDFType(BSDF_REFLECTION | BSDF_SPECULAR));
	Spectrum L = 0.f;
	if (pdf > 0.f && !f.is_black() && abs_dot(wi, n) != 0.f) {
		// Compute ray differential _rd_ for specular reflection
		RayDifferential rd(p, wi, ray, isect.ray_epsilon);
		if (ray.has_differentials) {
			rd.has_differentials = true;
			rd.rx_origin = p + isect.dg.dpdx;
			rd.ry_origin = p + isect.dg.dpdy;
			// Compute differential reflected directions
			Normal dndx = bsdf->shading_diff_geom.dndu * bsdf->shading_diff_geom.dudx +
						  bsdf->shading_diff_geom.dndv * bsdf->shading_diff_geom.dvdx;
			Normal dndy = bsdf->shading_diff_geom.dndu * bsdf->shading_diff_geom.dudy +
						  bsdf->shading_diff_geom.dndv * bsdf->shading_diff_geom.dvdy;
			Vec3 dwodx = -ray.rx_direction - wo, dwody = -ray.ry_direction - wo;
			float dDNdx = dot(dwodx, n) + dot(wo, dndx);
			float dDNdy = dot(dwody, n) + dot(wo, dndy);
			rd.rx_direction = wi - dwodx + 2 * Vec3(dot(wo, n) * dndx + dDNdx * n);
			rd.ry_direction = wi - dwody + 2 * Vec3(dot(wo, n) * dndy + dDNdy * n);
		}
		
		//Spectrum Li = renderer->Li(scene, rd, sample, rng, arena);
		//L = f * Li * abs_dot(wi, n) / pdf;
		L = f * abs_dot(wi, n) / pdf;
	}
	return L;
}

Spectrum FresnelDielectric::evaluate(float cosi) const
{
	cosi = clamp(cosi, -1.f, 1.f);
	bool entering = cosi > 0.f;
	float ei = _eta_i, et = _eta_t;
	if (!entering)
		std::swap(ei, et);
	float sint = ei / et * sqrtf(max(0.f, 1.f - cosi * cosi));	// Snell's law

	// handle total internal reflection
	if (sint >= 1.f)
		return 1.f;

	float cost = sqrtf(max(0.f, 1.f - sint*sint));
	return fresnel_dielectric(fabsf(cosi), cost, ei, et);
}

Spectrum BxDF::sample_f(const Vec3 &wo, Vec3 *wi, float u1, float u2, float *pdf) const
{
	*wi = CosineSampleHemisphere(u1, u2);
	if (wo.z < 0.f)
		wi->z *= -1.f;
	*pdf = this->pdf(wo, *wi);
	return this->f(wo, *wi);
}

Spectrum BxDF::rho(const Vec3 &wo, int n_samples, const float *samples) const
{
	Spectrum r = 0;
	for (int i = 0; i < n_samples; ++i) {
		Vec3 wi;
		float pdf = 0.f;
		Spectrum f = sample_f(wo, &wi, samples[2 * i], samples[2 * i + 1], &pdf);
		if (pdf > 0.) r += f * abs_cos_theta(wi) / pdf;
	}
	return r / float(n_samples);
}

Spectrum BxDF::rho(int nSamples, const float *samples1, const float *samples2) const
{
	Spectrum r = 0.;
	for (int i = 0; i < nSamples; ++i) {
		// Estimate one term of $\rho_\roman{hh}$
		Vec3 wo, wi;
		wo = UniformSampleHemisphere(samples1[2 * i], samples1[2 * i + 1]);
		float pdf_o = INV_2PI, pdf_i = 0.f;
		Spectrum f = sample_f(wo, &wi, samples2[2 * i], samples2[2 * i + 1], &pdf_i);
		if (pdf_i > 0.)
			r += f * abs_cos_theta(wi) * abs_cos_theta(wo) / (pdf_o * pdf_i);
	}
	return r / (M_PI*nSamples);
}

Spectrum BSDF::f(const Vec3 &wo_world, const Vec3 &wi_world, BxDFType flags /*= BSDF_ALL*/) const
{

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

Spectrum BSDF::sample_f(const Vec3 &wo_w, Vec3 *wi_w, const BSDFSample &bsdf_sample, float *pdf, BxDFType flags /*= BSDF_ALL*/, BxDFType *sampled_type /*= nullptr*/) const
{
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
	Assert(bxdf);

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

float BSDF::pdf(const Vec3 &wo_w, const Vec3 &wi_w, BxDFType flags /*= BSDF_ALL*/) const
{
	if (_n_BxDFs == 0)
		return 0;
	Vec3 wo = world_to_local(wo_w), wi = world_to_local(wi_w);
	float pdf = 0.f;
	int matching_comps = 0;
	for (int i = 0; i < _n_BxDFs; ++i) {
		if (_bxdfs[i]->matches_flags(flags)) {
			++matching_comps;
			pdf += _bxdfs[i]->pdf(wo, wi);
		}
	}
	float v = matching_comps > 0 ? pdf / matching_comps : 0.f;
	return v;
}
