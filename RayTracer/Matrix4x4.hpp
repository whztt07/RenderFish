#pragma once

#include <cstring>
#include "Vector.h"

struct Matrix4x4
{
	float m[4][4];
	
	Matrix4x4() { 
		//ZeroMemory(this, sizeof(*this));
		memset(this, 0, sizeof(*this));
		m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1;
	}
	Matrix4x4(float mat[4][4])
	{
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				m[i][j] = mat[i][j];
	}
	Matrix4x4(float m11, float m12, float m13, float m14,
		float m21, float m22, float m23, float m24,
		float m31, float m32, float m33, float m34,
		float m41, float m42, float m43, float m44)
	{
		m[0][0] = m11; m[0][1] = m12; m[0][2] = m13; m[0][3] = m14;
		m[1][0] = m21; m[1][1] = m22; m[1][2] = m23; m[1][3] = m24;
		m[2][0] = m31; m[2][1] = m32; m[2][2] = m33; m[2][3] = m34;
		m[3][0] = m41; m[3][1] = m42; m[3][2] = m43; m[3][3] = m44;
	}

	static Matrix4x4 transpose(const Matrix4x4& m) {
		return Matrix4x4(
			m[0][0], m[1][0], m[2][0], m[3][0],
			m[0][1], m[1][1], m[2][1], m[3][1],
			m[0][2], m[1][2], m[2][2], m[3][2],
			m[0][3], m[1][3], m[2][3], m[3][3]);
	}

	const float* operator[](int index) const { return m[index]; }
	float* operator[](int index) { return m[index]; }
	float& operator()(int row, int col) { return m[row][col]; }
	float operator()(int row, int col) const { return m[row][col]; }

	friend const Matrix4x4 operator*(const Matrix4x4& lhs, const Matrix4x4& rhs)
	{
		Matrix4x4 result;
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				result[i][j] = 0;
				for (int k = 0; k < 4; k++)
				{
					result[i][j] += lhs[i][k] * rhs[k][j];
				}
			}
		}
		return result;
	}

	friend const Vec4 operator*(const Matrix4x4& lhs, const Vec4& rhs)
	{
		Vec4 result;
		for (int i = 0; i < 4; i++)
		{
			result[i] = rhs.x * lhs[i][0] + rhs.y * lhs[i][1] + rhs.z * lhs[i][2] + rhs.w * lhs[i][3];
		}
		return result;
	}
};

