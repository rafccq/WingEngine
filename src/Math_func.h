#pragma once

#include <ogc\gu.h>

#include "Math_types.h"

namespace Math3D {
	// ----------------------------------------------- Vector -----------------------------------------------
	f32 dot(const Vec3& va, const Vec3& vb);

	Vec3 cross(const Vec3& va, const Vec3& vb);

	f32 length(const Vec3& v);

	void normalize(Vec3& v);

	Vec3 normalized(const Vec3& v);

	// ----------------------------------------------- Matrix -----------------------------------------------
	void rotateX(Matrix34& M, f32 angleRad);

	Matrix34 matrixRotationX(f32 angleRad);

	void rotateY(Matrix34& M, f32 angleRad);

	Matrix34 matrixRotationY(f32 angleRad);

	void rotateZ(Matrix34& M, f32 angleRad);

	Matrix34 matrixRotationZ(f32 angleRad);

	void rotateAxis(Matrix34& M, Vec3 axis, f32 angleRad);

	Matrix34 matrixRotationAxis(Vec3 axis, f32 angleRad);

	// talvez essa função é TEMP
	Vec3 matrixVecMul(Mtx m, const Vec3& v);
}