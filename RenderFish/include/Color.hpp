#pragma once
#include "Math.hpp"

class Color
{
public:
	union {
		struct { int r, g, b, a; };
		int m[4];
	};
	
	Color() : Color(0, 0, 0, 255) {};
	Color(int r, int g, int b, int a = 255) : r(r), g(g), b(b), a(a) {}
	explicit Color(const Vec3& v) { for (int i = 0; i < 3; i++) m[i] = int(v[i] * 255);  a = 255; }
	explicit Color(const Normal& n) {
		r = int((n.x * 0.5 + 0.5) * 255); 
		g = int((n.y * 0.5 + 0.5) * 255); 
		b = int((n.z * 0.5 + 0.5) * 255); 
		a = 255; 
	}
	
	~Color() {};

	static const Color black;
	static const Color white;
	static const Color red;
	static const Color green;
	static const Color blue;
	static const Color yellow;
};

