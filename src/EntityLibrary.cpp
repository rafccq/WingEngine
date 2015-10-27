#include <cwchar>
#include <cstring>
#include <math.h>

#include <ogc\gu.h>

#include "EntityLibrary.h"
#include "System.h"

#define PLATFORM_BIG_ENDIAN

#include "foundation\collection_types.h"
#include "foundation\temp_allocator.h"
#include "foundation\hash.h"
#include "foundation\queue.h"
#include "foundation\memory.h"

using namespace foundation;

Transform::Transform() {
	guMtxIdentity(m);

	position = guVector();

	Mtx33P r = rotation;
	r[0][0] = 1.0f;  r[0][1] =  0.0f;    r[0][2] =  0.0f;
	r[1][0] = 0.0f;  r[1][1] =  1.0f;    r[1][2] =  0.0f;
	r[2][0] = 0.0f;  r[2][1] =  0.0f;    r[2][2] =  1.0f;

	scale = 1.0f;

	m[0][0] = 1.0f;		m[0][1] =  0.0f;    m[0][2] =  0.0f;
	m[1][0] = 0.0f;		m[1][1] =  1.0f;    m[1][2] =  0.0f;
	m[2][0] = 0.0f;		m[2][1] =  0.0f;    m[2][2] =  1.0f;

	m[0][3] = 0.0f;		m[1][3] = 0.0f;		m[2][3] = 0.0f;
}

void Transform::translate(f32 x, f32 y, f32 z) {
	position.x += x;
	position.y += y;
	position.z += z;
}

void Transform::setPosition(f32 x, f32 y, f32 z) {
	position.x = x;
	position.y = y;
	position.z = z;
}

void Transform::rotateX(f32 angle_rad) {
	f32 sinA = sinf(angle_rad);
	f32 cosA = cosf(angle_rad);

	Mtx33P r = rotation;
	r[0][0] = 1.0f;  r[0][1] =  0.0f;    r[0][2] =   0.0f;
	r[1][0] = 0.0f;  r[1][1] =  cosA;    r[1][2] =  -sinA;
	r[2][0] = 0.0f;  r[2][1] =  sinA;    r[2][2] =   cosA;
}

void Transform::rotateY(f32 angle_rad) {
	f32 sinA = sinf(angle_rad);
	f32 cosA = cosf(angle_rad);

	Mtx33P r = rotation;
	r[0][0] =  cosA;  r[0][1] =  0.0f;    r[0][2] =  sinA;
	r[1][0] =  0.0f;  r[1][1] =  1.0f;    r[1][2] =  0.0f;
	r[2][0] = -sinA;  r[2][1] =  0.0f;    r[2][2] =  cosA;
}

void Transform::rotateZ(f32 angle_rad) {
	f32 sinA = sinf(angle_rad);
	f32 cosA = cosf(angle_rad);

	Mtx33P r = rotation;
	r[0][0] =  cosA;  r[0][1] = -sinA;    r[0][2] =  0.0f;
	r[1][0] =  sinA;  r[1][1] =  cosA;    r[1][2] =  0.0f;
	r[2][0] =  0.0f;  r[2][1] =  0.0f;    r[2][2] =  1.0f;
}

void Transform::update() {
	// copy position to Mtx
	m[0][3] = position.x;
	m[1][3] = position.y;
	m[2][3] = position.z;

	// copy rotation + scale to Mtx
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			m[i][j] = rotation[i][j] * scale;
}

void Transform::multiply(const Transform& parent) {
	Mtx33 M_temp;

	Mtx33P M_rot = rotation;
	
	// setup rotation
	f32 a00 = parent.m[0][0];				f32 a01 = parent.m[0][1];		f32 a02 = parent.m[0][2];
	M_temp[0][0] = a00 * M_rot[0][0] +		a01 * M_rot[1][0] +				a02 * M_rot[2][0];
	M_temp[0][1] = a00 * M_rot[0][1] +		a01 * M_rot[1][1] +				a02 * M_rot[2][1];
	M_temp[0][2] = a00 * M_rot[0][2] +		a01 * M_rot[1][2] +				a02 * M_rot[2][2];

	f32 a10 = parent.m[1][0];				f32 a11 = parent.m[1][1];		f32 a12 = parent.m[1][2];
	M_temp[1][0] = a10 * M_rot[0][0] +		a11 * M_rot[1][0] +				a12 * M_rot[2][0];
	M_temp[1][1] = a10 * M_rot[0][1] +		a11 * M_rot[1][1] +				a12 * M_rot[2][1];
	M_temp[1][2] = a10 * M_rot[0][2] +		a11 * M_rot[1][2] +				a12 * M_rot[2][2];

	f32 a20 = parent.m[2][0];				f32 a21 = parent.m[2][1];		f32 a22 = parent.m[2][2];
	M_temp[2][0] = a20 * M_rot[0][0] +		a21 * M_rot[1][0] +				a22 * M_rot[2][0];
	M_temp[2][1] = a20 * M_rot[0][1] +		a21 * M_rot[1][1] +				a22 * M_rot[2][1];
	M_temp[2][2] = a20 * M_rot[0][2] +		a21 * M_rot[1][2] +				a22 * M_rot[2][2];

	// setup scale
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			m[i][j] = M_temp[i][j] * scale;

	//scale *= parent.scale;

	// parent position
	f32 px = parent.m[0][3];
	f32 py = parent.m[1][3];
	f32 pz = parent.m[2][3];

	// local position
	f32 lx = position.x;
	f32 ly = position.y;
	f32 lz = position.z;

	// setup world space position
	m[0][3] = px + lx * parent.m[0][0] + ly * parent.m[0][1] + lz * parent.m[0][2];
	m[1][3] = py + lx * parent.m[1][0] + ly * parent.m[1][1] + lz * parent.m[1][2];
	m[2][3] = pz + lx * parent.m[2][0] + ly * parent.m[2][1] + lz * parent.m[2][2];
}

void EntityLibrary::update() {
	TempAllocator<MAX_OBJS*4> allocator1;
	Hash<u32> hash_childrenID(allocator1);

	TempAllocator<MAX_OBJS*4> allocator2;
	Queue<u32> queue_parents(allocator2);

	/* first, we build queue with the roots and a multi-hash with the children. For example, lets say we have the following hierarchy:
			  1                 6
			 / \			  / | \
		    2   5			 7	8  9
		   / \				    | 
		  3   4				    10

		1 and 6 have no parent, therefore, they go to the queue Q:
		Q = {1, 6}

		Each children goes into the hash H, where its parent is the hash key:
		H(1) = {2, 5}
		H(2) = {3, 4}
		H(6) = {7, 8, 9}
		H(8) = {10}
	*/	
	
	for (u32 i = 0; i < mNumEntities; i++) {
		u32 parent = mEntities[i].parentID;
		//System::Log(L"p(%d)=%d", i+1, parent+1);

		if (parent == UINT32_MAX)
			queue::push_back(queue_parents, i);
		else
			multi_hash::insert(hash_childrenID, parent, i);
	}

	// build the array with the indices in the right order
	TempAllocator<MAX_OBJS*4> allocator3;
	Array<u32> array_indicesInOrder(allocator3);

	//System::Log(L"UPDATE ORDER:");
	// the "queue" is actually a stack. As chilren are discovered, they are pushed into the same end they're removed later.
	while (queue::size(queue_parents) != 0) {
		// pops the entity's ID from the stack...
		u32 id = queue_parents[0];
		queue::pop_front(queue_parents);

		// ... and insert it into the final array
		array::push_back(array_indicesInOrder, id);

		// if the entity has any children, insert push them into the stack
		const typename Hash<u32>::Entry* e = multi_hash::find_first(hash_childrenID, id);
		while (e) {
			queue::push_front(queue_parents, e->value);
			e = multi_hash::find_next(hash_childrenID, e);
		}
		
		//System::Log(L">>%d", id+1);
	}

	// finally, the transforms are in the right order: parents always come before children. Now traverse all of them and update the xforms.
	for (u16 i = 0; i < mNumEntities; i++) {
		u16 id = array_indicesInOrder[i] & INDEX_MASK;
		Entity& ent = entity(id);
		Transform& xForm = mTransforms[id];
		u32 parentID = ent.parentID;

		// root entity: no parent, update the transform directly
		if (parentID == UINT32_MAX) {
			xForm.update();
		}
		else {
			Transform& parentXform = mTransforms[parentID & INDEX_MASK];
			xForm.multiply(parentXform);
		}
	}
}

