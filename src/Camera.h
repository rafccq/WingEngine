#pragma once

#include <gccore.h>
#include "Math_types.h"

class Camera {
public:
	Camera();

	void setProj(f32 fov, f32 aspect, f32 near, f32 far);
	void yaw(float theta);
	void pitch(float theta);
	void buildViewMtx();
	
	Vec3& position();
	void setPosition(Vec3 pos);
	void moveForward(f32 delta);

	Vec3& forward();
	Vec3& right();
	Vec3& up();
	void setForward(Vec3 f);

	void moveUp(f32 delta);
	void moveRight(f32 delta);

	void rotate(f32 yaw, f32 pitch);

	void getViewMtx(Mtx out_view);
	void getProjMatrix(Mtx44 out_proj);

private:
	Vec3 mPosition;

	Vec3 mUp;
	Vec3 mRight;
	Vec3 mLook;

	Mtx44 projMatrix;
	Mtx viewMatrix;
};

