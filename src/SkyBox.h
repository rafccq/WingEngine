#pragma once

#include <gctypes.h>
#include <ogc\gu.h>

// --------------------------------------- Forward Declaractions ---------------------------------------
class Renderer;
class Texture;
// -----------------------------------------------------------------------------------------------------

class SkyBox {
public:
	SkyBox();
	void init(Renderer* renderer, const char* textureFileName);
	void render(Mtx view);

private:
	Renderer* mRenderer;
	Texture* mTexture;
};

