#ifndef MATHHELPER_H
#define MATHHELPER_H

#include <cstdint>

#define EPSILON 1e-6f;
//#define ZERO(x) abs(x) < 1e-6f

#ifdef M_PI
#undef M_PI
#endif
#define M_PI	3.14159265358979323846f
#define INV_PI	0.31830988618379067154f
#define INV_2PI 0.15915494309189533577f
#define INV_4PI 0.07957747154594766788f

#ifndef INFINITY
#define INFINITY FLT_MAX
#endif

template<typename T>
inline bool zero(T val)
{
	return abs(val) < EPSILON;
}

template bool zero(float val);

template <>
inline bool zero(int val)
{
	return val == 0;
}

inline bool equal(float lhs, float rhs)
{
	return zero(lhs - rhs);
}

inline float lerp(float t, float a, float b)
{
	return (1.0f - t) * a + t * b;;
}

inline float clamp(float val, float low, float high)
{
	if (val < low) return low;
	else if (val > high) return high;
	else return val;
}

inline int mod(int a, int b)
{
	int n = int(a / b);
	a -= n*b;
	if (a < 0) a += b;
	return a;
}

inline int float2int(float f)
{
	return (int)f; // cut off fractional part
}

inline int round2int(float f)
{
	return int(round(f));
}

inline int floor2int(float f)
{
	return int(floor(f));
}

inline int ceil2int(float f)
{
	return int(ceil(f));
}

inline int log2int(float f)
{
	return floor2int(log2f(f));
}

inline bool is_power_of_2(int v)
{
	return (v & (v - 1)) == 0;
}

inline uint32_t round_up_pow_2(uint32_t v)
{
	v--;
	v |= v >> 1; v |= v >> 2;
	v |= v >> 4; v |= v >> 8;
	v |= v >> 16;
	return v + 1;
}

static float degrees(float radians)
{
	return radians * (180.0f / M_PI);
}

static float radians(float degrees)
{
	return degrees * (M_PI / 180.0f);
}

#endif // !MATHHELPER_H
