#include <cstdio>
#include <cstdlib>
#include <cmath>

#include <ogc\gx.h>
#include <ogc\gu.h>
#include <wiiuse\wpad.h>

#include "Console.h"
#include "Utils.h"
#include "System.h"
#include "FreeTypeGX.h"

void Console::init(Renderer* renderer) {
	mRenderer = renderer;
	GX_SetVtxAttrFmt(GX_VTXFMT3, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT3, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT3, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

	FONT_NAME = "font.ttf";
	initFont();
	setupMatrices();

	mActive = false;
	mScrollLock = false;
}


void Console::initFont() {
	// reads the default font, and create the font system
	u32 size = 0;
	u8* fontData = Utils::ReadFileBytes(FONT_NAME, &size);
	
	WING_ASSERT(fontData != 0, "CONSOLE FONT FILE NOT FOUND.");

	mFontSystem = new FreeTypeGX(GX_TF_IA8, GX_VTXFMT2);
	mFontSystem->loadFont(fontData, size, 20, false);
}

u16 Console::getNumLines() {
	if (mFirst > mLast)
		return N_LINES - 1;

	return mLast - mFirst;
}

void Console::setupMatrices() {
	guMtxIdentity(mModelView2D);
	guMtxTransApply(mModelView2D, mModelView2D, 0.0f, 20.0f, -5.0f);
	guOrtho(mProjection, 0, 479+30, 0, 639, 0, 300);
}

void Console::log(wchar_t* fmt, ...) {
	wchar_t* text = mBuffer[mLast];

	va_list args;
    va_start(args, fmt);

	vswprintf(text, 128, fmt, args);

    va_end(args);

	increaseLines();
}

void Console::increaseLines() {
	u16 numLines = getNumLines();

	mLast = (mLast + 1) % N_LINES;

	// increase the first element when the buffer is full
	if (numLines == (N_LINES-1)) {
		mFirst = (mFirst + 1) % N_LINES;
	}

	// auto scroll down
	if (numLines > mLinesPerPage) {
		s16 diff = mLast - mLinesPerPage;
		mFirstLine = (diff >= 0) ? diff : N_LINES + diff;
	}
}

void Console::debugPrint(f32 x, f32 y, wchar_t* fmt, ...) {
	wchar_t* text = mBuffer[N_LINES-1];

	setupRender();
    
	va_list args;
    va_start(args, fmt);

	vswprintf(text, 128, fmt, args);
	mFontSystem->drawText(x, y, text, (GXColor){0x00, 0x00, 0x00, 0xFF}, FTGX_ALIGN_TOP);

    va_end(args);
		
	GX_SetBlendMode(GX_BM_NONE, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_SET);
}

wchar_t* Console::getCurrentLine() {
	return mBuffer[mLast];
}


void Console::update() {
	if(WPAD_ButtonsDown(0) & WPAD_BUTTON_2) {
		mActive = !mActive;
	}
	
	if(WPAD_ButtonsDown(0) & WPAD_BUTTON_1) mScrollLock = !mScrollLock;
	
	if (!mActive) {
		return;
	}

	if(!mScrollLock && WPAD_ButtonsDown(0) & WPAD_BUTTON_DOWN) {
		u16 bottom = (mFirstLine + mLinesPerPage) % N_LINES;
		if (bottom != mLast) {
			mFirstLine = (mFirstLine + 1) % N_LINES;
		}
	}
	else if(!mScrollLock && WPAD_ButtonsDown(0) & WPAD_BUTTON_UP) {
		// wraps when under 0
		if (mFirstLine != mFirst)
			mFirstLine = (mFirstLine - 1) >= 0 ? (mFirstLine - 1) : (N_LINES - 1);
	}

	setupRender();
	render();
	GX_SetAlphaUpdate(GX_FALSE);
}

void Console::setupRender() {
	// setup vertex descriptor
	GX_ClearVtxDesc();
	GX_SetVtxDesc(GX_VA_TEX0, GX_NONE);
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);

	GX_SetVtxAttrFmt(GX_VTXFMT3, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT3, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT3, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

	// load model-view, and projection matrices
	GX_LoadPosMtxImm(mModelView2D, GX_PNMTX0);
	GX_LoadProjectionMtx(mProjection, GX_ORTHOGRAPHIC);

	// setup tev
	GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
	GX_SetAlphaUpdate(GX_TRUE);

	GX_SetNumChans(1);
	GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
}

void Console::render() {
	u16 x = 10, y = 10;
	u16 w = 620, h = 300;
	mRenderer->drawRectangle(GX_VTXFMT3, x, y, w, h, (GXColor){0x00, 0x00, 0x00, 0xCD});

	u16 y0 = 15;
	y = y0;
	u16 dy = 25;

	u16 start = mFirstLine;
	u16 end = mFirstLine + mLinesPerPage;
	//end = end > 127 ? 127 : end;
	//end = end > mNumLines ? mNumLines : end;

	if (mLast < 0) {
		return;
	}

	for (u16 i = start; i < end; i++) {
		int lineIdx = i % N_LINES;

		if (lineIdx == mLast) {
			break;
		}

		mFontSystem->drawText(x, y, mBuffer[lineIdx], (GXColor){0xFF, 0xFF, 0xFF, 0xFF}, FTGX_ALIGN_TOP);
		y += dy;
	}
}

void Console::clear() {
	mFirst = 0;
	mLast = 0;
}