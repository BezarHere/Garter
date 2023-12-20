#include <iostream>
#define GARTER_STATIC
#include <garter.h>
#include <thread>

#include <windows.h>


using namespace Gdiplus;
using namespace gart;


void wnd( gart::Window *wnd, EventType type, const Event *ev ) {
	if (type == EventType::Exit) {
		std::cout << "exit!\n";
	}

	if (type == EventType::Paint)
	{
		GdiplusStartupInput gdiplusStartupInput;
		ULONG_PTR gdiplusToken;
		GdiplusStartup( &gdiplusToken, &gdiplusStartupInput, NULL );

		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint( wnd->get_hwnd(), &ps );
			Pen pen{ Color( 23, 55, 255 ), 20.f };
			Graphics g{ hdc };
			g.DrawLine( &pen, 20, 10, 500, 800 );
			EndPaint( wnd->get_hwnd(), &ps );
		}

		GdiplusShutdown( gdiplusToken );
	}

	if (type == EventType::KeyDown)
	{
		if (ev->keypress.keycode == 'A')
		{
			wnd->set_size( 256, 256 );
		}
		if (ev->keypress.keycode == 'D')
		{
			wnd->set_position( 256, 256 );
		}
		if (ev->keypress.keycode == 'S')
		{
			wnd->set_rect( 500, 256, 600, 200 );
		}
	}

}

int main() {
	{
		Window window{ L"Hello", wnd };
		while (window)
		{
			std::this_thread::sleep_for( std::chrono::milliseconds( 200 ) ); // 5 fps
			std::cout << "polling\n";
			window.poll();
		}
	}
	std::cout << "\nCompleted mainloop!\n";
	std::cin.get();
}
