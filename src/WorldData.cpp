#include <cstdlib>
#include <cstdio>

#include "WorldData.h"
#include "Renderer.h"
#include "MeshReader.h"
#include "System.h"

void WorldData::loadMesh(const char* filename) {
	// TODO:> WING_ASSERT buff != 0
	Console& con = System::GetRenderer()->getConsole();
	//con.log(L"loading mesh: %s", filename);

	// prevents attempts to load the same mesh twice
	if (mMeshLibrary.contains(filename)) {
		//con.log(L">>mesh already loaded, skipped.");
		return;
	}

	int n = mMeshLibrary.size;
	Mesh* mesh = &mMeshLibrary.data[n];

	char matName[256];
	mesh_read(mesh, this, filename, matName);

	//con.log(L"n: %d, vert: %p", n, mesh->vertices);

	mMeshLibrary.add(filename);

	//con.log(L">>loading material: %s", matName);

	mesh->materialID = loadMaterial(matName);

	//con.log(L">>material loaded, ID: %u", mesh->materialID);
}

u32 WorldData::loadMaterial(const char* filename) {
	u32 matID = material_read(filename, mMaterialLibrary, mTextureLibrary);
	return matID;
}

Mesh* WorldData::getMeshes(u32* out_numMeshes) const {
	*out_numMeshes = mMeshLibrary.size;
	return mMeshLibrary.data;
}

const Buffer<f32>& WorldData::getPositionBuffer() const {
	return mPositionBuffer;
}

const Buffer<f32>& WorldData::getNormalBuffer() const {
	return mNormalBuffer;
}

const Buffer<f32>& WorldData::getTexCoordBuffer() const {
	return mTexCoordBuffer;
}

const Buffer<u16>& WorldData::getIndexBuffer() const {
	return mIndexBuffer;
}

MaterialLibrary& WorldData::getMaterialLibrary() {
	return mMaterialLibrary;
}

TextureLibrary& WorldData::getTextureLibrary() {
	return mTextureLibrary;
}

MeshLibrary& WorldData::getMeshLibrary() {
	return mMeshLibrary;
}

EntityLibrary& WorldData::getEntityLibrary() {
	return mEntityLibrary;
}

void WorldData::allocateBuffer(BufferType::Type type, u64 n_elements) {
	Console& con = System::GetRenderer()->getConsole();
	if (type == BufferType::POSITION) {
		mPositionBuffer.allocate(n_elements);
	}
	if (type == BufferType::INDEX) {
		mIndexBuffer.allocate(n_elements);
	}
	if (type == BufferType::NORMAL) {
		mNormalBuffer.allocate(n_elements);
	}
	if (type == BufferType::TEX_COORD) {
		mTexCoordBuffer.allocate(n_elements);
	}
	if (type == BufferType::TEX_DATA) {
		mTextureDataBuffer.allocate(n_elements);
	}
}

void WorldData::deallocateBuffer(BufferType::Type type) {
	Console& con = System::GetRenderer()->getConsole();
	if (type == BufferType::POSITION) {
		mPositionBuffer.deallocate();
	}
	if (type == BufferType::INDEX) {
		mIndexBuffer.deallocate();
	}
	if (type == BufferType::NORMAL) {
		mNormalBuffer.deallocate();
	}
	if (type == BufferType::TEX_COORD) {
		mTexCoordBuffer.deallocate();
	}
	if (type == BufferType::TEX_DATA) {
		mTextureDataBuffer.deallocate();
	}
}

void WorldData::allocateLibrary(LibraryType::Type type, u32 size) {
	if (type == LibraryType::MATERIAL) {
		mMaterialLibrary.allocate(size);
	}
	else if (type == LibraryType::TEXTURE) {
		mTextureLibrary.allocate(size);
	}
	else if (type == LibraryType::MESH) {
		mMeshLibrary.allocate(size);
	}
}

void WorldData::deallocateLibrary(LibraryType::Type type) {
	if (type == LibraryType::MATERIAL) {
		mMaterialLibrary.deallocate();
	}
	else if (type == LibraryType::TEXTURE) {
		mTextureLibrary.deallocate();
	}
	else if (type == LibraryType::MESH) {
		mMeshLibrary.deallocate();
	}
}


u32 WorldData::createEntity(const char* meshName, f32 x, f32 y, f32 z) {
	u32 meshID = mMeshLibrary.getID(meshName);
	u32 id = 0xFFFF;
	
	if (meshID != 0xFFFF) {
		id = mEntityLibrary.add();
		Entity& ent = mEntityLibrary.get(id);
		ent.meshID = meshID;
		Transform& transf = mEntityLibrary.transform(id & INDEX_MASK);
		transf.setPosition(x, y, z);
	}
	else {
		WING_ASSERT(false && "mesh does not exist: " && meshID);
	}

	return id;
}

Transform& WorldData::getTransform(u32 entityID) {
	return mEntityLibrary.transform(entityID);
}
