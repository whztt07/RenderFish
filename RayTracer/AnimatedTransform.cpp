#include "AnimatedTransform.hpp"

void AnimatedTransform::decompose(const Matrix4x4 & m, Vec3 * T, Quaternion * Rquat, Matrix4x4 * S) {
	// extract translation T
	T->x = m[0][3];
	T->y = m[1][3];
	T->z = m[2][3];

	// compute new transformation matrix without translation
	Matrix4x4 M = m;
	for (int i = 0; i < 3; ++i)
		M.m[i][3] = M.m[3][i] = 0.f;
	M.m[3][3] = 1.f;

	// extract rotation R
	float norm;
	int count = 0;
	Matrix4x4 R = M;
	do {
		// compute next matrix Rnext in series
		Matrix4x4 Rnext;
		Matrix4x4 Rit = inverse(transpose(R));
		for (int i = 0; i < 4; ++i)
			for (int j = 0; j < 4; ++j)
				Rnext[i][j] = 0.5f * (R.m[i][j] + Rit.m[i][j]);
		// compute norm of difference between R and Rnext
		norm = 0;
		for (int i = 0; i < 3; ++i) {
			float n = fabsf(R[i][0] - Rnext.m[i][0]) +
				fabsf(R[i][1] - Rnext[i][1]) +
				fabsf(R[i][2] - Rnext[i][2]);
			norm = max(norm, n);
		}
		R = Rnext;
	} while (++count < 100 && norm > .0001f);
	*Rquat = Quaternion(R);

	// compute scale S
	*S = inverse(R) * M;
}

void AnimatedTransform::interpolate(float time, Transform * t) const {
	// handle boundary conditions for matrix interpolation
	float dt = (time - _start_time) / (_end_time - _start_time);
	if (!_actually_animated || time <= _start_time) {
		*t = *_start_transform;
		return;
	}
	if (time >= _end_time) {
		*t = *_end_transform;
		return;
	}

	auto trans = (1.f - dt) * T[0] + dt * T[1];
	auto rotate = slerp(dt, R[0], R[1]);
	Matrix4x4 scale;
	for (int i = 0; i < 3; ++i)
		for (int j = 0; j < 3; ++j)
			scale[i][j] = lerp(dt, S[0][i][j], S[1][i][j]);

	*t = Transform::translate(trans) * rotate.to_transform() * Transform(scale);
}

inline BBox AnimatedTransform::motion_bounds(const BBox & b, bool use_inverse) const {
	if (!_actually_animated)
		return inverse(*_start_transform)(b);
	BBox ret;
	const int n_steps = 128;
	for (int i = 0; i < n_steps; ++i) {
		Transform t;
		float time = lerp(float(i) / float(n_steps - 1), _start_time, _end_time);
		interpolate(time, &t);
		if (use_inverse)
			t = inverse(t);
		ret = combine(ret, t(b));
	}
	return ret;
}
