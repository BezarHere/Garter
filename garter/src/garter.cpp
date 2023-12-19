#include "pch.h"
#include "garter.h"
using namespace gart;

static FORCEINLINE HINSTANCE GetOwenHInstance();

// is it worth it to replace with a situation specific aproach?
static std::map<HWND, Window *> g_WinHandles{};
// any window wich is currently make the window
// why: the window can send messages before returning the handle, so this messages gotta
//         get handled someway
static Window *g_WindowPremake;

struct WCData
{
	WNDCLASSEX &wndc;
	LPCWSTR title;
	struct Style
	{
		DWORD ex_style = 0, style = 0;
		int x = CW_USEDEFAULT, y = CW_USEDEFAULT;
		int w = CW_USEDEFAULT, h = CW_USEDEFAULT;
	} style;
	HWND parent = nullptr;
	HMENU menu = nullptr;
	HINSTANCE instance = GetOwenHInstance();
	LPVOID param = nullptr;

	inline WCData( WNDCLASSEX &wndc, LPCWSTR title )
		: wndc{ wndc }, title{ title } {

	}

	FORCEINLINE HWND activate() const {
		return CreateWindowEx(
			this->style.ex_style,
			this->wndc.lpszClassName,
			this->title,
			this->style.style,
			this->style.x, this->style.y, this->style.w, this->style.h, // x, y, w, h
			this->parent, this->menu,
			this->instance,
			this->param
		);
	}

};

class Window::_WND_
{
	_WND_() = delete;
	~_WND_() = delete;
public:
	static FORCEINLINE void SendEvent( const Window *w, EventType type ) {
		w->m_callproc(w, type, &w->m_event);
	}

	static FORCEINLINE \
	LRESULT CALLBACK WndProc( const HWND hwnd,
														const UINT msg,
														const WPARAM wp,
														const LPARAM lp  );

};

static FORCEINLINE HINSTANCE GetOwenHInstance() {
	return GetModuleHandle( NULL );
}

FORCEINLINE LRESULT CALLBACK \
Window::_WND_::WndProc( const HWND hwnd,
												const UINT msg ,
												const WPARAM wp,
												const LPARAM lp  ) {

	decltype(g_WinHandles)::iterator pos = g_WinHandles.find(hwnd);
	Window *wnd = nullptr;
	
	if (pos == g_WinHandles.end())
	{
		if (g_WindowPremake)
		{
			wnd = g_WindowPremake;
		}
		else
		{
#ifdef _DEBUG
			fprintf( stderr, "unexpected message from hwnd [%p]: msg [%x] with wp=%llx & lp=%llx\n", hwnd, msg, wp, lp );
#endif // _DEBUG

			return (LRESULT)FALSE;
		}
	}
	else
	{
		wnd = pos->second;
	}
#ifdef _DEBUG
	printf( "%p: msg %x and wp=0x%llX lp=0x%llX\n", hwnd, msg, wp, lp );
#endif

	switch (msg)
	{
	case WM_SIZE:
		wnd->m_event.windowsize = { LOWORD(lp), HIWORD(lp) };
		SendEvent( wnd, EventType::Resized );
		break;
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint( hwnd, &ps );
			HBRUSH bsh = CreateSolidBrush(RGB(255, 44, 129));
			// TODO: Add any drawing code that uses hdc here...

			FillRect( hdc, &ps.rcPaint, bsh );

			
			EndPaint( hwnd, &ps );
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return (LRESULT)TRUE; // <- no reason to contniue, right?

	default:
		return DefWindowProc( hwnd, msg, wp, lp );
	}

	return (LRESULT)TRUE;
}

static WNDCLASSEX MakeWindowClass( const wchar_t *name ) {
	WNDCLASSEX wndc = { 0 };
	wndc.lpfnWndProc = Window::_WND_::WndProc;

	wndc.hInstance = GetOwenHInstance();
	wndc.lpszClassName = name;

	wndc.style = CS_HREDRAW | CS_VREDRAW;
	wndc.cbClsExtra = 0;
	wndc.cbWndExtra = 0;
	// wndc.hIcon          = LoadIcon(hinst, MAKEINTRESOURCE(IDI_WINDOWTEMPLATE));
	wndc.hCursor = LoadCursor( NULL, IDC_ARROW );
	wndc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	//wndc.lpszMenuName = MAKEINTRESOURCEA( IDC_WINDOWTEMPLATE );

	wndc.cbSize = sizeof( WNDCLASSEX );
	return wndc;
}

static ATOM RegisterWindowClass( const WNDCLASSEX &wndc ) {
	return RegisterClassEx( &wndc );
}

static HWND RegisterWindow( HWND hwnd, Window *window ) {
	// huh? you can't make a window and create it without first setting it to the premake ptr
	// this may blow up for the amount of unregistered events
	if (g_WindowPremake != window)
	{
		// dump message
		std::_Xruntime_error( "Invalid create of a window, didn't expected a window" );
	}

	g_WinHandles.insert( { hwnd, window } );

	g_WindowPremake = nullptr;

	return hwnd;
}


// will also destroy the window
static void UnregisterWindow( const Window *window ) {
	g_WinHandles.erase( window->get_hwnd() );
	DestroyWindow( window->get_hwnd() );
}

//static HWND InitWindow( const wchar_t *const wnd_classname, const char *const wnd_title ) {
//	HINSTANCE hinst = GetModuleHandle( NULL );
//	WNDCLASSA wc = { 0 };
//
//	{
//		wc.lpfnWndProc = reinterpret_cast<WNDPROC>(WndProc);
//		wc.hInstance = hinst;
//		wc.lpszClassName = wnd_classname;
//
//		wc.style = CS_HREDRAW | CS_VREDRAW;
//		wc.cbClsExtra = 0;
//		wc.cbWndExtra = 0;
//		// wc.hIcon          = LoadIcon(hinst, MAKEINTRESOURCE(IDI_WINDOWTEMPLATE));
//		wc.hCursor = LoadCursor( NULL, IDC_ARROW );
//		wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
//		//wc.lpszMenuName = MAKEINTRESOURCEA( IDC_WINDOWTEMPLATE );
//		wc.lpszClassName = wnd_classname;
//	}
//
//	RegisterClassA( &wc );
//
//	HWND wnd = CreateWindowA(
//		wc.lpszClassName,
//		wnd_title,
//		WS_OVERLAPPEDWINDOW,
//		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, // x, y, w, h
//		NULL, // parent
//		NULL,
//		hinst,
//		NULL
//	);
//	if (wnd == nullptr)
//	{
//		std::_Xruntime_error( "failed to create window" );
//	}
//
//	BOOL shown = ShowWindow( wnd, 10 );
//	printf( "win show returned %d\n", shown );
//	BOOL updated = UpdateWindow( wnd );
//	printf( "win update returned %d\n", updated );
//
//	//MSG msg;
//
//	//while (GetMessageA( &msg, wnd, 0, 0 ))
//	//{
//	//	TranslateMessage( &msg );
//	//	DispatchMessageA( &msg );
//	//}
//
//	return wnd;
//}

namespace gart
{

	Window::Window( const std::wstring &title, CallbackProc proc )
		: m_hwnd{ nullptr }, m_callproc{ proc }, m_msg{} {
		
		// modify if you want
		WNDCLASSEX wndc = MakeWindowClass( title.c_str() );

		// calls the winapi RegisterClass
		// TODO: check for errors
		(void)RegisterWindowClass( wndc );
		
		WCData wcdata{ wndc, title.c_str() };

		wcdata.style.style = WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU;

		g_WindowPremake = this;

		m_hwnd = wcdata.activate();
		
		// will return m_hwnd
		(void)RegisterWindow( m_hwnd, this );
		
		BOOL shown = ShowWindow( m_hwnd, SW_SHOW );
		printf( "win show returned %d\n", shown );
		BOOL updated = UpdateWindow( m_hwnd );
		printf( "win update returned %d\n", updated );
	}

	void Window::poll() {
		
		while (GetMessage( &m_msg, m_hwnd, 1, -1 ))
		{
			TranslateMessage( &m_msg );
			DispatchMessage( &m_msg );
		}
	}

}
