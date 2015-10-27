#include <cstdlib>
#include <cwchar>
#include <cstdio>
#include <cstring>
#include <malloc.h>
#include <ogc\gu.h>
#include <ogc\lwp_watchdog.h>
#include "FreeTypeGX.h"

#include "Math_types.h"
#include "Renderer.h"
#include "Mesh.h"
#include "Utils.h"
#include "System.h"
#include "Terrain.h"

#include <cstdarg>

#define DEFAULT_FIFO_SIZE (256 * 1024)

Terrain terrain;

void Renderer::init() {
	setRenderMode(NULL);
	initGX();
	initVideo();

	mConsole.init(this);
	createCamera(45.0f, 0.1f, 5000.0f);
}

Renderer::~Renderer() {
}

u16 Renderer::getFBwidth() {
	return render_mode->fbWidth;
}

u16 Renderer::getFBheight() {
	return render_mode->xfbHeight;
}

void Renderer::initGX() {
	fb = 0;
	first_frame = 1;

	// allocate the fifo buffer
	gx_fifo = (GXFifoObj*) memalign(32, DEFAULT_FIFO_SIZE);
	memset(gx_fifo, 0, DEFAULT_FIFO_SIZE);

	frame_buffer[0] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(render_mode));
	frame_buffer[1] = SYS_AllocateFramebuffer(render_mode);

	// ====== Initialise the console, required for printf
	console_init(frame_buffer[0], 20, 20, render_mode->fbWidth, render_mode->xfbHeight, render_mode->fbWidth*VI_DISPLAY_PIX_SZ);

	// init the flipper
	GX_Init(gx_fifo, DEFAULT_FIFO_SIZE);
 
	GXColor bgColor = {0xAC, 0xAC, 0xAC, 0xFF};
	//GXColor bgColor = {0xFF, 0xFF, 0xFF, 0xFF};
	//GXColor bgColor = {0x00, 0x00, 0x55, 0xFF};
	// clears the bg to color and clears the z buffer
	GX_SetCopyClear(bgColor, 0x00FFFFFF);
 
	// other gx setup
	GX_SetViewport(0, 0, render_mode->fbWidth, render_mode->efbHeight, 0, 1);
	f32 yscale = GX_GetYScaleFactor(render_mode->efbHeight,render_mode->xfbHeight);
	u32 xfbHeight = GX_SetDispCopyYScale(yscale);
	GX_SetScissorBoxOffset(0, 0);
	GX_SetScissor(0, 0, render_mode->fbWidth, render_mode->efbHeight);
	GX_SetDispCopySrc(0, 0, render_mode->fbWidth, render_mode->efbHeight);
	GX_SetDispCopyDst(render_mode->fbWidth, xfbHeight);
	GX_SetCopyFilter(render_mode->aa, render_mode->sample_pattern, GX_TRUE, render_mode->vfilter);
	GX_SetFieldMode(render_mode->field_rendering, ((render_mode->viHeight==2*render_mode->xfbHeight) ? GX_ENABLE : GX_DISABLE));
 
	if (render_mode->aa)
        GX_SetPixelFmt(GX_PF_RGB565_Z16, GX_ZC_LINEAR);
    else
        GX_SetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);

	GX_SetCullMode(GX_CULL_NONE);
	GX_CopyDisp(frame_buffer[fb], GX_TRUE);
	GX_SetDispCopyGamma(GX_GM_1_0);
}

Camera& Renderer::createCamera(f32 fov, f32 near, f32 far, f32 x, f32 y, f32 z) {
	Vec3 camPos(x, y, z);

    f32 w = render_mode->viWidth;
	f32 h = render_mode->viHeight;

	mCamera.setPosition(camPos);
	mCamera.setProj(fov, w/h, near, far);

	return mCamera;
}

Camera& Renderer::getCamera(u8 idx) {
	return mCamera;
}


WorldData& Renderer::getWorldData() {
	return mWorldData;
}

void Renderer::drawRectangle(u8 vertexFmt, f32 x, f32 y, f32 width, f32 height, GXColor color, bool filled) {
	long n = 4;
	f32 x2 = x+width;
	f32 y2 = y+height;
	guVector v[] = {{x,y,0.0f}, {x2,y,0.0f}, {x2,y2,0.0f}, {x,y2,0.0f}, {x,y,0.0f}};
	u8 fmt = GX_TRIANGLEFAN;

	if(!filled) {
		fmt = GX_LINESTRIP;
		n = 5;
	}

	GX_Begin(fmt, vertexFmt, n);
	for(int i = 0; i < n; ++i) {
		GX_Position3f32(v[i].x, v[i].y, v[i].z);
		GX_Color4u8(color.r, color.g, color.b, color.a);
	}
	GX_End();
}

void Renderer::createSkyBox(const char* textureFileName) {
	mSkyBox.init(this, textureFileName);
}

void Renderer::TMP_renderSkyBox() {
	Mtx v;

	mCamera.buildViewMtx();
	mCamera.getViewMtx(v);

	mSkyBox.render(v);
}

void Renderer::update() {
	Mtx v;

	mCamera.buildViewMtx();
	mCamera.getViewMtx(v);
	//mSkyBox.render(v);

	// Setup projection matrix
	Mtx44 projMatrix;
	mCamera.getProjMatrix(projMatrix);
	GX_LoadProjectionMtx(projMatrix, GX_PERSPECTIVE);

	//terrain.render(v);

	EntityLibrary& entLibrary = mWorldData.getEntityLibrary();
	u16 n = entLibrary.numEntities();
	for (u16 i = 0; i < n; i++) {
		Entity& e = entLibrary.entity(i);
		render(e);
	}

	//mConsole.debugPrint(10, 10, L"frame time: %llums", deltaMS);
	mConsole.update();
	//GX_SetAlphaUpdate(GX_FALSE);
}

void Renderer::initVideo() {
	VIDEO_Init();
	// configure video
	VIDEO_Configure(render_mode);
	VIDEO_SetNextFramebuffer(frame_buffer[fb]);
	//VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(render_mode->viTVMode & VI_NON_INTERLACE) VIDEO_WaitVSync();
}

void Renderer::beforeRender() {
}

void Renderer::doneRender() {
	GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
	GX_SetColorUpdate(GX_TRUE);
    
	// Issue display copy command
	GX_CopyDisp(frame_buffer[fb],GX_TRUE);

    // Wait until everything is drawn and copied into XFB.
	GX_DrawDone();

    // Set the next frame buffer
    swapBuffers();
}

void Renderer::swapBuffers() {
	VIDEO_SetNextFramebuffer(frame_buffer[fb]);

	if(first_frame) {
		first_frame = 0;
		VIDEO_SetBlack(FALSE);
	}

	VIDEO_Flush();
	VIDEO_WaitVSync();

	fb ^= 1;
}

GXRModeObj* Renderer::getRenderModeObj() {
	return render_mode;
}

void* Renderer::getCurrentBuffer() {
	return frame_buffer[fb];
}

void Renderer::setRenderMode(GXRModeObj* mode) {
	if (render_mode != NULL) {
        render_mode = mode;
	}
    else
    {
		render_mode = VIDEO_GetPreferredMode(NULL);

        // Trim off from top & bottom 16 scan lines (which will be overscanned).
        // So almost all demos actually render only 448 lines (in NTSC case.)
        // Since this setting is just for SDK demos, you can specify this
        // in order to match your application requirements.
        //GX_AdjustForOverscan(render_mode, mode, 0, 16);
    }
}

Console& Renderer::getConsole() {
	return mConsole;
}

//==================================================================================
static void SendVertex(u16 posIndex, u16 texCoordIndex, u16 normalIdx) {
    GX_Position1x16(posIndex);
    GX_Normal1x16(normalIdx);
    GX_Color1x16(0);
    GX_TexCoord1x16(texCoordIndex);
}

//==================================================================================
// TODO: Get rid of this!
u8  ColorRGBA8[] ATTRIBUTE_ALIGN(32) = {0xFF, 0, 0, 0}; //GX_RGBA8

void Renderer::render(Entity& e) {
	const MeshLibrary& meshLibrary = mWorldData.getMeshLibrary();
	const MaterialLibrary& matLibrary = mWorldData.getMaterialLibrary();
	const TextureLibrary& texLibrary = mWorldData.getTextureLibrary();
	EntityLibrary& entLibrary = mWorldData.getEntityLibrary();

	Mesh* mesh = &meshLibrary.data[e.meshID];

	GX_ClearVtxDesc();
    GX_SetVtxDesc(GX_VA_POS, GX_INDEX16);
    GX_SetVtxDesc(GX_VA_NRM, GX_INDEX16);
    GX_SetVtxDesc(GX_VA_CLR0, GX_INDEX16);
    GX_SetVtxDesc(GX_VA_TEX0, GX_INDEX16);

    GX_SetVtxAttrFmt(GX_VTXFMT1, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT1, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT1, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT1, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

    GX_SetArray(GX_VA_POS, mesh->vertices, 3*sizeof(f32));
	GX_SetArray(GX_VA_NRM, mesh->normals, 3*sizeof(f32));
    GX_SetArray(GX_VA_TEX0, mesh->texcoord, 2*sizeof(f32));
    GX_SetArray(GX_VA_CLR0, ColorRGBA8, 4);
	
	// ---------
	Transform& transf = entLibrary.transform(e.id);
	Mtx M_view, M_modelView;
	guMtxIdentity(M_modelView);
	mCamera.getViewMtx(M_view);
	guMtxConcat(M_view, transf.m, M_modelView);
	GX_LoadPosMtxImm(M_modelView, GX_PNMTX0);

	GX_SetNumChans(1); // default, color = vertex color
	GX_SetNumTexGens(1);

	GX_SetTevOp(GX_TEVSTAGE0, GX_REPLACE);
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
	GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);	

	u16 n_subMeshes = mesh->n_subMeshes;

	u32 matID = mesh->materialID;

	for (int i = 0; i < n_subMeshes; i++) {
		SubMesh& sm = mesh->subMeshes[i];
		Material& mat = matLibrary.data[matID + sm.materialID + 1];
		Texture& tex = texLibrary.data[mat.diffuseTexID];
		GX_LoadTexObj(&tex.texObj, GX_TEXMAP0);

		u16 start	= 3*sm.start;
		u16 size	= 3*sm.size;
	
		u16 end = start + size;
		// render the submesh
	    GX_Begin(GX_TRIANGLES, GX_VTXFMT1, size);
			for (int k = start; k < end; k++) {
				u16 idx = mesh->indices[k];
				SendVertex(idx, idx, idx);
			}
	    GX_End();
	}
}
