#define NOMINMAX
#include <windows.h>
#include "app_2048.h"

int WINAPI wWinMain(HINSTANCE instance,
					HINSTANCE /*prev_isnstance*/,
					LPWSTR /*command_line*/,
					int show_command) {
	app_2048 app{ instance };
	return app.run(show_command);
}
					