#include <chrono>
#include "../RayTracer/RenderFish.hpp"
#include "../RayTracer/Transform.hpp"

#include <xmmintrin.h>

using namespace std;

__m128 addWithAssembly(
	__m128 a,
	__m128 b)
{
	__m128 r;
	__asm
	{
		movaps xmm0, xmmword ptr[a]
		movaps xmm1, xmmword ptr[b]
		addps xmm0, xmm1
		movaps xmmword ptr[r], xmm0
	}
	return r;
}

__m128 addWithIntrinsics(
	__m128 a,
	__m128 b)
{
	__m128 r = _mm_add_ps(a, b);
	return r;
}

__declspec(align(16)) float A[] = { 2.0f, -1.0f, 3.0f, 4.0f };
__declspec(align(16)) float B[] = { -1.0f, 3.0f, 4.0f, 2.0f };
__declspec(align(16)) float C[] = { 0.0f, 0.0f, 0.0f, 0.0f };
__declspec(align(16)) float D[] = { 0.0f, 0.0f, 0.0f, 0.0f };

int main()
{
	log_system_init();
	Assert(1 == 1);
	//Assert(1 == 2);

	Transform t;
	//Assert(t.has_scale());
	t = Transform::scale(2, 2, 2);
	Assert(t.has_scale());
	Vec3 v(1, 2, 3);
	Point p(1, 2, 3);
	v = Transform::translate(1, 2, 3)(v);
	p = Transform::translate(1, 2, 3)(p);
	Assert(v == Vec3(1, 2, 3));
	Assert(p == Vec3(2, 4, 6));

	t = Transform::rotate(180, Vec3(1, 1, 0));
	v = t(Vec3(0, 1, 0));
	Assert(v == Vec3(1, 0, 0));

	info("hello %d\n", 1);
	warning("%s\n", "warning");
	error("%f\n", M_PI);

	//World w;
	//w.build();
	//w.render_scene();

	// load a and b from floating-point data arrays above
	__m128 a = _mm_load_ps(&A[0]);
	__m128 b = _mm_load_ps(&B[0]);
	// test the two functions
	__m128 c = addWithAssembly(a, b);
	__m128 d = addWithIntrinsics(a, b);
	// store the original values back to check that they
	// weren't overwritten
	_mm_store_ps(&A[0], a);
	_mm_store_ps(&B[0], b);
	// store results into float arrays so we can print
	// them
	_mm_store_ps(&C[0], c);
	_mm_store_ps(&D[0], d);
	// inspect the results
	printf("%g %g %g %g\n", A[0], A[1], A[2], A[3]);
	printf("%g %g %g %g\n", B[0], B[1], B[2], B[3]);
	printf("%g %g %g %g\n", C[0], C[1], C[2], C[3]);
	printf("%g %g %g %g\n", D[0], D[1], D[2], D[3]);

#define randv for (int i = 0; i < 4; i++) v4.v[i] = rand() / RAND_MAX;
#define randm for (int i = 0; i < 4; i++) for (int j = 0; j < 4; j++) m.m[i][j] = rand() / RAND_MAX;
#define randm2 for (int i = 0; i < 4; i++) for (int j = 0; j < 4; j++) m2.m[i][j] = rand() / RAND_MAX;

	const int times = 1e7;

	Vec4 v4;
	Matrix4x4 m;
	Matrix4x4 m2;
	Vec4 result;
	Matrix4x4 result_m;

	auto start = chrono::high_resolution_clock::now();
	for (int i = 0; i < times; i++)
	{
		//randv;
		randm;
		randm2;
		result_m = m * m2;
		//result = Matrix4x4::mul(m, v);
	}
	auto ms = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - start).count();

	printf("%lld\n", ms);

	start = chrono::high_resolution_clock::now();
	for (int i = 0; i < times; i++)
	{
		randv;
		randm;
		//randm2;
		result = m * v4;
		//result_m = Matrix4x4::mul(m, m2);
	}
	ms = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - start).count();

	printf("%lld\n", ms);

	float t0 = 0, t1 = 0;
	quadratic(1, 1, -6, &t0, &t1);
	Assert(t0 == -3.0f);
	Assert(t1 == 2.0f);

	std::cin >> ms;
	return 0;
}