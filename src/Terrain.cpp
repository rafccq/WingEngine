#include "Terrain.h"
#include "System.h"

Terrain::Terrain(): mTerrainData() {
}

void Terrain::init(const TerrainData& terrainData) {
	mTerrainData.numPatchesX = terrainData.numPatchesX;
	mTerrainData.numPatchesZ = terrainData.numPatchesZ;

	u16 nVerticesX = terrainData.numPatchesX + 1;
	u16 nVerticesZ = terrainData.numPatchesZ + 1;

	u32 nTotalVertices = nVerticesX*nVerticesZ;

	mVertexBuffer.allocate(nTotalVertices*3);
	//System::Log(L"nv=%d", nTotalVertices);

	f32 width = terrainData.numPatchesX * terrainData.dx;
	f32 depth = terrainData.numPatchesZ * terrainData.dz;

	f32 xOffset = -width * 0.5f + terrainData.centerX;
	f32 zOffset =  depth * 0.5f + terrainData.centerZ;

	f32* verts = mVertexBuffer.cursor;
	for (int i = 0; i < nVerticesZ; i++) {
		for (int j = 0; j < nVerticesX; j++) {
			verts[0] =  j*terrainData.dx + xOffset;	// x
			verts[1] = terrainData.centerY;			// y
			verts[2] = -i*terrainData.dz + zOffset;	// z

			verts += 3;
		}
	}

	verts = mVertexBuffer.cursor;

	int k = 0;
	for (int i = 0; i < nVerticesX; i++) {
		for (int j = 0; j < nVerticesZ; j++) {
			//System::Log(L"v[%d]= (%f, %f, %f)", k, verts[0], verts[1], verts[2]);

			verts += 3;
			k++;
		}
	}

	//System::Log(L"vbuf=%p", mVertexBuffer.cursor);

	genIndices(terrainData);
}

void Terrain::genIndices(const TerrainData& terrainData) {
	u32 nTotalIndices = terrainData.numPatchesX * terrainData.numPatchesZ * 6;
	mIndicesBuffer.allocate(nTotalIndices);
	//System::Log(L"ni=%d", nTotalIndices);

	u16 nVerticesX = terrainData.numPatchesX + 1;

	u16* indices = mIndicesBuffer.cursor;
	for (int i = 0; i < terrainData.numPatchesZ; i++) {
		for (int j = 0; j < terrainData.numPatchesX; j++) {
			indices[0] =	 i		* nVerticesX + j;
			indices[1] =	 i		* nVerticesX + j + 1;
			indices[2] = (i + 1)	* nVerticesX + j;

			indices[3] = (i + 1)	* nVerticesX + j;
			indices[4] =	 i		* nVerticesX + j + 1;
			indices[5] = (i + 1)	* nVerticesX + j + 1;

			indices += 6;
		}
	}

	indices = mIndicesBuffer.cursor;

	//System::Log(L"------");
	int k = 0;
	for (int i = 0; i < terrainData.numPatchesX; i++) {
		for (int j = 0; j < terrainData.numPatchesZ; j++) {
			//System::Log(L"i[%d]= (%d, %d, %d), (%d, %d, %d)", k, indices[0], indices[1], indices[2], indices[3], indices[4], indices[5]);

			indices += 6;
			k++;
		}
	}
}

u8  vertexColorRGBA[] ATTRIBUTE_ALIGN(32) = {0xFF, 0xFF, 0x00, 0xFF}; //GX_RGBA8

void Terrain::SendVertex(u16 posIndex) {
	GX_Position1x16(posIndex);
}

void Terrain::render(Mtx M_view) {
	static int tmp = 1;

	GX_ClearVtxDesc();

	GX_SetVtxDesc(GX_VA_POS, GX_INDEX16);

    GX_SetVtxAttrFmt(GX_VTXFMT4, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);

	GX_SetArray(GX_VA_POS, mVertexBuffer.cursor, 3*sizeof(f32));

	// build and load model-view matrix
	Mtx M_modelView;
	guMtxIdentity(M_modelView);
	guMtxConcat(M_view, M_modelView, M_modelView);
	GX_LoadPosMtxImm(M_modelView, GX_PNMTX0);

	GX_SetNumChans(1); // default, color = vertex color

    GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);

	u16* indices = mIndicesBuffer.cursor;

	u16 numPatches = mTerrainData.numPatchesX * mTerrainData.numPatchesZ;

	GX_Begin(GX_LINES, GX_VTXFMT4, numPatches*12);
	for (u16 i = 0; i < numPatches; i++) {
		// face 1
		SendVertex(indices[0]);
		SendVertex(indices[1]);

		SendVertex(indices[1]);
		SendVertex(indices[2]);

		SendVertex(indices[2]);
		SendVertex(indices[0]);

		// face 2
		SendVertex(indices[3]);
		SendVertex(indices[4]);

		SendVertex(indices[4]);
		SendVertex(indices[5]);

		SendVertex(indices[5]);
		SendVertex(indices[3]);

		indices += 6;
	}
	GX_End();
}
