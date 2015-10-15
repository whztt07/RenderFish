#include "Quaternion.hpp"

Quaternion::Quaternion(const Transform & t) {
	float zw = (t.m[0][1] - t.m[1][0]) * 0.25f;
	float yw = (t.m[2][0] - t.m[0][2]) * 0.25f;
	float xw = (t.m[1][2] - t.m[2][1]) * 0.25f;
	float c = zw * zw + yw * yw + xw * xw;
	// w^4 - w^2 + c = 0;
	float w20, w21;
	Assert(quadratic(1, -1, c, &w20, &w21));
	Assert(w21 >= 0 || w20 >= 0);
	w = w21 > 0 ? sqrtf(w21) : sqrtf(w20);
	v.x = xw / w;
	v.y = yw / w;
	v.z = zw / w;
}

Transform Quaternion::to_transform() const {
	float xx = v.x * v.x;
	float yy = v.y * v.y;
	float zz = v.z * v.z;
	float xw = v.x * w;
	float yw = v.y * w;
	float zw = v.z * w;
	float xy = v.x * v.y;
	float xz = v.x * v.z;
	float yz = v.y * v.z;
	Matrix4x4 m(
		1 - 2 * (yy + zz), 2 * (xy + zw), 2 * (xz - yw), 0,
		2 * (xy - zw), 1 - 2 * (xx + zz), 2 * (yz + xw), 0,
		2 * (xz + yw), 2 * (yz - xw), 1 - 2 * (xx + yy), 0,
		0, 0, 0, 1);
	return Transform(m);
}
