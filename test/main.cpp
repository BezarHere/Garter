#include <iostream>
#define GARTER_STATIC
#include <garter.h>
#include <thread>

#include <windows.h>


using namespace Gdiplus;
using namespace gart;

static bool moving = false;

void wnd( gart::Window *wnd, EventType type, const Event *ev ) {
	if (type == EventType::Exit) {
		std::cout << "exit!\n";
	}

	if (type == EventType::Moved)
	{
		moving = true;
	}
	else if (type == EventType::Moving)
	{
		moving = false;
	}

	else if (!moving && type == EventType::Paint)
	{
		GdiplusStartupInput gdiplusStartupInput;
		ULONG_PTR gdiplusToken;
		GdiplusStartup( &gdiplusToken, &gdiplusStartupInput, NULL );

		{
			Rect rct = wnd->rect();
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint( wnd->get_hwnd(), &ps );
			Pen pen{ Color( 18, 18, 18 ), 4.f };
			Graphics g{ hdc };

			rct.X -= 1;
			rct.Width += 1;
			

			g.SetSmoothingMode( SmoothingModeHighQuality );

			g.FillRectangle( pen.GetBrush(), rct);

			pen.SetColor( Color( 180, 180, 18 ) );

			g.DrawLine( &pen, 0, 0, rct.Width, rct.Height );
			EndPaint( wnd->get_hwnd(), &ps );
		}

		GdiplusShutdown( gdiplusToken );
	}

	if (type == EventType::KeyDown)
	{
		if (ev->keypress.keycode == 'A')
		{
			wnd->set_size( 256, 256 );
			wnd->set_title( L"A PRESSED" );
		}
		if (ev->keypress.keycode == 'D')
		{
			wnd->set_position( 256, 256 );
			wnd->set_title( L"D PRESSED" );
		}
		if (ev->keypress.keycode == 'S')
		{
			wnd->set_rect( 500, 256, 600, 200 );
			wnd->set_title( L"S PRESSED" );
		}
	}

}

int main() {
	{
		Garter::enable_logging();
		Window window{ L"Hello", wnd };
		while (window)
		{
#ifdef _DEBUG
			std::this_thread::sleep_for( std::chrono::milliseconds( 200 ) ); // 5 fps
#else
			std::this_thread::sleep_for( std::chrono::milliseconds( 16 ) ); // 60 fps
#endif // _DEBUG

			std::cout << "polling\n";
			window.poll();
		}
	}
	std::cout << "\nCompleted mainloop!\n";
	std::cin.get();
}
