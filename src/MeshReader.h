#pragma once

#include <fstream>
#include <gctypes.h>

#include "AssetLibrary.h"
#include "Math_types.h"

#define PRINT_DEBUG { \
	printf("n_vertices = %d\n", header.n_vertices); \
	printf("n_tris = %d\n", header.n_tris); \
	printf("n_texcoord = %d\n", header.n_texcoord); \
	printf("n_normals = %d\n", header.n_normals); \
	printf("n_submeshes = %d\n", header.n_subMeshes); \
}

#define PRINT_I(msg, i) {printf(msg); printf("%d\n", i);}

struct header_t {
	u16 n_vertices;
	u16 n_faces;
	u16 n_subMeshes;
	u16 len_material;
};

class WorldData;

void readVec3f(std::ifstream& inFile, int n_elements, Vec3* out, const char* dataName);
void readVec2f(std::ifstream& inFile, int n_elements, Vec2* out, const char* dataName);

void mesh_read(Mesh* out, WorldData* worldData, const char* filename, char* out_matName);
void mesh_dump(const char* filename, Mesh& in);
u32 texture_read(const char* filename, TextureLibrary& textureLibrary);
u32 material_read(const char* filename, MaterialLibrary& materialLibrary, TextureLibrary& textureLibrary);