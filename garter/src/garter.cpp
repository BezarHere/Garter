#include "pch.h"
#include "garter.h"
#include "wm.h"
using namespace gart;

static FORCEINLINE HINSTANCE GetOwenHInstance();
#define SENDEVENT(type) (Window::_WND_::SendEvent(wnd, type))
#define BREAK_EVENT(type) SENDEVENT(type); break

#define MSGPROC_SUCCESS FALSE
#define MSGPROC_FAILURE TRUE

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
		w->m_callproc( w, type, &w->m_event );
	}

	static FORCEINLINE LRESULT CALLBACK \
		WndProc( const HWND hwnd, const UINT msg, const WPARAM wp, const LPARAM lp );

	static FORCEINLINE LRESULT \
		WndProcProcessor( Window *wnd, const UINT msg, const WPARAM wp, const LPARAM lp );

};

static FORCEINLINE HINSTANCE GetOwenHInstance() {
	return GetModuleHandle( NULL );
}

FORCEINLINE LRESULT CALLBACK \
Window::_WND_::WndProc( const HWND hwnd, const UINT msg, const WPARAM wp, const LPARAM lp ) {

	decltype(g_WinHandles)::iterator pos = g_WinHandles.find( hwnd );
	Window *wnd = nullptr;

	if (pos == g_WinHandles.end())
	{
		if (g_WindowPremake)
		{
			wnd = g_WindowPremake;

			// we are making a window (those g_WindowPremake != nullptr) some messages will be send before returning thr HWND
			// there for the premake window will have a null hwnd
			if (!wnd->m_hwnd)
				wnd->m_hwnd = hwnd;
		}
		else
		{
#ifdef _DEBUG
			fprintf( stderr, "unexpected message from hwnd [%p]: msg [%x] with wp=%llx & lp=%llx\n", hwnd, msg, wp, lp );
#endif // _DEBUG

			return MSGPROC_FAILURE;
		}
	}
	else
	{
		wnd = pos->second;
	}

#ifdef _DEBUG
	//printf( "%x with wp=0x%llX lp=0x%llX\n", msg, wp, lp );
#endif

	if (!wnd->m_callproc)
		return DefWindowProc( hwnd, msg, wp, lp );

	return WndProcProcessor( wnd, msg, wp, lp );
}

FORCEINLINE LRESULT \
Window::_WND_::WndProcProcessor( Window *wnd, const UINT msg, const WPARAM wp, const LPARAM lp ) {
	Event *event = &wnd->m_event;
	switch (msg)
	{
#pragma region Mouse Event
	case WM_LBUTTONDOWN:
		event->mouse_button = MouseButton::Left;
		BREAK_EVENT( EventType::MouseDown );
	case WM_RBUTTONDOWN:
		event->mouse_button = MouseButton::Right;
		BREAK_EVENT( EventType::MouseDown );
	case WM_MBUTTONDOWN:
		event->mouse_button = MouseButton::Middle;
		BREAK_EVENT( EventType::MouseDown );

	case WM_LBUTTONUP:
		event->mouse_button = MouseButton::Left;
		BREAK_EVENT( EventType::MouseUp );
	case WM_RBUTTONUP:
		event->mouse_button = MouseButton::Right;
		BREAK_EVENT( EventType::MouseUp );
	case WM_MBUTTONUP:
		event->mouse_button = MouseButton::Middle;
		BREAK_EVENT( EventType::MouseUp );

	case WM_LBUTTONDBLCLK:
		event->mouse_button = MouseButton::Left;
		BREAK_EVENT( EventType::MouseClick );
	case WM_RBUTTONDBLCLK:
		event->mouse_button = MouseButton::Right;
		BREAK_EVENT( EventType::MouseClick );
	case WM_MBUTTONDBLCLK:
		event->mouse_button = MouseButton::Middle;
		BREAK_EVENT( EventType::MouseClick );

	case WM_MOUSEMOVE:
		event->mousepos = { LOWORD( lp ), HIWORD( lp ) };
		BREAK_EVENT( EventType::MouseMoved );
#pragma endregion

	case WM_MOVE:
		event->windowpos = { (SHORT)LOWORD( lp ), (SHORT)HIWORD( lp ) };
		BREAK_EVENT( EventType::Moved );
	case WM_SIZE:
		event->windowsize = { LOWORD( lp ), HIWORD( lp ) };
		BREAK_EVENT( EventType::Resized );
	case WM_PAINT:
		{
			HWND hwnd = wnd->m_hwnd;
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint( hwnd, &ps );
			HBRUSH bsh = CreateSolidBrush( RGB( 255, 44, 129 ) );
			// TODO: Add any drawing code that uses hdc here...

			FillRect( hdc, &ps.rcPaint, bsh );


			EndPaint( hwnd, &ps );
			
		}
		BREAK_EVENT( EventType::Paint );
	case WM_DESTROY:
		PostQuitMessage( 0 );
		SENDEVENT( EventType::Exit );
		return MSGPROC_SUCCESS; // <- no reason to contniue, right?
	default:
		event->raw.m = msg;
		event->raw.lp = lp;
		event->raw.wp = wp;
#ifdef _DEBUG
		printf( "Unhandled msg: %s with wp=0x%llX lp=0x%llX\n", get_wm_name(msg), wp, lp);
#endif
		SENDEVENT( EventType::Raw );
		return DefWindowProc( wnd->m_hwnd, msg, wp, lp );
	}

	return MSGPROC_SUCCESS;
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

		while (GetMessage( &m_msg, m_hwnd, 1, 0x400 ))
		{
			TranslateMessage( &m_msg );
			DispatchMessage( &m_msg );
		}
	}

}
