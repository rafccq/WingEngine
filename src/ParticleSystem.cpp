#include <gccore.h>
#include <malloc.h>
#include <wiiuse/wpad.h>

#include "Math_func.h"
#include "ParticleSystem.h"
#include "System.h"
#include "Camera.h"
#include "Random.h"

f32* buffer ATTRIBUTE_ALIGN(32);

f32 buffer2[] ATTRIBUTE_ALIGN(32) = {
	-15.0f, 200.0f, 215.0f,
	-15.0f, 200.0f, 185.0f,
	+15.0f, 200.0f, 185.0f,
	+15.0f, 200.0f, 215.0f
};


void Particle_Init(ParticleSystem* psystem, u16 n) {
	psystem->n_particles = n;

	psystem->positions = new Vec3[n];
	psystem->velocities = new Vec3[n];
	psystem->rotations = new f32[n];
	psystem->colors = new u32[n];
	psystem->sizes = new f32[n];
	psystem->lifetimes = new f32[n];
	psystem->ages = new f32[n];
	psystem->binsAllocations = new BinPair[n];
	
	// test
	psystem->depths = new f32[n];

	for (int i = 0; i < n; i++) {
		psystem->binsAllocations[i].particleIdx = i;
	}

	// init bins
	int i;
	u16 n_1stRange = 500;
	f32 firstRange = 300;
	f32 delta = firstRange/n_1stRange;
	for (i = 0; i < n_1stRange; i++) {
		psystem->bins[i].start = i*delta;
		psystem->bins[i].end = psystem->bins[i].start + delta;
	}

	u16 n_2stRange = 100;
	u16 total = n_1stRange + n_2stRange;
	f32 lastRange = firstRange + 300;
	delta = (lastRange-firstRange)/n_2stRange;
	for (int k = 0; i < total; i++, k++) {
		psystem->bins[i].start = firstRange + k*delta;
		psystem->bins[i].end = psystem->bins[i].start + delta;
	}
	
	psystem->bins[total-1].end = 1000000.0f;
}

void Particle_SphereSpread(ParticleSystem* psystem, float maxRadius, Vec3 emitterPos) {
	u16 n = psystem->n_particles;

	for (u16 i = 0; i < n; i++) {
		float altitude = RandRange(0, M_PI);
		float azimuth = RandRange(0, 2*M_PI);
		float radius = RandRange(0.1f, maxRadius);

		float sinAlt = sin(altitude);

		float x = sinAlt * cos(azimuth);
		float y = sinAlt * sin(azimuth);
		float z = cos(altitude);

		Vec3 pointOnSphere(x, y, z);
		Vec3 pos = pointOnSphere*radius + emitterPos;

		f32 speed = RandRange(3, 6);
		Vec3 vel = pointOnSphere * speed;

		psystem->positions[i] = pos;
		psystem->velocities[i] = vel;
	}
}

void Particle_Update(ParticleSystem* psystem, f32 dt) {
	u16 n = psystem->n_particles;

	for (u16 i = 0; i < n; i++) {
		Vec3& pos = psystem->positions[i];
		Vec3& vel = psystem->velocities[i];

		Vec3 force(0, -9.81f*0.0005f*dt, 0);
		vel += force;

		pos += vel;
		psystem->ages[i] += dt;
	}
}

void SendVertex(Vec3& p, u32 color) {
	GX_Position3f32(p.x, p.y, p.z);
	GX_Color1u32(color);
}

void SendVertex(Vec3& p, u32 color, f32 u, f32 v) {
	GX_Position3f32(p.x, p.y, p.z);
	GX_Color1u32(color);
	GX_TexCoord2f32(u, v);
}

void SendVertex_i(u16 index, u32 color, ParticleSystem* psystem) {
    GX_Position1x16(index);
	GX_Color1u32(color);

	f32* v = buffer + 3*index;
	System::Log(L"SEND_VERTEX %d:(%.3f, %.3f, %.3f)", index,	v[0], v[1], v[2]);
}

int compareFunc(const void* _a, const void* _b) {
	const BinPair* a = (const BinPair*) _a;
	const BinPair* b = (const BinPair*) _b;

	return (a->binIdx - b->binIdx);
}
inline void swap(u16& a, u16& b) {
	a = a ^ b;
	b = a ^ b;
	a = a ^ b;
}

void Particle_Sort(Mtx M_view, ParticleSystem* psystem) {
	int n = psystem->n_particles;
	System::LogClear();


	for (int i = 0; i < n; i++) {
		Vec3& p = psystem->positions[i];
		Vec3 p_view = Math3D::matrixVecMul(M_view, p);
		psystem->depths[i] = p_view.z;
	}

	for (int i = 0; i < n; i++) {
		Vec3& p = psystem->positions[i];
		Vec3 p_view = Math3D::matrixVecMul(M_view, p);
		f32 depth = p_view.z;
		for (u16 b = 0; b < N_BINS; b++) {
			Bin& bin = psystem->bins[b];
			if (depth > bin.start && depth < bin.end) {
				psystem->binsAllocations[i].binIdx = b;
				psystem->binsAllocations[i].particleIdx = i;
				break;
			}
		}
	}
	qsort(psystem->binsAllocations, n, sizeof(BinPair), compareFunc);
}

void Particle_Render(Mtx M_view, ParticleSystem* psystem, Camera* camera) {
	// setup TEV
	GX_ClearVtxDesc();
	
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);

	GX_SetVtxAttrFmt(GX_VTXFMT6, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT6, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT6, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

	// load model-view matrix
	Mtx M_modelView;
	guMtxIdentity(M_modelView);
	GX_LoadPosMtxImm(M_modelView, GX_PNMTX0);

	// setup tev
	GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
	GX_SetAlphaUpdate(GX_TRUE);

	GX_SetNumChans(1);

	GX_SetTevColorIn( 
		GX_TEVSTAGE0, 
		GX_CC_TEXC,  // a
		GX_CC_RASC,  // b
		GX_CC_TEXA,  // c 
		GX_CC_ZERO); // d

	GX_SetTevColorOp(
		GX_TEVSTAGE0,	// stage
		GX_TEV_ADD,		// op
		GX_TB_ZERO,		// bias
		GX_CS_SCALE_2,	// scale
		GX_ENABLE,		// clamp 0-255
		GX_TEVPREV);	// output reg


	GX_SetTevAlphaIn(
		GX_TEVSTAGE0, 
		GX_CA_ZERO,  // a
		GX_CA_TEXA,  // b
		GX_CA_RASA,  // c 
		GX_CA_ZERO); // d

	GX_SetTevAlphaOp(
		GX_TEVSTAGE0,	// stage
		GX_TEV_ADD,		// op
		GX_TB_ZERO,		// bias
		GX_CS_SCALE_1,	// scale
		GX_ENABLE,		// clamp 0-255
		GX_TEVPREV);	// output reg



	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);

	GX_SetNumTexGens(1);
	GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);

	GX_LoadTexObj(&psystem->texture, GX_TEXMAP0);

	int n = psystem->n_particles;

	f32 texCoords[] = {
		0, 0,
		0, 1,
		1, 1,
		1, 0
	};

	GX_InvVtxCache();
	Vec3 camPos = camera->position();

	Mtx axisMtx;
	
	System::LogClear();
	
	Particle_Sort(M_view, psystem);

	GX_Begin(GX_QUADS, GX_VTXFMT6, n*4);
		for (int k = 0; k < n; k++) {
			int i = psystem->binsAllocations[k].particleIdx;
			//System::Log(L"p%d", i);
			Vec3& p  = psystem->positions[i];
			f32 lifetime = psystem->lifetimes[i];
			f32 age = psystem->ages[i];

			f32 ratio = age/lifetime;

			Vec3 forward = Math3D::normalized(camPos - p);

			Vec3 worldUp = Vec3(0, 1, 0);
			Vec3 right = Math3D::normalized(Math3D::cross(worldUp, forward));

			Vec3 up = Math3D::cross(forward, right);

			Mtx pm;

			guMtxCopy(M_view, pm);
			guMtxTranspose(pm, pm);
			pm[0][3] = p.x;		pm[1][3] = p.y;		pm[2][3] = p.z;

			guMtxConcat(M_view, pm, pm);
			
			f32 rotation = psystem->rotations[i];
			if (psystem->rotationInterpolator)
				rotation = psystem->rotationInterpolator->getValue(rotation, ratio);

			f32 size = psystem->sizes[i];
			if (psystem->sizeInterpolator)
				size = psystem->sizeInterpolator->getValue(size, ratio);

			if (psystem->colorInterpolator)
				psystem->colors[i] = psystem->colorInterpolator->getValue(ratio);

			Matrix34 M_rot = Math3D::matrixRotationZ(rotation);
			f32 hs = size*0.5f;
			Vec3 p0(-hs, +hs, 0);
			Vec3 p1(-hs, -hs, 0);
			Vec3 p2(+hs, -hs, 0);
			Vec3 p3(+hs, +hs, 0);

			Vec3 vs[4] = {p0, p1, p2, p3};

			for (int i = 0; i < 4; i++) {
				vs[i] = M_rot * vs[i];
				vs[i] = Math3D::matrixVecMul(pm, vs[i]);
			}
			
			for (int i = 0; i < 4; i++) {
				f32* coords = &texCoords[2*i];
				SendVertex(vs[i], psystem->colors[i], coords[0], coords[1]);
			}
		}
	GX_End();


	return;
}
