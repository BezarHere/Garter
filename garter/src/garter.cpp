#include "pch.h"
#include "garter.h"
#include "wm.h"
using namespace gart;

static bool g_UseLogging;

static FORCEINLINE HINSTANCE GetOwenHInstance();
#define SENDEVENT(type) (Window::_WND_::SendEvent(wnd, type))
#define BREAK_EVENT(type) SENDEVENT(type); break

#define SCREAM_LASTWINERR(context) {printf(context " FAILED WITH LAST ERR OF %d\n", GetLastError()); fflush(stdout);}

#define MSGPROC_SUCCESS FALSE
#define MSGPROC_FAILURE TRUE

#define UNUSED_LOCAL(l) ((void)l)

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
	static FORCEINLINE void SendEvent( Window *w, EventType type ) {
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
			if (g_UseLogging)
			{
				fprintf( stderr, "UNEXPECTED MESSAGE: [%p]: msg [%s] with wp=%llx & lp=%llx\n", hwnd, get_wm_name( msg ), wp, lp );
			}
#endif // _DEBUG

			return MSGPROC_FAILURE;
		}
	}
	else
	{
		wnd = pos->second;
	}

#ifdef _DEBUG
	if (g_UseLogging)
	{
		//printf( "%x with wp=0x%llX lp=0x%llX\n", msg, wp, lp );
	}
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
#pragma region Mouse input handling
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

#pragma region Keyboard input handling
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
		{
			WORD vk_code = LOWORD( wp );

			const WORD key_flags = HIWORD( lp );
			WORD scancode = LOBYTE( key_flags );

			const BOOL is_extended = (key_flags & KF_EXTENDED) == KF_EXTENDED; // extended - key flag, 1 if scancode has 0xE0 prefix

			if (is_extended)
			{
				scancode = MAKEWORD( scancode, 0xE0 );
			}

			const BOOL was_key_down = (key_flags & KF_REPEAT) == KF_REPEAT; // previous key - state flag, 1 on autorepeat
			const WORD repeat_n = LOWORD( lp ); // repeat count, > 0 if several keydown messages was combined into one message
			const BOOL is_key_released = (key_flags & KF_UP) == KF_UP; // transition - state flag, 1 on keyup
			
			UNUSED_LOCAL( repeat_n );
			UNUSED_LOCAL( is_key_released );
			
			// if we want to distinguish these keys:
			switch (vk_code)
			{
			case VK_SHIFT: // converts to VK_LSHIFT or VK_RSHIFT
			case VK_CONTROL: // converts to VK_LCONTROL or VK_RCONTROL
			case VK_MENU: // converts to VK_LMENU or VK_RMENU
				vk_code = LOWORD( MapVirtualKeyW( scancode, MAPVK_VSC_TO_VK_EX ) );
				break;
			}			event->keypress.keycode = vk_code;			event->keypress.state = scancode & event->keypress.ScancodeMask;			if (was_key_down)				event->keypress.state &= event->keypress.ScancodeMask;			else				event->keypress.state |= ~event->keypress.ScancodeMask;		}
		// keyup and syskeyup are odd (for window 0x0A00)
		BREAK_EVENT( msg & 1 ? EventType::KeyUp : EventType::KeyDown );
#pragma endregion

#pragma region Window transformations
	case WM_MOVING:
		event->windowmoving.rect = reinterpret_cast<PRECT>(lp);
		BREAK_EVENT( EventType::Moving );
	case WM_MOVE:
		event->windowpos = { (SHORT)LOWORD( lp ), (SHORT)HIWORD( lp ) };
		BREAK_EVENT( EventType::Moved );
	
	case WM_SIZING:
		event->windowresizing.rect = reinterpret_cast<PRECT>(lp);
		event->windowresizing.resized_edge = static_cast<Edge>(wp);
		BREAK_EVENT( EventType::Moving );
	case WM_SIZE:
		event->windowsize = { LOWORD( lp ), HIWORD( lp ) };
		BREAK_EVENT( EventType::Resized );

#pragma endregion

	case WM_PAINT:
		BREAK_EVENT( (EventType)msg );

	case WM_DESTROY:
		//SENDEVENT( EventType::Exit );
		/// VVVV Will send the Exit event type
		wnd->close();
		return MSGPROC_SUCCESS; // <- no reason to contniue, right?
	default:
		event->raw.m = msg;
		event->raw.lp = lp;
		event->raw.wp = wp;
#ifdef _DEBUG
		if (g_UseLogging)
		{
			printf( "Unhandled: [%s] with wp=0x%llX lp=0x%llX\n", get_wm_name( msg ), wp, lp );
		}
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

		wcdata.style.style = WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU | WS_THICKFRAME;

		g_WindowPremake = this;

		m_hwnd = wcdata.activate();

		// will return m_hwnd
		(void)RegisterWindow( m_hwnd, this );

		BOOL shown = ShowWindow( m_hwnd, SW_SHOW );

#ifdef _DEBUG
		if (g_UseLogging)
		{
			printf( "win show returned %d\n", shown );
		}
#endif

		BOOL updated = UpdateWindow( m_hwnd );

#ifdef _DEBUG
		if (g_UseLogging)
		{
			printf( "win update returned %d\n", updated );
		}
#endif // _DEBUG
	}

	Window::~Window() {
		if (m_hwnd)
		{
			close();
		}
	}

	void Window::poll() {

		while (PeekMessage( &m_msg, m_hwnd, 1, 0x400, PM_REMOVE ))
		{
			TranslateMessage( &m_msg );
			DispatchMessage( &m_msg );
		}
	}

	void Window::close() {
		PostQuitMessage( 0 );
		_WND_::SendEvent( this, EventType::Exit );
		UnregisterWindow( this );
		m_hwnd = nullptr;
	}

#pragma warning(push)
#pragma warning(disable: 4172)
	static WCHAR WindowTitleStaticBuffer[ 1024 ] = { 0 }; // <- can't this overflow? or be tampered with
	const std::wstring &Window::title() const {
		(void)GetWindowText( m_hwnd, WindowTitleStaticBuffer, 1023 ); // <- does this account null termination?
		return std::wstring( WindowTitleStaticBuffer );
	}
#pragma warning(pop)

	LONG Window::width() const {
		RECT r;
		GetClientRect( m_hwnd, &r );
		return r.right - r.left;
	}

	LONG Window::height() const {
		RECT r;
		GetClientRect( m_hwnd, &r );
		return r.bottom - r.top;
	}

	SIZE Window::size() const {
		RECT r;
		GetClientRect( m_hwnd, &r );
		return { r.right - r.left, r.bottom - r.top };
	}

	POINT Window::position() const {
		RECT r;
		GetClientRect( m_hwnd, &r );
		return { r.left, r.top };
	}

	Gdiplus::Rect Window::rect() const {
		RECT r;
		GetClientRect( m_hwnd, &r );
		return Gdiplus::Rect( r.left, r.top, r.right - r.left, r.bottom - r.top );
	}

	float Window::dpi() const {
		return (float)GetDpiForWindow( m_hwnd );
	}

	void Window::set_title( const std::wstring &title ) {
		if (!SetWindowText( m_hwnd, title.c_str() ))
		{
			SCREAM_LASTWINERR( "SETTING THE TITLE BAR" );
		}
	}

	void Window::set_size( int x, int y ) {
		SetWindowPos(m_hwnd, nullptr, 0, 0, x, y, SWP_NOMOVE );
	}

	void Window::set_position( int x, int y ) {
		SetWindowPos( m_hwnd, nullptr, x, y, 0, 0, SWP_NOSIZE );
	}

	void Window::set_rect( int x, int y, int w, int h ) {
		SetWindowPos( m_hwnd, nullptr, x, y, w, h, 0 );
	}

#ifdef _DEBUG
	void Garter::enable_logging() {
		g_UseLogging = true;
	}

	void Garter::disable_logging() {
		g_UseLogging = false;
	}
#endif

}
