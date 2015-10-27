#pragma once

#include "WorldData.h"
#include <gctypes.h>

struct TerrainData {
	u16 numPatchesX;
	u16 numPatchesZ;

	f32 dx;
	f32 dz;

	f32 centerX;
	f32 centerY;
	f32 centerZ;

	u16 heightMap;
	u16 alphaMap;
};

class Terrain {
public:
	Terrain();

	void init(const TerrainData& terrainData);
	void render(Mtx M_view);

private:
	Buffer<f32> mVertexBuffer;
	Buffer<u16> mIndicesBuffer;

	TerrainData mTerrainData;
	
	void genIndices(const TerrainData& terrainData);

	void SendVertex(u16 posIndex);
};

