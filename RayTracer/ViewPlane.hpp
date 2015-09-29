#pragma once
class ViewPlane
{
public:
	ViewPlane() {}
	~ViewPlane() {}
	
	int hres;	// horizontal image resolution, width
	int vres;	// height
	float pixel_size;	// pixel size
	float gamma;
	float inv_gamma;

	void set_hres(const int hres) { this->hres = hres; }
	void set_vres(const int vres) { this->vres = vres; }
	void set_pixel_size(const float pixel_size) { this->pixel_size = pixel_size; }
	void set_gamma(const float gamma) { this->gamma = gamma; this->inv_gamma = 1.0f / gamma; }
};

