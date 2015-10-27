#include <math.h>

#include "Math_types.h"
#include "SkyBox.h"
#include "Renderer.h"
#include "MeshReader.h"
#include "Material.h"

f32 posCoords[] ATTRIBUTE_ALIGN(32) = 
{
	// Top (+y)
	-1.0, 0, +1.0,	// 0
	+1.0, 0, +1.0,	// 1
	+1.0, 0, -1.0,	// 2
	-1.0, 0, -1.0,	// 3

	// Right (+x)
	0, +1.0, -1.0,	// 4
	0, +1.0, +1.0,	// 5
	0, -1.0, +1.0,	// 6
	0, -1.0, -1.0,	// 7

	// Bottom (-y)
	-1.0, 0, -1.0,	// 8
	+1.0, 0, -1.0,	// 9
	+1.0, 0, +1.0,	// 10
	-1.0, 0, +1.0,	// 11

	// Left (-x)
	0, +1.0, +1.0,	// 12
	0, +1.0, -1.0,	// 13
	0, -1.0, -1.0,	// 14
	0, -1.0, +1.0,	// 15

	// Back (+z)
	+1.0, +1.0, 0,	// 16
	-1.0, +1.0, 0,	// 17
	-1.0, -1.0, 0,	// 18
	+1.0, -1.0, 0,	// 19

	// Front (-z)
	-1.0, +1.0, 0,	// 20
	+1.0, +1.0, 0,	// 21
	+1.0, -1.0, 0,	// 22
	-1.0, -1.0, 0	// 23
};

f32 U = 1.0f/6.0f;
f32 texCoords[] ATTRIBUTE_ALIGN(32) = {
	0, 0,
	U, 0,
	U, 1,
	1, 1
};

SkyBox::SkyBox() {

}

void SkyBox::init(Renderer* renderer, const char* textureFileName) {
	mRenderer = renderer;
	WorldData& worldData = mRenderer->getWorldData();
	TextureLibrary& textureLibrary = worldData.getTextureLibrary();
	u32 texID = texture_read(textureFileName, textureLibrary);
	mTexture = &textureLibrary.data[texID];

	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
}

void SkyBox::render(Mtx view) {
	// setup TEV
	GX_SetNumChans(1);
	GX_SetNumTexGens(1);

	GX_SetTevOp(GX_TEVSTAGE0, GX_REPLACE);
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);

	GX_ClearVtxDesc();

	GX_SetVtxDesc(GX_VA_POS, GX_INDEX8);
	GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);

    GX_SetArray(GX_VA_POS, posCoords, 3*sizeof(f32));

	// Load up the texture
	GX_LoadTexObj(&mTexture->texObj, GX_TEXMAP0);

	GX_SetZMode(GX_FALSE, GX_LEQUAL, GX_FALSE);

	f32 u = 0;
	f32 du = 1.0/6.0f;
	f32 s = 4000.0f;

	Mtx S;
	guMtxScale(S, s, s, s);

	f32 translations[] = {
		+0, +s, +0,	// +y
		+s, +0, +0,	// +x
		+0, -s, +0,	// -y
		-s, +0, +0,	// -x
		+0, +0, +s,	// +z
		+0, +0, -s,	// -z
	};

	// get camera position
 	Camera& cam = mRenderer->getCamera(0);
	Vec3 pos = cam.position();

	for (int i = 0, t = 0, idx = 0; i < 6; i++) {
		Mtx T, modelView;

		f32 tx = pos.x + translations[t];
		f32 ty = pos.y + translations[t+1];
		f32 tz = pos.z + translations[t+2];
		guMtxTrans(T, tx, ty, tz);

		guMtxConcat(T, S, modelView);
		guMtxConcat(view, modelView, modelView);
		GX_LoadPosMtxImm(modelView, GX_PNMTX0);

		GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
			GX_Position1x8(idx);
			GX_TexCoord2f32(u, 0);

			GX_Position1x8(idx+1);
			GX_TexCoord2f32(u+du, 0);

			GX_Position1x8(idx+2);
			GX_TexCoord2f32(u+du, 1);

			GX_Position1x8(idx+3);
			GX_TexCoord2f32(u, 1);
		GX_End();

		t += 3;
		idx += 4;
		u += du;
	}

	GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
}

