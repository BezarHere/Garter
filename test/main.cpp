#include <iostream>
#define GARTER_STATIC
#include <garter.h>

#include <wingdi.h>
using namespace gart;

void wnd( const gart::Window *, EventType type, const Event *ev ) {
	if (type == EventType::Resized) {
		std::cout << "resized: " << ev->windowpos.x << ' ' << ev->windowpos.y << '\n';
	}
}

int main() {
	Window window{ L"Hello", wnd };
	while (true)
	{ 
		window.poll();
	}
}
