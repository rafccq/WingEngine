#pragma once

#include <gctypes.h>

struct Vec3 {
	f32 x, y, z;

	// ----------------------------------------------- Constructors -----------------------------------------------
	Vec3(): x(0.0f), y(0.0f), z(0.0f) { }

	Vec3(const f32 x, const f32 y, const f32 z): x(x), y(y), z(z) {}

	Vec3(const Vec3& v): x(v.x), y(v.y), z(v.z) { }

	// ----------------------------------------------- Arithmetic Ops -----------------------------------------------
	Vec3 operator-() const {
		return Vec3(-x, -y, -z);
	}

	Vec3 operator+(const Vec3& v) const {
		return Vec3(x + v.x, y + v.y, z + v.z);
	}

	Vec3& operator+=(const Vec3& v) {
		return *this = *this + v;
	}

	Vec3 operator-(const Vec3& v) const {
		return Vec3(x - v.x, y - v.y, z - v.z);
	}

	Vec3& operator-=(const Vec3& v) {
		return *this = *this - v;
	}

	Vec3 operator*(const f32 f) const {
		return Vec3(x * f, y * f, z * f);
	}

	Vec3& operator*=(const f32 f) {
		return *this = *this * f;
	}

	Vec3 operator/(const f32 f) const {
		return Vec3(x / f, y / f, z / f);
	}

	Vec3& operator/=(const f32 f) {
		return *this = *this / f;
	}
};

struct Matrix34 {
	f32 m[3][4];

	Vec3 operator*(const Vec3 v) {
		Vec3 result;
		result.x = m[0][0]*v.x + m[0][1]*v.y + m[0][2]*v.z + m[0][3];
		result.y = m[1][0]*v.x + m[1][1]*v.y + m[1][2]*v.z + m[1][3];
		result.z = m[2][0]*v.x + m[2][1]*v.y + m[2][2]*v.z + m[2][3];

		return result;
	}

	void operator*=(const f32 s) {
		m[0][0] *= s;	m[0][1] *= s;	m[0][2] *= s;
		m[1][0] *= s;	m[1][1] *= s;	m[1][2] *= s;
		m[2][0] *= s;	m[2][1] *= s;	m[2][2] *= s;
	}

	f32* operator[](unsigned int index) {
		return m[index];
	}
};

