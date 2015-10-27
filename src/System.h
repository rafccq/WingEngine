#pragma once

#include <cwchar>

#include "Renderer.h"

// assert
#ifdef WII_DEBUG
	#define WING_ASSERT(e, msg) \
	if (!(e)) System::Assert(#e, msg, __FILE__, __LINE__)
#else
	#define WING_ASSERT(e, msg)
#endif

namespace System {
	void Init();
	void Assert(const char* expression, const char* message, const char* filename, int line);
	void Log(wchar_t* fmt, ...);
	void LogClear();

	Renderer* GetRenderer();
}

