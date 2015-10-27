#include "Camera.h"
#include "Math_func.h"

#include <ogc/gx.h>
#include <math.h>

void setVec3(Vec3& dest, f32 src[3]) {
	dest.x = src[0];
	dest.y = src[1];
	dest.z = src[2];
}

Camera::Camera() {
	mPosition = Vec3(0, 0, 0);
	mUp		= Vec3(0, 1, 0);
	mLook	= Vec3(0, 0, -1);
	mRight	= Vec3(1, 0, 0);
}

void Camera::setProj(f32 fov, f32 aspect, f32 near, f32 far) {
	guPerspective(projMatrix, fov, aspect, near, far);
}

void Camera::yaw(float theta) {
	theta = DegToRad(theta);
	
	mRight = mRight * cos(theta) + mLook * sin(theta);

	Math3D::normalize(mRight);
	mUp = Math3D::cross(mRight, mLook);
	mLook = -mLook;

	buildViewMtx();
}

void Camera::pitch(float theta) {
	theta = DegToRad(theta);
	mLook = mLook * cos(theta) + mUp * sin(theta);

	Math3D::normalize(mLook);
	mRight = Math3D::cross(mLook, mUp);

	buildViewMtx();
}

void Camera::rotate(f32 yaw, f32 pitch) {
	yaw = DegToRad(yaw);
	pitch = DegToRad(pitch);

	Matrix34 R = Math3D::matrixRotationAxis(mRight, pitch);
	mLook = R * mLook;
	mUp = R * mUp;

	R = Math3D::matrixRotationY(yaw);
	mRight = R * mRight;
	mUp = R * mUp;
	mLook = R * mLook;
}

void Camera::buildViewMtx() {
	f32 x = Math3D::dot(mPosition, mRight);
	f32 y = Math3D::dot(mPosition, mUp);
	f32 z = Math3D::dot(mPosition, mLook);

	viewMatrix[0][0] = mRight.x;
	viewMatrix[0][1] = mRight.y;
	viewMatrix[0][2] = mRight.z;
	viewMatrix[0][3] = -x;

	viewMatrix[1][0] = mUp.x;
	viewMatrix[1][1] = mUp.y;
	viewMatrix[1][2] = mUp.z;
	viewMatrix[1][3] = -y;

	viewMatrix[2][0] = -mLook.x;
	viewMatrix[2][1] = -mLook.y;
	viewMatrix[2][2] = -mLook.z;
	viewMatrix[2][3] = z;
}

Vec3& Camera::position() {
	return mPosition;
}


void Camera::setPosition(Vec3 pos) {
	mPosition = pos;
}

Vec3& Camera::forward() {
	return mLook;
}

Vec3& Camera::right() {
	return mRight;
}

Vec3& Camera::up() {
	return mUp;
}

void Camera::setForward(Vec3 f) {
	mLook = f;
}

void Camera::moveForward(f32 delta) {
	Vec3 velocity = mLook;

	mPosition += velocity;
}

void Camera::moveRight(f32 delta) {
	Vec3 velocity = mRight;
	velocity *= delta;

	mPosition += velocity;
}

void Camera::moveUp(f32 delta) {
	Vec3 velocity = mUp;
	velocity *= delta;

	mPosition += velocity;
}

void Camera::getViewMtx(Mtx out_view) {
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 4; j++)
			out_view[i][j] = viewMatrix[i][j];
}

void Camera::getProjMatrix(Mtx44 out_proj) {
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			out_proj[i][j] = projMatrix[i][j];
}
