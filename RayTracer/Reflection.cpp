#include "Reflection.hpp"


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