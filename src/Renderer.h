#pragma once

#include <gccore.h>

#include "WorldData.h"
#include "EntityLibrary.h"
#include "Console.h"
#include "SkyBox.h"
#include "Camera.h"

// --------------------------------------- Forward Declaractions ---------------------------------------
class Mesh;
// -----------------------------------------------------------------------------------------------------

class Renderer {
public:
	Renderer() {};
	~Renderer();

	void init();
	void initGX();
	Camera& createCamera(f32 fov, f32 near, f32 far, f32 x = 0, f32 y = 0, f32 z = 0);
	void beforeRender();
	void doneRender();
	void swapBuffers();
	void* getCurrentBuffer();
	void initVideo();
	void setRenderMode(GXRModeObj* mode);
	
	void render(Entity& e);

	WorldData& getWorldData();
	void update();
	GXRModeObj* getRenderModeObj();
	void setFBIndex(u8 _fb) {fb = _fb;}
	u16 getFBwidth();
	u16 getFBheight();

	void drawRectangle(u8 vertexFmt, f32 x, f32 y, f32 width, f32 height, GXColor color, bool filled = true);
	void createSkyBox(const char* textureFileName);
	Console& getConsole();

	Camera& getCamera(u8);

	// ----- TEMP
	void renderMesh(Mesh* mesh, Mtx v);

private:
	WorldData mWorldData;
	WorldData mPersistentWorldData;

	GXRModeObj* render_mode;
	GXFifoObj* gx_fifo;
	u8 fb;
	void* frame_buffer[2];
	u32 first_frame;
	SkyBox mSkyBox;

	Console mConsole;

	/* ------------------------------------------------------------------------------------------
	>	The Renderer Camera.
	>	Note: In the future, will be possible to have more than one camera.
	>  ------------------------------------------------------------------------------------------*/
	Camera mCamera;
};
