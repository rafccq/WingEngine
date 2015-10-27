#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cstdarg>

#include <wiiuse/wpad.h>
#include <fat.h>

#include <gctypes.h>

#include "Renderer.h"
#include "System.h"

#define PLATFORM_BIG_ENDIAN

#include "foundation\collection_types.h"
#include "foundation\temp_allocator.h"
#include "foundation\hash.h"
#include "foundation\memory.h"

using namespace foundation;

namespace System {
	Renderer g_Renderer;

	void Init() {
		WPAD_Init();

		if (false == fatInitDefault()) {
			Assert("fatInitDefault()", "Could not init fat device, aborting...", __FILE__, __LINE__);
		}

		//initNet();

		g_Renderer.init();
	}

	void Assert(const char* expression, const char* message, const char* filename, int line) {
		g_Renderer.setFBIndex(0);
		g_Renderer.doneRender();

		filename = strrchr(filename, '/') + 1;

		// ---- temp: log to file
		FILE* logFile = fopen("_WING_LOG.txt", "w");
		fprintf(logFile, "Assertion failed: %s\n", expression);
		fprintf(logFile, "Message: %s\n", message);
		fprintf(logFile, "LOCATION: %s, line %d\n", filename, line);
		fclose(logFile);


		printf("\x1b[4;0H");

		printf("Assertion failed: %s\n", expression);
		printf("Message: %s\n", message);

		printf("LOCATION: %s, line %d\n", filename, line);
		printf("Press 'HOME' to exit.\n");

		while (1) {
			WPAD_ScanPads();

			if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME) {
				printf("\x1b[10;0H");
				exit(0);
			}
		}
	}

	void Log(wchar_t* fmt, ...) {
		wchar_t* text = g_Renderer.getConsole().getCurrentLine();
		
		va_list args;

		va_start(args, fmt);
			vswprintf(text, 128, fmt, args);
		va_end(args);

		g_Renderer.getConsole().increaseLines();
	}

	void LogClear() {
		g_Renderer.getConsole().clear();
	}

	Renderer* GetRenderer() {
		return &g_Renderer;
	}

}