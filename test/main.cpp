#include <iostream>
#define GARTER_STATIC
#include <garter.h>

#include <wingdi.h>
using namespace gart;

static bool window_run = true;

void wnd( const gart::Window *, EventType type, const Event *ev ) {
	if (type == EventType::Exit) {
		std::cout << "exit!\n";
		window_run = false;
	}
}

int main() {
	Window window{ L"Hello", wnd };
	while (window_run)
	{ 
		std::cout << "pooling\n";
		window.poll();
	}
}
