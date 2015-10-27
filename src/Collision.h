#pragma once

#include <math.h>
#include <ogc/gu.h>
#include <cfloat>
#include <string>

#include "WorldData.h"
#include "Mesh.h"

typedef guVector Vec;

#define dot guVecDotProduct

struct AABB {
	Vec min;
	f32 d[3];
};

struct OBB {
	Vec c;
	Vec u[3];
	f32 e[3];
};

struct Sphere {
	Vec c;
	f32 r;
};

void BuildAABB(Mesh& mesh, AABB* out);
void TransformAABB(Mtx M, Vec T, AABB& a, AABB* ab);

void DrawAABB(MtxP M_model, Mtx M_view, AABB& box, u32 color);
void DrawOBB(Mtx M_view, OBB& box, u32 color, bool drawAxes = true);

bool TestOBBOBB(OBB& a, OBB& b);
bool TestSphereOBB(Sphere& s, OBB& box, Vec* out_p);
bool TestSphereOBB(Sphere& s, OBB& box);
void ClosestPtPointOBB(Vec& p, OBB& box, Vec* out_q);


void EulerToMtx(f32 head, f32 pitch, f32 bank, Mtx out_mtx);
void OBBFitBruteForce(f32* vertices, int n_vertices, OBB* out_box);

// temporary aux
inline guVector _vec(f32 x, f32 y, f32 z) {
	guVector v;
	v.x = x;
	v.y = y;
	v.z = z;

	return v;
}

