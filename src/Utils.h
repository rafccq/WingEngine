#pragma once

#include <gctypes.h>
#include <cstdio>

namespace Utils {
	u32 FileSize(FILE* f);
	u8* ReadFileBytes(const char* filename, u32* out_size);
}

