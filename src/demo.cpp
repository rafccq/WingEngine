#include <network.h>

#include "demo.h"
#include "Renderer.h"


//Renderer renderer;

// <PATCH>
//GXRModeObj* DEMOGetRenderModeObj() {
//	return renderer.getRenderModeObj();
//}
//
//void DEMOBeforeRender() {
//	renderer.beforeRender();
//}
//
//void DEMODoneRender() {
//	renderer.doneRender();
//}


// <PATCH>
void initNet() {
	char localip[16] = {0};
	char gateway[16] = {0};
	char netmask[16] = {0};
	
	puts("Configuring network ...");

	// Configure the network interface
	s32 ret = if_config (localip, netmask, gateway, TRUE);
	if (ret >= 0)
		printf ("Network configured, ip: %s, gw: %s, mask %s\n", localip, gateway, netmask);
}

bool loadPngImage(char* filename, int* outWidth, int* outHeight, bool* outHasAlpha, char** outData) {
	FILE* file = fopen(filename, "rb");
	PNGUPROP imgProp;
	IMGCTX ctx = PNGU_SelectImageFromDevice(filename);
	PNGU_GetImageProperties(ctx, &imgProp);
	*outData = (char*) memalign(32, imgProp.imgWidth * imgProp.imgHeight * 4);
	PNGU_DecodeTo4x4RGBA8 (ctx, imgProp.imgWidth, imgProp.imgHeight, *outData, 255);
	PNGU_ReleaseImageContext (ctx);
	*outWidth = imgProp.imgWidth;
	*outHeight = imgProp.imgHeight;
	//DCFlushRange (data, imgProp.imgWidth * imgProp.imgHeight * 4);
	//GX_InitTexObj (&texture, data, imgProp.imgWidth, imgProp.imgHeight, GX_TF_RGBA8, GX_MIRROR, GX_MIRROR, GX_FALSE);
	fclose(file);

	DCFlushRange (*outData, imgProp.imgWidth * imgProp.imgHeight * 4);

	return true;
}

void loadTexture(const char* filename, GXTexObj* texObj) {
	FILE* temp = fopen(filename, "rb");
	PNGUPROP imgProp;
	IMGCTX ctx = PNGU_SelectImageFromDevice(filename);
	PNGU_GetImageProperties(ctx, &imgProp);
	void* data = memalign(32, imgProp.imgWidth * imgProp.imgHeight * 4);
	PNGU_DecodeTo4x4RGBA8 (ctx, imgProp.imgWidth, imgProp.imgHeight, data, 255);
	PNGU_ReleaseImageContext (ctx);
	DCFlushRange (data, imgProp.imgWidth * imgProp.imgHeight * 4);
	GX_InitTexObj (texObj, data, imgProp.imgWidth, imgProp.imgHeight, GX_TF_RGBA8, GX_REPEAT, GX_REPEAT, GX_TRUE);
	GX_InitTexObjMaxAniso(texObj, GX_ANISO_4);
	fclose(temp);
}


bool DEMOCheckExtension(u8 extensionID) {
	struct expansion_t data;
	WPAD_ScanPads();
	WPAD_Expansion(WPAD_CHAN_0, &data);

	return data.type == extensionID;
}

u32 DEMOPadGetButtonDown(int channel) {
	return WPAD_ButtonsDown(channel);
}

bool DEMOReadJoystickXY(f32* vx, f32* vy, f32 scaleX, f32 scaleY) {
	struct expansion_t data;
	//WPAD_ScanPads();

	WPAD_Expansion(WPAD_CHAN_0, &data); // Get expansion info from the first wiimote

	if (data.type == WPAD_EXP_NUNCHUK)
	{
		joystick_t js = data.nunchuk.js;
		f32 size_x = (f32) (js.max.x - js.center.x);
		f32 size_y = (f32) (js.max.y - js.center.y);

		f32 jx = (js.pos.x - js.center.x) / size_x;
		f32 jy = (js.pos.y - js.center.y) / size_y;

		f32 treshold = 0.03f;
		jx = (fabs(jx) < treshold) ? 0.0f : jx;
		jy = (fabs(jy) < treshold) ? 0.0f : jy;

		*vx = jx * scaleX;
		*vy = jy * scaleY;

		return true;
	}
	else
		return false;
}

bool DEMOReadJoystick(f32* sx, f32* sy, f32* subx, f32* suby, f32 scale, u16 nunchuck_button) {
	f32 jx = 0, jy = 0;
	*sx = *sy = *subx = *suby = 0;

	bool nunchuck = DEMOReadJoystickXY(&jx, &jy, scale, scale);

	struct expansion_t data;
	WPAD_ScanPads();
	WPAD_Expansion(WPAD_CHAN_0, &data);
	if (data.type == WPAD_EXP_NUNCHUK && (data.nunchuk.btns & nunchuck_button)) {
		*subx = jx;
		*suby = jy;
	}
	else {
		*sx = jx;	
		*sy = jy;
	}

	return nunchuck;
}

f32 Clamp(f32 val, f32 min, f32 max) {
	if (val < min) return min;
	if (val > max) return max;

	return val;
}

f32 Max(f32 val1, f32 val2) {
	return (val1 > val2) ? val1 : val2;
}
