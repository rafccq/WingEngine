#include "Math_func.h"
#include "math.h"
#include <ogc/gu.h>

namespace Math3D {
	// ----------------------------------------------- Vector -----------------------------------------------
	f32 dot(const Vec3& va, const Vec3& vb) {
		return va.x*vb.x + va.y*vb.y + va.z*vb.z;
	}

	Vec3 cross(const Vec3& va, const Vec3& vb) {
		return Vec3(va.y * vb.z - va.z * vb.y, va.z * vb.x - va.x * vb.z, va.x * vb.y - va.y * vb.x);
	}

	f32 length(const Vec3& v) {
		return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
	}

	void normalize(Vec3& v) {
		f32 len = length(v);
		v.x /= len;
		v.y /= len;
		v.z /= len;
	}

	Vec3 normalized(const Vec3& v) {
		f32 len = length(v);
		Vec3 temp = v;
		temp.x /= len;
		temp.y /= len;
		temp.z /= len;

		return temp;
	}
	// ----------------------------------------------- Matrix -----------------------------------------------
	void rotateX(Matrix34& M, f32 angleRad) {
		f32 s = sin(angleRad);
		f32 c = cos(angleRad);

		M.m[0][0] = 1.0f;  M.m[0][1] = 0.0f;	M.m[0][2] = 0.0f;  M.m[0][3] = 0.0f;
		M.m[1][0] = 0.0f;  M.m[1][1] = c;		M.m[1][2] = -s;		M.m[1][3] = 0.0f;
		M.m[2][0] = 0.0f;  M.m[2][1] = s;		M.m[2][2] = c;		M.m[2][3] = 0.0f;
	}

	Matrix34 matrixRotationX(f32 angleRad) {
		Matrix34 M;
		rotateX(M, angleRad);
		return M;
	}

	void rotateY(Matrix34& M, f32 angleRad) {
		f32 s = sin(angleRad);
		f32 c = cos(angleRad);

		M.m[0][0] = c;		M.m[0][1] = 0.0f;		M.m[0][2] = s;		M.m[0][3] = 0.0f;
		M.m[1][0] = 0.0f;	M.m[1][1] = 1.0f;		M.m[1][2] = 0.0f;	M.m[1][3] = 0.0f;
		M.m[2][0] = -s;	M.m[2][1] = 0.0f;			M.m[2][2] = c;		M.m[2][3] = 0.0f;
	}

	Matrix34 matrixRotationY(f32 angleRad) {
		Matrix34 M;
		rotateY(M, angleRad);
		return M;
	}


	void rotateZ(Matrix34& M, f32 angleRad) {
		f32 s = sin(angleRad);
		f32 c = cos(angleRad);

		M.m[0][0] = c;		M.m[0][1] = -s;		M.m[0][2] = 0.0f;	M.m[0][3] = 0.0f;
		M.m[1][0] = s;		M.m[1][1] = c;		M.m[1][2] = 0.0f;	M.m[1][3] = 0.0f;
		M.m[2][0] = 0.0f;	M.m[2][1] = 0.0f;	M.m[2][2] = 1.0f;	M.m[2][3] = 0.0f;
	}

	Matrix34 matrixRotationZ(f32 angleRad) {
		Matrix34 M;
		rotateZ(M, angleRad);
		return M;
	}

	void rotateAxis(Matrix34& M, Vec3 axis, f32 angleRad) {
		f32 s = sinf(angleRad), c = cosf(angleRad);
		f32 t = 1.0f - c;
	
		Math3D::normalize(axis);
	
		f32 x = axis.x, y = axis.y, z = axis.z;

		f32 xSq = x*x, ySq = y*y, zSq = z*z;

		M.m[0][0] = (t * xSq)   + (c);
		M.m[0][1] = (t * x * y) - (s * z);
		M.m[0][2] = (t * x * z) + (s * y);
		M.m[0][3] =    0.0f;

		M.m[1][0] = (t * x * y) + (s * z);
		M.m[1][1] = (t * ySq)   + (c);
		M.m[1][2] = (t * y * z) - (s * x);
		M.m[1][3] =    0.0f;

		M.m[2][0] = (t * x * z) - (s * y);
		M.m[2][1] = (t * y * z) + (s * x);
		M.m[2][2] = (t * zSq)   + (c);
		M.m[2][3] = 0.0f;
	}


	Matrix34 matrixRotationAxis(Vec3 axis, f32 angleRad) {
		Matrix34 M;
		rotateAxis(M, axis, angleRad);
		return M;
	}

	Vec3 matrixVecMul(Mtx m, const Vec3& v) {
		Vec3 result;
		result.x = m[0][0]*v.x + m[0][1]*v.y + m[0][2]*v.z + m[0][3];
		result.y = m[1][0]*v.x + m[1][1]*v.y + m[1][2]*v.z + m[1][3];
		result.z = m[2][0]*v.x + m[2][1]*v.y + m[2][2]*v.z + m[2][3];

		return result;
	}
}