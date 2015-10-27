#include <cstdio>
#include <malloc.h>
#include <pngu.h>
#include "ogc/cache.h"
#include <wiiuse/wpad.h>

#include "MeshReader.h"
#include "WorldData.h"

#include "System.h"
#include "Console.h"

using namespace std;

void Debug(const Buffer<f32>& buffer) {
	Renderer* renderer = System::GetRenderer();
	renderer->setFBIndex(0);
	renderer->doneRender();

	//printf("\x1b[4;0H");

	Console& con = renderer->getConsole();
	//con.log(L"data = %p", buffer.data);
	//con.log(L"curs = %p", buffer.cursor);
	//con.log(L"capac= %llu", buffer.capacity());
	//con.log(L"size = %llu", buffer.size());

	while (1) {
		WPAD_ScanPads();
		con.update();
		renderer->doneRender();

		if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME) {
			printf("\x1b[9;0H");
			printf("\tExiting...");
			exit(0);
		}
	}
}

void mesh_read(Mesh* out, WorldData* worldData, const char* filename, char* out_matName) {
	// read file contents
	ifstream inFile(filename, ios::in | ios::binary);
	
	WING_ASSERT(inFile.good() && "File does not exist.", filename);

	// read header
	header_t header;
	inFile.read((char*) &header, sizeof(header));

	// fill sizes info
	out->n_vertices	= header.n_vertices;
	out->n_texcoord	= header.n_vertices;
	out->n_normals	= header.n_vertices;
	out->n_subMeshes = header.n_subMeshes;

	// read the material name
	inFile.read(out_matName, header.len_material);

	// get buffers
	Buffer<f32>& positionBuffer	= const_cast<Buffer<f32>&>(worldData->getPositionBuffer());
	Buffer<f32>& normalBuffer	= const_cast<Buffer<f32>&>(worldData->getNormalBuffer());
	Buffer<f32>& texCoordBuffer = const_cast<Buffer<f32>&>(worldData->getTexCoordBuffer());
	Buffer<u16>& indexBuffer	= const_cast<Buffer<u16>&>(worldData->getIndexBuffer());

	// read positions
	u32 n_elements = 3*header.n_vertices;

	//System::GetRenderer()->getConsole().log(L"n_elements= %d", n_elements);
	//System::GetRenderer()->getConsole().log(L"n_vertices= %d", header.n_vertices);
	//System::GetRenderer()->getConsole().log(L"fit= %d", positionBuffer.fit(n_elements));

	bool fits = positionBuffer.fit(n_elements);
	//System::GetRenderer()->getConsole().log(L"fits= %s", fits ? "true" : "false");
	if (!fits) {
		Debug(positionBuffer);
	}
	//Debug(positionBuffer);
	//WING_ASSERT(fit);
	//WING_ASSERT(positionBuffer.fit(n_elements));
	inFile.read((char*) positionBuffer.cursor, n_elements*sizeof(f32));
	out->vertices = positionBuffer.cursor;
	positionBuffer.add(n_elements);

	// read normals
	n_elements = 3*out->n_normals;
	WING_ASSERT(normalBuffer.fit(n_elements), "NOT ENOUGH SPACE IN NORMAL BUFFER");
	inFile.read((char*) normalBuffer.cursor, n_elements*sizeof(f32));
	out->normals = normalBuffer.cursor;
	normalBuffer.add(n_elements);

	// read texture coordinates
	n_elements = 2*out->n_texcoord;
	WING_ASSERT(texCoordBuffer.fit(n_elements), "NOT ENOUGH SPACE IN TEXCOORD BUFFER");
	inFile.read((char*) texCoordBuffer.cursor, n_elements*sizeof(f32));
	out->texcoord = texCoordBuffer.cursor;
	texCoordBuffer.add(n_elements);

	// read indices
	n_elements = 3*header.n_faces;
	WING_ASSERT(indexBuffer.fit(n_elements), "NOT ENOUGH SPACE IN INDEX BUFFER");
	inFile.read((char*) indexBuffer.cursor, n_elements*sizeof(u16));
	out->indices = indexBuffer.cursor;
	indexBuffer.add(n_elements);

	// read sub meshes
	n_elements = 2*out->n_subMeshes;
	u16* subMeshes_src = new u16[n_elements];
	out->subMeshes = new SubMesh[out->n_subMeshes];

	inFile.read((char*) subMeshes_src, n_elements*sizeof(u16));

	for (u32 i = 0,k=0; i < n_elements; i+=2,k++) {
		u32 start	= subMeshes_src[i];
		u32 n		= subMeshes_src[i+1];
		out->subMeshes[k].set(start, n);
		printf("subMesh[%d] = (%d, %d)\n", k, start, n);
	}

	delete[] subMeshes_src;

	// read materials
	n_elements = out->n_subMeshes;
	u8* materials = new u8[n_elements];
	inFile.read((char*) materials, n_elements*sizeof(u8));

	for (u32 i = 0; i < n_elements; i++)
		out->subMeshes[i].materialID = materials[i];

	delete[] materials;
}

/**
	Reads the material from the file to memory. 
	
	filename -- Name of the material file.
	materialLibrary -- The pre-allocated array of materials, where the loaded material will be put.
	index -- index into the materialLibrary.

	return: reference to the loaded material.
*/

u32 material_read(const char* filename, MaterialLibrary& materialLibrary, TextureLibrary& textureLibrary) {
	u16 index = materialLibrary.size;
	Material& newMaterial = materialLibrary.data[index];

	ifstream input(filename, ios::in | ios::binary);

	u8 size_info[2];
	input.read((char*)size_info, 2*sizeof(u8));

	u8 name_size = size_info[0];
	u8 n_subMat = size_info[1];

	char name[256];
	input.read((char*)name, name_size);

	printf("read material: %s (%d submaterials)\n", name, n_subMat);
	
	newMaterial.ID = index;
	newMaterial.n_subMaterials = n_subMat;

	// read the submaterials
	for (int i = 0; i < n_subMat; i++) {
		Material& subMat = materialLibrary.data[++index];
		u8 diffuse_size;
		input.read((char*) &diffuse_size, 1);

		char tex_diffuse[256];
		input.read(tex_diffuse, diffuse_size);

		u32 diffuseID = texture_read(tex_diffuse, textureLibrary);
		subMat.diffuseTexID = diffuseID;

		subMat.colorRGBA = 0;
	}

	materialLibrary.size = index;

	materialLibrary.add(filename);

	return newMaterial.ID;
}

u32 texture_read(const char* filename, TextureLibrary& textureLibrary) {
	//Console& con = System::GetRenderer()->getConsole();

	if (textureLibrary.contains(filename)) {
		return textureLibrary.dictionary[filename];
	}

	//fprintf(f, "Library does NOT contains %s\n", filename);

	u32 n = textureLibrary.size;
	Texture& newTexture = textureLibrary.data[n];
	newTexture.ID = n;

	PNGUPROP imgProp;
	IMGCTX ctx = PNGU_SelectImageFromDevice(filename);
	PNGU_GetImageProperties(ctx, &imgProp);
	#warning "ALLOCATE THIS MEMORY EXTERNALLY"
	void* data = memalign(32, imgProp.imgWidth * imgProp.imgHeight * 4);

	PNGU_DecodeTo4x4RGBA8(ctx, imgProp.imgWidth, imgProp.imgHeight, data, 255);
	//PNGU_DecodeToRGBA8(ctx, imgProp.imgWidth, imgProp.imgHeight, data, 0, 255);
	PNGU_ReleaseImageContext(ctx);
	DCFlushRange(data, imgProp.imgWidth * imgProp.imgHeight * 4);
	GX_InitTexObj(&newTexture.texObj, data, imgProp.imgWidth, imgProp.imgHeight, GX_TF_RGBA8, GX_REPEAT, GX_REPEAT, GX_TRUE);

	textureLibrary.add(filename);

	return newTexture.ID;
}
