#pragma once

#include <ogc\gx.h>

struct Texture {
	u16 ID;
	GXTexObj texObj;
};

/**
	The actual Material data, which defines the visual properties of a surface.
*/

/* -------------------------------------------------------------------------------------------------
	A Material is just a container for SubMaterials, which contain the actual data. A Material 
	has at least 1 SubMaterial.
   ------------------------------------------------------------------------------------------------*/
struct Material {
	union {
		// ---- Material
		u16 ID;	// ID is also the material's position into the array where it's allocated

		// ---- SubMaterial
		u16 diffuseTexID;
	};

	union {
		// ==== Material
		u16 n_subMaterials;

		// ==== SubMaterial
		u32 colorRGBA;
	};
};

/**
|D1 D0|C3 C2 C1 C0|
|I1 I0|  

48		       32	   24	   16       8   	0
|      DIFF     |   R   |   G   |   B   |   A   |

|       ID      |     START     |   N   |xxxxxxx|
*/

