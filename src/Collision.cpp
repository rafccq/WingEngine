#include <gccore.h>
#include <cwchar>

#include "Collision.h"
#include "System.h"

#define SYS_LOG wprintf

void BuildAABB(Mesh& mesh, AABB* out) {
	f32* vertices = mesh.vertices;

	f32 x_extents[2] = {FLT_MAX, -FLT_MAX};
	f32 y_extents[2] = {FLT_MAX, -FLT_MAX};
	f32 z_extents[2] = {FLT_MAX, -FLT_MAX};

	u16 n = mesh.n_vertices;
	for (u16 i = 0, k = 0; i < n; i++) {
		f32 vx = vertices[k];
		f32 vy = vertices[k+1];
		f32 vz = vertices[k+2];

		x_extents[0] = vx < x_extents[0] ? vx : x_extents[0];
		x_extents[1] = vx > x_extents[1] ? vx : x_extents[1];

		y_extents[0] = vy < y_extents[0] ? vy : y_extents[0];
		y_extents[1] = vy > y_extents[1] ? vy : y_extents[1];

		z_extents[0] = vz < z_extents[0] ? vz : z_extents[0];
		z_extents[1] = vz > z_extents[1] ? vz : z_extents[1];

		k += 3;
	}

	f32 dx = x_extents[1] - x_extents[0];
	f32 dy = y_extents[1] - y_extents[0];
	f32 dz = z_extents[1] - z_extents[0];
	out->min.x = x_extents[0];
	out->min.y = y_extents[0];
	out->min.z = z_extents[0];
	out->d[0] = dx;
	out->d[1] = dy;
	out->d[2] = dz;

	//SYS_LOG(L"n_v=%d", n);
	//SYS_LOG(L"d=(%.4f, %.4f, %.4f)", dx, dy, dz);
}

void TransformAABB(Mtx M, Vec T, AABB& a, AABB* ab) {
    // Copy box A into a min array and a max array for easy reference.
	f32 Amin[3], Amax[3];
    Amin[0] = a.min.x;  Amax[0] = a.min.x + a.d[0];
    Amin[1] = a.min.y;  Amax[1] = a.min.y + a.d[1];
    Amin[2] = a.min.z;  Amax[2] = a.min.z + a.d[2];

    // Take care of translation by beginning at T.
	f32 Bmin[3], Bmax[3];
    Bmin[0] = Bmax[0] = T.x;
    Bmin[1] = Bmax[1] = T.y;
    Bmin[2] = Bmax[2] = T.z;

    /* Now find the extreme points by considering the product of the 
       min and max with each component of M.  */
    for(u8 i = 0; i < 3; i++ ) {
		for(u8 j = 0; j < 3; j++ ) {
			f32 a = M[i][j] * Amin[j];
			f32 b = M[i][j] * Amax[j];
			if( a < b ) {
				Bmin[i] += a; 
				Bmax[i] += b;
			}
			else { 
				Bmin[i] += b; 
				Bmax[i] += a;
			}
		}
	}

    // Copy the result into the new box.
    ab->min.x = Bmin[0];  ab->d[0] = Bmax[0] - Bmin[0];
    ab->min.y = Bmin[1];  ab->d[1] = Bmax[1] - Bmin[1];
    ab->min.z = Bmin[2];  ab->d[2] = Bmax[2] - Bmin[2];
} 

#define EPSILON 1e-07f

bool TestOBBOBB(OBB& a, OBB& b) {
    f32 ra, rb;
    Mtx R, AbsR;

    // Compute rotation matrix expressing b in a's coordinate frame
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            //R[i][j] = dot(&a.u[i], &b.u[j]);
            R[j][i] = dot(&a.u[i], &b.u[j]);
    
	for (int i = 0; i < 3; i++) {
		//R[i][0] = a.u[i].x * b.u[0].x + a.u[i].y * b.u[1].x + a.u[i].z * b.u[2].x;
		//R[i][1] = a.u[i].x * b.u[0].y + a.u[i].y * b.u[1].y + a.u[i].z * b.u[2].y;
		//R[i][2] = a.u[i].x * b.u[0].z + a.u[i].y * b.u[1].z + a.u[i].z * b.u[2].z;

		//R[i][0] = b.u[i].x * a.u[0].x + b.u[i].y * a.u[1].x + b.u[i].z * a.u[2].x;
		//R[i][1] = b.u[i].x * a.u[0].y + b.u[i].y * a.u[1].y + b.u[i].z * a.u[2].y;
		//R[i][2] = b.u[i].x * a.u[0].z + b.u[i].y * a.u[1].z + b.u[i].z * a.u[2].z;
	}

	SYS_LOG(L"R={ %.3f, %.3f, %.3f",  R[0][0], R[0][1], R[0][2]);
	SYS_LOG(L"    %.3f, %.3f, %.3f",  R[1][0], R[1][1], R[1][2]);
	SYS_LOG(L"    %.3f, %.3f, %.3f}", R[2][0], R[2][1], R[2][2]);

	guMtxTranspose(R, R);
    // Compute translation vector t
    //Vector t = b.c - a.c;
	Vec _t;
	_t.x = b.c.x - a.c.x;
	_t.y = b.c.y - a.c.y;
	_t.z = b.c.z - a.c.z;
    // Bring translation into a's coordinate frame
	f32 t[3];

    t[0] = _t.x*a.u[0].x + _t.y*a.u[0].y + _t.z*a.u[0].z;
    t[1] = _t.x*a.u[1].x + _t.y*a.u[1].y + _t.z*a.u[1].z;
    t[2] = _t.x*a.u[2].x + _t.y*a.u[2].y + _t.z*a.u[2].z;

    
	// Compute common subexpressions. Add in an epsilon term to counteract arithmetic errors when two edges are parallel and
    // their cross product is (near) null (see text for details)
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            AbsR[i][j] = fabs(R[i][j]) + EPSILON;

	SYS_LOG(L"abs(R)={ %.3f, %.3f, %.3f",  AbsR[0][0], AbsR[0][1], AbsR[0][2]);
	SYS_LOG(L"         %.3f, %.3f, %.3f",  AbsR[1][0], AbsR[1][1], AbsR[1][2]);
	SYS_LOG(L"         %.3f, %.3f, %.3f}", AbsR[2][0], AbsR[2][1], AbsR[2][2]);

    // Test axes L = A0, L = A1, L = A2
    for (int i = 0; i < 3; i++) {
        ra = a.e[i];
        rb = b.e[0] * AbsR[i][0] + b.e[1] * AbsR[i][1] + b.e[2] * AbsR[i][2];
		SYS_LOG(L":A%i, t={%.3f, %.3f, %.3f}, ra=%.3f, rb=%.3f", i, t[0], t[1], t[2], ra, rb);
        if (fabs(t[i]) > ra + rb) return 0;
    }
	
    // Test axes L = B0, L = B1, L = B2
    for (int i = 0; i < 3; i++) {
        ra = a.e[0] * AbsR[0][i] + a.e[1] * AbsR[1][i] + a.e[2] * AbsR[2][i];
        rb = b.e[i];
		SYS_LOG(L":B%i", i);
        if (fabs(t[0] * R[0][i] + t[1] * R[1][i] + t[2] * R[2][i]) > ra + rb) return 0;
    }

    // Test axis L = A0 x B0
	SYS_LOG(L":A0 x B0");
    ra = a.e[1] * AbsR[2][0] + a.e[2] * AbsR[1][0];
    rb = b.e[1] * AbsR[0][2] + b.e[2] * AbsR[0][1];
    if (fabs(t[2] * R[1][0] - t[1] * R[2][0]) > ra + rb) return 0;

    // Test axis L = A0 x B1
	SYS_LOG(L":A0 x B1");
    ra = a.e[1] * AbsR[2][1] + a.e[2] * AbsR[1][1];
    rb = b.e[0] * AbsR[0][2] + b.e[2] * AbsR[0][0];
    if (fabs(t[2] * R[1][1] - t[1] * R[2][1]) > ra + rb) return 0;

    // Test axis L = A0 x B2
	SYS_LOG(L":A0 x B2");
    ra = a.e[1] * AbsR[2][2] + a.e[2] * AbsR[1][2];
    rb = b.e[0] * AbsR[0][1] + b.e[1] * AbsR[0][0];
    if (fabs(t[2] * R[1][2] - t[1] * R[2][2]) > ra + rb) return 0;

    // Test axis L = A1 x B0
	SYS_LOG(L":A1 x B0");
    ra = a.e[0] * AbsR[2][0] + a.e[2] * AbsR[0][0];
    rb = b.e[1] * AbsR[1][2] + b.e[2] * AbsR[1][1];
    if (fabs(t[0] * R[2][0] - t[2] * R[0][0]) > ra + rb) return 0;

    // Test axis L = A1 x B1
	SYS_LOG(L":A1 x B1");
    ra = a.e[0] * AbsR[2][1] + a.e[2] * AbsR[0][1];
    rb = b.e[0] * AbsR[1][2] + b.e[2] * AbsR[1][0];
    if (fabs(t[0] * R[2][1] - t[2] * R[0][1]) > ra + rb) return 0;

    // Test axis L = A1 x B2
	SYS_LOG(L":A1 x B2");
    ra = a.e[0] * AbsR[2][2] + a.e[2] * AbsR[0][2];
    rb = b.e[0] * AbsR[1][1] + b.e[1] * AbsR[1][0];
    if (fabs(t[0] * R[2][2] - t[2] * R[0][2]) > ra + rb) return 0;

    // Test axis L = A2 x B0
	SYS_LOG(L":A2 x B0");
    ra = a.e[0] * AbsR[1][0] + a.e[1] * AbsR[0][0];
    rb = b.e[1] * AbsR[2][2] + b.e[2] * AbsR[2][1];
    if (fabs(t[1] * R[0][0] - t[0] * R[1][0]) > ra + rb) return 0;

    // Test axis L = A2 x B1
	SYS_LOG(L":A2 x B1");
    ra = a.e[0] * AbsR[1][1] + a.e[1] * AbsR[0][1];
    rb = b.e[0] * AbsR[2][2] + b.e[2] * AbsR[2][0];
    if (fabs(t[1] * R[0][1] - t[0] * R[1][1]) > ra + rb) return 0;

    // Test axis L = A2 x B2
	SYS_LOG(L":A2 x B2");
    ra = a.e[0] * AbsR[1][2] + a.e[1] * AbsR[0][2];
    rb = b.e[0] * AbsR[2][1] + b.e[1] * AbsR[2][0];
    if (fabs(t[1] * R[0][2] - t[0] * R[1][2]) > ra + rb) return 0;

    // Since no separating axis found, the OBBs must be intersecting
	SYS_LOG(L":INTERSECTION");
    return 1;
}

void SendVertex(guVector& p, u32 color) {
	GX_Position3f32(p.x, p.y, p.z);
	GX_Color1u32(color);
}

void DrawAABB(MtxP M_model, Mtx M_view, AABB& box, u32 color) {
	GX_ClearVtxDesc();

	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);

    GX_SetVtxAttrFmt(GX_VTXFMT5, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT5, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);

	// build and load model-view matrix
	Mtx M_modelView;
	guMtxIdentity(M_modelView);
	guMtxConcat(M_view, M_model, M_modelView);
	GX_LoadPosMtxImm(M_modelView, GX_PNMTX0);

	GX_SetNumChans(1); // default, color = vertex color
	//GX_SetNumTexGens(0);

    GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);

	u8 indices[8] = {0, 1, 0, 2, 1, 3, 2, 3};
	guVector& m = box.min;
	f32* d = box.d;

	guVector v_bottom[4];
	v_bottom[0] = _vec(m.x,			m.y,	m.z);	
	v_bottom[1] = _vec(m.x + d[0],	m.y,	m.z);
	v_bottom[2] = _vec(m.x,			m.y,	m.z + d[2]);
	v_bottom[3] = _vec(m.x + d[0],	m.y,	m.z + d[2]);

	guVector v_top[4];
	for (int i = 0; i < 4; i++)
		v_top[i] = _vec(v_bottom[i].x, v_bottom[i].y + d[1], v_bottom[i].z);

	GX_Begin(GX_LINES, GX_VTXFMT5, 24);
		// draw bottom
		for (u8 i = 0; i < 8; i += 2) {
			guVector& v0 = v_bottom[indices[i]];
			guVector& v1 = v_bottom[indices[i+1]];

			SendVertex(v0, color);
			SendVertex(v1, color);
		}

		// draw top
		for (u8 i = 0; i < 8; i += 2) {
			guVector& v0 = v_top[indices[i]];
			guVector& v1 = v_top[indices[i+1]];

			SendVertex(v0, color);
			SendVertex(v1, color);
		}

		// connect bottom vertices with the top ones
		for (u8 i = 0; i < 4; i++) {
			guVector& v0 = v_bottom[i];
			guVector& v1 = v_top[i];

			SendVertex(v0, color);
			SendVertex(v1, color);
		}
	GX_End();
}

void DrawOBB(Mtx M_view, OBB& box, u32 color, bool drawAxes) {
	//for (int i = 0; i < 3; i++)
		//SYS_LOG(L"u%i={%.3f, %.3f, %.3f}", i, box.u[i].x, box.u[i].y, box.u[i].z);
	//SYS_LOG(L"color=%x", color);
	GX_ClearVtxDesc();

	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);

    GX_SetVtxAttrFmt(GX_VTXFMT5, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT5, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);

	// build and load model-view matrix
	Mtx M_modelView;
	guMtxIdentity(M_modelView);
	
	// rotation
	M_modelView[0][0] = box.u[0].x;		M_modelView[0][1] = box.u[0].y;		M_modelView[0][2] = box.u[0].z;
	M_modelView[1][0] = box.u[1].x;		M_modelView[1][1] = box.u[1].y;		M_modelView[1][2] = box.u[1].z;
	M_modelView[2][0] = box.u[2].x;		M_modelView[2][1] = box.u[2].y;		M_modelView[2][2] = box.u[2].z;
	
	// translation
	M_modelView[0][3] = box.c.x;		M_modelView[1][3] = box.c.y;		M_modelView[2][3] = box.c.z;

	guMtxConcat(M_view, M_modelView, M_modelView);
	GX_LoadPosMtxImm(M_modelView, GX_PNMTX0);

	GX_SetNumChans(1); // default, color = vertex color
	//GX_SetNumTexGens(0);

    GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);

	u8 indices[8] = {0, 1, 0, 2, 1, 3, 2, 3};
	f32* e = box.e;

	guVector v_bottom[4];
	v_bottom[0] = _vec(-e[0], -e[1], +e[2]);
	v_bottom[1] = _vec(+e[0], -e[1], +e[2]);
	v_bottom[2] = _vec(-e[0], -e[1], -e[2]);
	v_bottom[3] = _vec(+e[0], -e[1], -e[2]);

	guVector v_top[4];
	for (int i = 0; i < 4; i++)
		v_top[i] = _vec(v_bottom[i].x, e[1], v_bottom[i].z);

	GX_Begin(GX_LINES, GX_VTXFMT5, 24);
		// draw bottom
		for (u8 i = 0; i < 8; i += 2) {
			guVector& v0 = v_bottom[indices[i]];
			guVector& v1 = v_bottom[indices[i+1]];

			SendVertex(v0, color);
			SendVertex(v1, color);
		}

		// draw top
		for (u8 i = 0; i < 8; i += 2) {
			guVector& v0 = v_top[indices[i]];
			guVector& v1 = v_top[indices[i+1]];

			SendVertex(v0, color);
			SendVertex(v1, color);
		}

		// connect bottom vertices with the top ones
		for (u8 i = 0; i < 4; i++) {
			guVector& v0 = v_bottom[i];
			guVector& v1 = v_top[i];

			SendVertex(v0, color);
			SendVertex(v1, color);
		}
	GX_End();

	if (!drawAxes)
		return;

	// axes
	const f32 len = 150;
	GX_Begin(GX_LINES, GX_VTXFMT5, 6);
		guVector zero	= {0, 0, 0};
		guVector x_unit = {len, 0, 0};
		guVector y_unit = {0, len, 0};
		guVector z_unit = {0, 0, len};

		color = 0xFF0000FF;
		SendVertex(zero, color);
		SendVertex(x_unit, color);

		color = 0x00FF00FF;
		SendVertex(zero, color);
		SendVertex(y_unit, color);

		color = 0x0000FFFF;
		SendVertex(zero, color);
		SendVertex(z_unit, color);
	GX_End();
}

// Given point p, return point q on (or in) OBB b, closest to p
void ClosestPtPointOBB(Vec& p, OBB& box, Vec* out_q) {
    //Vector d = p – b.c;
	Vec d;
	guVecSub(&p, &box.c, &d);
    // Start result at center of box; make steps from there
	Vec q = box.c;
    // For each OBB axis...
    for (int i = 0; i < 3; i++) {
		// ...project d onto that axis to get the distance along the axis of d from the box center
		f32 dist = dot(&d, &box.u[i]);
		// If distance farther than the box extents, clamp to the box
		if (dist >  box.e[i]) dist =  box.e[i];
		if (dist < -box.e[i]) dist = -box.e[i];
		// Step that distance along the axis to get world coordinate
		//q += dist * b.u[i];
		q.x += dist * box.u[i].x;
		q.y += dist * box.u[i].y;
		q.z += dist * box.u[i].z;
    }

	*out_q = q;
}

// Returns true if sphere s intersects OBB b, false otherwise.
// The point p on the OBB closest to the sphere center is also returned
bool TestSphereOBB(Sphere& s, OBB& box, Vec* out_p) {
    // Find point p on OBB closest to sphere center
    ClosestPtPointOBB(s.c, box, out_p);

    // Sphere and OBB intersect if the (squared) distance from sphere
    // center to point p is less than the (squared) sphere radius
    //Vec v = p - s.c;
	Vec v ;
	guVecSub(out_p, &s.c, &v);
    return dot(&v, &v) <= s.r * s.r;
}

bool TestSphereOBB(Sphere& s, OBB& box) {
	Vec p;
    // Find point p on OBB closest to sphere center
    ClosestPtPointOBB(s.c, box, &p);

    // Sphere and OBB intersect if the (squared) distance from sphere
    // center to point p is less than the (squared) sphere radius
    //Vec v = p - s.c;
	Vec v;
	guVecSub(&p, &s.c, &v);
    return dot(&v, &v) <= s.r * s.r;
}

void EulerToMtx(f32 rotX, f32 rotY, f32 rotZ, Mtx out_M) {
	f32 sinX  = sin(rotX);		f32 cosX  = cos(rotX);
	f32 sinY  = sin(rotY);		f32 cosY  = cos(rotY);
	f32 sinZ  = sin(rotZ);		f32 cosZ  = cos(rotZ);

	out_M[0][0] =  cosY*cosZ + sinY*sinX*sinZ;
	out_M[1][0] = -cosY*sinZ + sinY*sinX*cosZ;
	out_M[2][0] =  sinY*cosX;

	out_M[0][1] =  sinZ*cosX;
	out_M[1][1] =  cosZ*cosX;
	out_M[2][1] = -sinX;

	out_M[0][2] = -sinY*cosZ + cosY*sinX*sinZ;
	out_M[1][2] =  sinZ*sinY + cosY*sinX*cosZ;
	out_M[2][2] =  cosY*cosX;
}

void OBBFitBruteForce(f32* vertices, int n_vertices, OBB* out_box) {
	f32 delta = M_PI/6;

	int n = 3*n_vertices;
	f32 extents0[2] = {FLT_MAX, -FLT_MAX};
	f32 extents1[2] = {FLT_MAX, -FLT_MAX};
	f32 extents2[2] = {FLT_MAX, -FLT_MAX};

	f32 bestVolume = FLT_MAX;
	for (f32 rot_x = -M_PI; rot_x < M_PI; rot_x += delta)
		for (f32 rot_y = -M_PI; rot_y < M_PI; rot_y += delta) {
			for (f32 rot_z = -M_PI; rot_z < M_PI; rot_z += delta) {
				Mtx M;
				EulerToMtx(rot_x, rot_y, rot_z, M);

				Vec u0 = {M[0][0], M[0][1], M[0][2]};
				Vec u1 = {M[1][0], M[1][1], M[1][2]};
				Vec u2 = {M[2][0], M[2][1], M[2][2]};

				for (int i = 0; i < n; i += 3) {
					Vec v_i = {vertices[i], vertices[i+1], vertices[i+2]};
					f32 proj0 = dot(&v_i, &u0);
					if (proj0 < extents0[0]) extents0[0] = proj0;
					if (proj0 > extents0[1]) extents0[1] = proj0;

					f32 proj1 = dot(&v_i, &u1); 
					if (proj1 < extents1[0]) extents1[0] = proj1;
					if (proj1 > extents1[1]) extents1[1] = proj1;

					f32 proj2 = dot(&v_i, &u2);
					if (proj2 < extents2[0]) extents2[0] = proj2;
					if (proj2 > extents2[1]) extents2[1] = proj2;
				}

				// compute best box
				f32 volume = (extents0[1] - extents0[0]) * (extents1[1] - extents1[0]) * (extents2[1] - extents2[0]);
				if (volume < bestVolume) {
					bestVolume = volume;
					out_box->u[0] = u0;
					out_box->u[1] = u1;
					out_box->u[2] = u2;

					out_box->e[0] = (extents0[1] - extents0[0]) * 0.5f;
					out_box->e[1] = (extents1[1] - extents1[0]) * 0.5f;
					out_box->e[2] = (extents2[1] - extents2[0]) * 0.5f;
				}
			}
		}
}