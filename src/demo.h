#pragma once

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <fat.h>
#include <pngu.h>


#define WHITE {0xFF, 0xFF, 0xFF, 0xFF}
#define RED {0xFF, 0, 0, 0xFF}
#define GREEN {0x0, 0xFF, 0, 0x00}
#define BLUE {0x0, 0, 0xFF, 0xFF}

class Renderer;

//GXRModeObj* DEMOGetRenderModeObj();
//
//void DEMOBeforeRender();
//
//void DEMODoneRender();

bool loadPngImage(char* filename, int* outWidth, int* outHeight, bool* outHasAlpha, char** outData);

bool DEMOReadJoystickXY(f32* vx, f32* vy, f32 scaleX, f32 scaleY);
bool DEMOReadJoystick(f32* sx, f32* sy, f32* subx, f32* suby, f32 scale, u16 nunchuck_button);
bool DEMOCheckExtension(u8 extensionID);
u32 DEMOPadGetButtonDown(int channel);

f32 Clamp(f32 val, f32 min, f32 max);
f32 Max(f32 val1, f32 val2);

// Sphere attributes 
#define OBJ_NORMAL 1
#define OBJ_TEXTURE 2
void GX_DrawSphere(float x, float y, float z, float width, float height, int sides, int mode);

// some fixes, to avoid annoying renaming
//#define GX_TEXMAP_NULL GX_TEXMAP_NULL 
#define GX_TEXCOORD_NULL GX_TEXCOORDNULL 
#define GX_COLOR_NULL GX_COLORNULL
#define GX_LIGHT_NULL GX_LIGHTNULL

// texture header
typedef struct _tplimgheader TPLImgHeader;

struct _tplimgheader {
	u16 height;
	u16 width;
	u32 fmt;
	void *data;
	u32 wraps;
	u32 wrapt;
	u32 minfilter;
	u32 magfilter;
	f32 lodbias;
	u8 edgelod;
	u8 minlod;
	u8 maxlod;
	u8 unpacked;
} ATTRIBUTE_PACKED;

// texture palette header
typedef struct _tplpalheader TPLPalHeader;

struct _tplpalheader {
	u16 nitems;
	u8 unpacked;
	u8 pad;
	u32 fmt;
	void *data;
} ATTRIBUTE_PACKED;

// texture descriptor
typedef struct _tpldesc TPLDescHeader;

struct _tpldesc {
	TPLImgHeader *imghead;
	TPLPalHeader *palhead;
} ATTRIBUTE_PACKED;

void GXDrawCylinder(u8 sides);
void GXDrawCube();

void loadTexture(const char* filename, GXTexObj* texObj);

