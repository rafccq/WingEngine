#ifndef _PSYSTEM_H_
#define _PSYSTEM_H_

#include <math.h>
#include <ogc/gu.h>
#include <cfloat>
#include <string>

#include "Math_func.h"
#include "foundation\array.h"
#include "foundation\memory.h"

class Camera;

enum ParticleFlags {
	ALIVE	= 1 << 0,
	GRAVITY = 1 << 1
};

// Helper classes: interpolators
template<typename T>
class SimpleInterpolator {
public:
	SimpleInterpolator(T delta) : m_Delta(delta) {};

	T getValue(T initial, f32 t) {
		T final = initial + m_Delta;
		return (1.0f-t)*initial + t*final;
	}

protected:
	T m_Delta;
};

// Specialized interpolator for colors
class ColorInterpolator {
public:
	ColorInterpolator(u32 initialColor, u32 finalColor) :
		m_InitialColor(initialColor),
		m_FinalColor(finalColor) {};

	u32 getValue(f32 t) {
		u8 r0 = (m_InitialColor & 0xFF000000) >> 24;
		u8 g0 = (m_InitialColor & 0x00FF0000) >> 16;
		u8 b0 = (m_InitialColor & 0x0000FF00) >> 8;
		u8 a0 = (m_InitialColor & 0x000000FF) >> 0;

		u8 r1 = (m_FinalColor & 0xFF000000) >> 24;
		u8 g1 = (m_FinalColor & 0x00FF0000) >> 16;
		u8 b1 = (m_FinalColor & 0x0000FF00) >> 8;
		u8 a1 = (m_FinalColor & 0x000000FF) >> 0;
		
		f32 m = 1.0f - t;
		u8 r = m*r0 + t*r1;
		u8 g = m*g0 + t*g1;
		u8 b = m*b0 + t*b1;
		u8 a = m*a0 + t*a1;

		u32 result = 0;
		result |= (r << 24);
		result |= (g << 16);
		result |= (b << 8);
		result |= (a << 0);

		return result;
	}

protected:
	u32 m_InitialColor;
	u32 m_FinalColor;
};

// helper structs used to perform bucket sort
struct Bin {
	f32 start;
	f32 end;
};

struct BinPair {
	u16 binIdx;
	u16 particleIdx;
};

#define N_BINS 600

// Particle System structure
struct ParticleSystem {
    Vec3* positions;
    Vec3* velocities;
    f32* rotations;
    
	f32* depths;

    u32* colors;
    f32* sizes;

	f32* lifetimes;
	f32* ages;

	u16 n_particles;
	GXTexObj texture;

	u8 flags;

	Bin bins[N_BINS];
	BinPair* binsAllocations;

	SimpleInterpolator<f32>*	rotationInterpolator;
	SimpleInterpolator<f32>*	sizeInterpolator;
	ColorInterpolator*			colorInterpolator;

	ParticleSystem(): 
		positions(0),	velocities(0),	rotations(0),
		colors(0),		sizes(0),		lifetimes(0),
		ages(0),		rotationInterpolator(0), 
		sizeInterpolator(0), colorInterpolator(0) {}
};

void Particle_Init(ParticleSystem* psystem, u16 n);
void Particle_SphereSpread(ParticleSystem* psystem, float maxRadius, Vec3 emitterPos);

void Particle_Update(ParticleSystem* psystem, float dt);

void Particle_Render(Mtx M_view, ParticleSystem* psystem, Camera* camera);

#endif
