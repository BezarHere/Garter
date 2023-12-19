#include <iostream>
#define GARTER_STATIC
#include <garter.h>

#include <wingdi.h>
using namespace gart;

void wnd( const gart::Window *, EventType type, const Event *ev ) {
	if (type == EventType::Moved) {
		std::cout << "moved: " << ev->windowpos.x << ' ' << ev->windowpos.y << '\n';
	}
	if (type == EventType::MouseMoved) {
		std::cout << "mmouse: " << ev->mousepos.x << ' ' << ev->mousepos.y << '\n';
	}
}

int main() {
	Window window{ L"Hello", wnd };
	while (true)
	{ 
		window.poll();
	}
}
