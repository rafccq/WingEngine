#pragma once

#include <gctypes.h>
#include <malloc.h>

#include "AssetLibrary.h"
#include "EntityLibrary.h"
#include "Material.h"
#include "Mesh.h"


namespace BufferType {
	enum Type {POSITION, INDEX, TEX_COORD, NORMAL, TEX_DATA};
};

namespace LibraryType {
	enum Type {MATERIAL, TEXTURE, MESH};
};

/* ------------------------------------------------------------------------------------------
	Buffer is a region of memory containing data of type T, and a cursor pointing somewhere 
	inside this region.
>  -----------------------------------------------------------------------------------------*/
template <typename T>
struct Buffer {
	T* cursor;

	Buffer() {
		clear();
	}

	void allocate(u64 n) {
		mCapacity = n * sizeof(T);
		// <!> TODO[MEMORY_ALLOCATION]: Move allocations to memory allocators later.
		mData = (T*) memalign(32, mCapacity);
		cursor = mData;
	}

	void clear() {
		mData = 0;
		cursor = 0;
		mSize_B = 0;
		mCapacity = 0;
	}

	void deallocate() {
		free(mData);
		clear();
	}

	bool fit(u32 n) {
		return (mSize_B + n * sizeof(T)) <= mCapacity;
	}

	void add(u32 n) {
		mSize_B += n*sizeof(T);
		cursor += n;
	}

	u64 size() const{
		return mSize_B;
	}

	u64 capacity() const {
		return mCapacity;
	}

	void resetCursor() {
		cursor = mData;
	}

private:
	T* mData;
	u64 mSize_B;
	u64 mCapacity;

	Buffer(Buffer<T>& b);
	Buffer<T>& operator=(Buffer<T>& b);
};

// --------------------------------------- Forward Declaractions ---------------------------------------
class Renderer;
// -----------------------------------------------------------------------------------------------------

class WorldData {
public:

	/* ----------------------------------------------------------------------------------------------------
		Will load the mesh contained in filename to memory. The mesh referenced material is loaded as well.
		After loading, the mesh will be available for access in mMeshLibrary.
	>  ---------------------------------------------------------------------------------------------------*/
	void loadMesh(const char* filename);
	u32 loadMaterial(const char* filename);

	/* ----------------------------------------------------------------------------------------------------
		"Getters" for the buffers, note that the return type is const, clients are not supposed to modify 
		these buffers directly.
	>  ---------------------------------------------------------------------------------------------------*/
	const Buffer<f32>& getPositionBuffer() const;
	const Buffer<f32>& getNormalBuffer() const;
	const Buffer<f32>& getTexCoordBuffer() const;
	const Buffer<u16>& getIndexBuffer() const;

	/* ----------------------------------------------------------------------------------------------------
		Allocates/Deallocates memory for the given buffer.
	 > ---------------------------------------------------------------------------------------------------*/
	void allocateBuffer(BufferType::Type type, u64 size);
	void deallocateBuffer(BufferType::Type type);

	Mesh* getMeshes(u32* out_numMeshes) const;

	void allocateLibrary(LibraryType::Type type, u32 size);
	void deallocateLibrary(LibraryType::Type type);

	/* ----------------------------------------------------------------------------------------------------
		Creates an object which has a mesh associated. Important note: Objects created through this function 
		will be dynamic renderable objects!
	>  ---------------------------------------------------------------------------------------------------*/
	u32 createEntity(const char* meshName, f32 x=0, f32 y=0, f32 z=0);

	Transform& getTransform(u32 entityID);

	/* ----------------------------------------------------------------------------------------------------
		Accessors to the libraries.
	 > ---------------------------------------------------------------------------------------------------*/
	MaterialLibrary& getMaterialLibrary();
	TextureLibrary& getTextureLibrary();
	EntityLibrary& getEntityLibrary();
	MeshLibrary& getMeshLibrary();

private:
	Buffer<f32> mPositionBuffer;
	Buffer<f32> mNormalBuffer;
	Buffer<f32> mTexCoordBuffer;
	Buffer<u16> mIndexBuffer;
	Buffer<u8> mTextureDataBuffer;

	MaterialLibrary mMaterialLibrary;
	TextureLibrary mTextureLibrary;
	MeshLibrary mMeshLibrary;
	EntityLibrary mEntityLibrary;
};

