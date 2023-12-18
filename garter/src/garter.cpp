#include "pch.h"
#include "garter.h"
using namespace gart;

// is it worth it to replace with a situation specific aproach?
static std::map<HWND, Window *> g_WinHandles{};
// any window wich is currently make the window
// why: the window can send messages before returning the handle, so this messages gotta
//         get handled someway
static Window *g_WindowPremake;

static FORCEINLINE HINSTANCE get_instance() {
	return GetModuleHandle( NULL );
}

static FORCEINLINE LRESULT CALLBACK WndProc( const HWND hwnd, const UINT msg, const LPARAM lp, const WPARAM wp ) {
	decltype(g_WinHandles)::iterator pos = g_WinHandles.find(hwnd);
	Window *wnd = nullptr;
	
	if (pos == g_WinHandles.end())
	{
		if (g_WindowPremake)
			wnd = g_WindowPremake;
		else
			return E_UNEXPECTED;
	}
	wnd = pos->second;

	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return NOERROR; // <- no reason to contniue, right?
	default:
		break;
	}

	return NOERROR;
}

static WNDCLASS MakeWindowClass( const wchar_t *name ) {
	WNDCLASS wndc;
	wndc.lpfnWndProc = reinterpret_cast<WNDPROC>(WndProc);
	wndc.hInstance = get_instance();
	wndc.lpszClassName = name;

	wndc.style = CS_HREDRAW | CS_VREDRAW;
	wndc.cbClsExtra = 0;
	wndc.cbWndExtra = 0;
	// wndc.hIcon          = LoadIcon(hinst, MAKEINTRESOURCE(IDI_WINDOWTEMPLATE));
	wndc.hCursor = LoadCursor( NULL, IDC_ARROW );
	wndc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	//wndc.lpszMenuName = MAKEINTRESOURCEA( IDC_WINDOWTEMPLATE );

	return wndc;
}

static ATOM RegisterWinClass( const WNDCLASS &wndc ) {
	return RegisterClass( &wndc );
}

static HWND RegisterWindow( HWND hwnd, Window *window ) {
	// huh? you can't make a window and create it without first setting it to the premake ptr
	// this may blow up for the amount of unregistered events
	if (g_WindowPremake != window)
	{
		// dump message
		std::_Xruntime_error( "Invalid create of a window, didn't expected a window" );
	}



}

static HWND InitWindow( const wchar_t *const wnd_classname, const char *const wnd_title ) {
	HINSTANCE hinst = GetModuleHandle( NULL );
	WNDCLASSA wc = { 0 };

	{
		wc.lpfnWndProc = reinterpret_cast<WNDPROC>(WndProc);
		wc.hInstance = hinst;
		wc.lpszClassName = wnd_classname;

		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		// wc.hIcon          = LoadIcon(hinst, MAKEINTRESOURCE(IDI_WINDOWTEMPLATE));
		wc.hCursor = LoadCursor( NULL, IDC_ARROW );
		wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		//wc.lpszMenuName = MAKEINTRESOURCEA( IDC_WINDOWTEMPLATE );
		wc.lpszClassName = wnd_classname;
	}

	RegisterClassA( &wc );

	HWND wnd = CreateWindowA(
		wc.lpszClassName,
		wnd_title,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, // x, y, w, h
		NULL, // parent
		NULL,
		hinst,
		NULL
	);
	if (wnd == nullptr)
	{
		std::_Xruntime_error( "failed to create window" );
	}

	BOOL shown = ShowWindow( wnd, 10 );
	printf( "win show returned %d\n", shown );
	BOOL updated = UpdateWindow( wnd );
	printf( "win update returned %d\n", updated );

	//MSG msg;

	//while (GetMessageA( &msg, wnd, 0, 0 ))
	//{
	//	TranslateMessage( &msg );
	//	DispatchMessageA( &msg );
	//}

	return wnd;
}

namespace gart
{

	Window::Window( const std::wstring &title )
		: m_hwnd{ nullptr } {
		m_hwnd = RegisterWindow( InitWindow( "hello", title.c_str() ), this );
	}

}
