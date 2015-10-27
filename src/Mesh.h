#pragma once

#include <gctypes.h>


struct SubMesh {
	u16 start;
	u16 size;
	u8 materialID;

	void set(u16 pStart, u16 pSize) {
		start = pStart;
		size = pSize;
	}
};

struct Mesh {
	u16 n_vertices;
	u16 n_tris;
	u16 n_texcoord;
	u16 n_normals;
	u16 n_subMeshes;

	f32* vertices;
	u16* indices;
	f32* texcoord;
	f32* normals;
	u32 materialID;
	SubMesh* subMeshes;

	Mesh() {
		vertices = 0;
		texcoord = 0;
		normals = 0;
		subMeshes = 0;
	}
};
