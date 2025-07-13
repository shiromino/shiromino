#include "Delay.h"
#include "SDL_timer.h"

#ifdef _WIN32
// By fully containing Windows-specific code in here, it doesn't pollute the
// rest of the code with junk like the min/max defines conflicting with
// std::min/std::max.
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <iostream>
#include <cinttypes>
#include <cstdlib>

void Delay(const std::uint32_t milliseconds) {
	static HANDLE timer = NULL;
	if (timer == NULL) {
		if ((timer = CreateWaitableTimer(NULL, TRUE, NULL)) == NULL) {
            std::cerr << "Failed to initialize support for the Delay function" << std::endl;
            std::exit(EXIT_FAILURE);
		}
	}

	LARGE_INTEGER duration = { .QuadPart = 0 };
	duration.QuadPart = milliseconds * INT64_C(-10000);
	SetWaitableTimer(timer, &duration, 0, NULL, NULL, 0);
	WaitForSingleObject(timer, INFINITE);
}
#else

void Delay(const std::uint32_t milliseconds) {
    SDL_Delay(milliseconds);
}
#endif
