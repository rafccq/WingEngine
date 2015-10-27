#pragma once

#include <gctypes.h>
#include <climits>

struct Entity {
	u32 id;
	u32 parentID;
	u16 meshID;
};

struct Index {
	u16 id;
	u16 index;
	u16 next;
};

struct Transform {
	Mtx m;
	Mtx33 rotation;
	guVector position;
	f32 scale;

	Transform();

	void translate(f32 x, f32 y, f32 z);
	void setPosition(f32 x, f32 y, f32 z);

	void rotateX(f32 angle_rad);
	void rotateY(f32 angle_rad);
	void rotateZ(f32 angle_rad);

	void update();

	void multiply(const Transform& parent);
};

#define MAX_OBJS 2*1024
#define NEW_OBJECT_ID_ADD 0x10000
#define INDEX_MASK 0xffff

// The EntityLibrary uses a free list (https://en.wikipedia.org/wiki/Free_list) to keep the entities.
class EntityLibrary {
public:
	EntityLibrary() {
		clear();
	}

	void clear() {
		mNumEntities = 0;

		// setup the indices array, and parents
		for (u16 i = 0; i < MAX_OBJS; ++i) {
			mIndices[i].id = i;
			mIndices[i].next = i+1;

			mEntities[i].parentID = UINT32_MAX;
		}

		// setup the freelists indices
		mFreeListHead = 0;
		mFreeListTail = MAX_OBJS - 1;
	}

	inline bool has(u32 id) {
		Index &in = mIndices[id & INDEX_MASK];
		return in.id == id && in.index != USHRT_MAX;
	}

	inline Entity& get(u32 id) {
		return mEntities[mIndices[id & INDEX_MASK].index];
	}

	u32 add() {
		Index& in = mIndices[mFreeListHead];
		mFreeListHead = in.next;
		in.id += NEW_OBJECT_ID_ADD;
		in.index = mNumEntities++;
		Entity& ent = mEntities[in.index];
		ent.id = in.id;
		return ent.id;
	}

	void remove(u32 id) {
		Index& in = mIndices[id & INDEX_MASK];
		
		Entity& ent = mEntities[in.index];
		ent = mEntities[--mNumEntities];
		mIndices[ent.id & INDEX_MASK].index = in.index;
		
		in.index = USHRT_MAX;
		mIndices[mFreeListTail].next = id & INDEX_MASK;
		mFreeListTail = id & INDEX_MASK;
	}

	inline Entity& entity(u16 i) {
		return mEntities[i];
	}

	inline Transform& transform(u32 id) {
		return mTransforms[id & INDEX_MASK];
	}

	inline u16 numEntities() {
		return mNumEntities;
	}

	void setParent(u32 id, u32 parent) {
		Entity& ent = mEntities[id & INDEX_MASK];
		ent.parentID = parent;
	}

	void update();

private:
	Entity mEntities[MAX_OBJS];
	Index mIndices[MAX_OBJS];

	Transform mTransforms[MAX_OBJS];

	u16 mFreeListHead;
	u16 mFreeListTail;
	u16 mNumEntities;
};

