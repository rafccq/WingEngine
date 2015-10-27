#pragma once

#include <map>
#include <cstdlib>
#include <cstring>
#include <string>
#include <gctypes.h>

#include "Material.h"
#include "Mesh.h"

typedef std::map<std::string, u32> Dictionary;

template <typename T>
struct AssetLibrary {
	T* data;
	u32 size;
	u32 allocatedSize;
	Dictionary dictionary;

	void allocate(u32 n) {
		data = new T[n];
		allocatedSize = n;
		size = 0;
	}

	void deallocate() {
		delete[] data;
		dictionary.clear();
	}

	bool contains(const char* assetName) {
		return dictionary.find(assetName) != dictionary.end();
	}

	void add(const char* assetName) {
		if (contains(assetName)) {
			return;
		}

		u32 assetIdx = this->size;
		dictionary.insert(Dictionary::value_type(assetName, assetIdx));
		this->size++;
	}

	T& get(const char* assetName) {
		u32 assetIdx = dictionary[assetName];
		return data[assetIdx];
	}

	u32 getID(const char* assetName) {
		if (contains(assetName)) {
			u32 assetIdx = dictionary[assetName];
			return assetIdx;
		}
		
		//System::Log(L"Asset not found: %s", assetName.c_str());
		return 0xFFFF;
	}

};

typedef AssetLibrary<Material> MaterialLibrary;
typedef AssetLibrary<Texture> TextureLibrary;
typedef AssetLibrary<Mesh> MeshLibrary;
