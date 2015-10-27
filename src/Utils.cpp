#include "Utils.h"

namespace Utils {
	u32 FileSize(FILE* f) {	
		fseek(f, 0, SEEK_END); 
		u32 size = ftell(f);
		rewind(f);

		return size;
	}

	u8* ReadFileBytes(const char* filename, u32* out_size) {
		FILE* f = fopen(filename, "r");

		// file not found: return NULL
		if (0 == f) {
			return 0;
		}

		u32 size = Utils::FileSize(f);
		u8* data = new u8[size];

		fread(data, 1, size, f);
		fclose(f);

		*out_size = size;
		return data;
	}
}