// Zaher abdulatif abdurab babker (C) 2023-2024
#pragma once
#include <string>

// hopefully, we will expand this to linux and *nix (IMacOs is shit, might never support)
#ifndef _WIN32
#error GARTER is a windows specific library
#endif


#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#define NOMINMAX
	#include <Windows.h>
	#include <objidl.h>
	#include <gdiplus.h>

	#pragma comment (lib,"Gdiplus.lib")
#endif

#ifdef GARTER_STATIC
	#define GARTER_API
#else
	#ifdef GARTER_EXPORTS
		#define GARTER_API __declspec(dllexport)
	#else
		#define GARTER_API __declspec(dllimport)
	#endif
#endif



namespace gart
{
	enum class EventType : WORD
	{
		Raw = WM_NULL, // <- other undeclared events (like WM_NC* stuff and others)
		
		Moved = WM_MOVE,
		Resized = WM_SIZE,

		Paint = WM_PAINT,

		Focused = WM_SETFOCUS,
		UnFocused = WM_KILLFOCUS,
		ShowWindow = WM_SHOWWINDOW,

		MouseMoved = WM_MOUSEMOVE,
		MouseDown = WM_LBUTTONDOWN | WM_RBUTTONDOWN | WM_MBUTTONDOWN,
		MouseUp = WM_LBUTTONUP | WM_RBUTTONUP | WM_MBUTTONUP,
		MouseClick = WM_LBUTTONDBLCLK | WM_RBUTTONDBLCLK | WM_MBUTTONDBLCLK,

		Minimized = WM_COMPACTING,

		UserChanged = WM_USERCHANGED,

		KeyDown = WM_KEYDOWN,
		KeyUp = WM_KEYUP,

		// always invoked before window closing
		Exit = (WORD)-1,
	};

	enum class MouseButton : WORD
	{
		Left = WM_LBUTTONDOWN,
		Right = WM_RBUTTONDOWN,
		Middle = WM_MBUTTONDOWN,
	};

	union Event
	{
		struct RAW 
		{
			UINT m;
			WPARAM wp;
			LPARAM lp;
		} raw;

		struct UPostionEvent
		{
			WORD x, y;
		} mousepos;

		struct IPostionEvent
		{
			SHORT x, y;
		} windowpos;

		struct ISize
		{
			WORD w, h;
		} windowsize;

		MouseButton mouse_button;

		struct IKey
		{
			static constexpr WORD ScancodeMask = static_cast<WORD>(~0x8000U);

			// first 15 bits is the scanecode, wich is hardware dependent
			// last bit is the previouse state, 1 if was pressed and 0 otherwise
			WORD state;
			// keycode given by windows
			WORD keycode;

			inline bool previouse_state() const noexcept {
				return state & ~ScancodeMask;
			}

			inline WORD scancode() const noexcept {
				return state & ScancodeMask;
			}

		} keypress;

	};
	
	class GARTER_API Window;
	typedef void(*CallbackProc)(Window *, EventType, const Event *);

	class GARTER_API Window
	{
	public:
		Window( const std::wstring &title, CallbackProc proc );
		
		Window( const Window & ) = delete;
		Window( Window && ) noexcept = delete;
		Window &operator=( const Window & ) = delete;
		Window &operator=( Window && ) noexcept = delete;

		~Window();

		// will read the message queue until it's empty
		// reading and invoking events
		void poll();

		// closing window will release it's HWND
		// will trigger an Exit event to be called
		// closing a window with null HWND will cause errors
		void close();

		// returns true if the window has a non-nullptr HWND
		// to implement mainloops as 'while (window) ...'
		inline operator bool() const noexcept {
			return m_hwnd != nullptr;
		}

		inline HWND get_hwnd() const noexcept {
			return m_hwnd;
		}

		inline CallbackProc get_callback() const noexcept {
			return m_callproc;
		}

		inline void set_callback( CallbackProc proc ) noexcept {
			m_callproc = proc;
		}

		const std::wstring &title() const;
		LONG width() const;
		LONG height() const;
		
		SIZE size() const; // <H, W>
		POINT position() const; // <X, Y>
		RECT rect() const; // <X, Y>

		float dpi() const;

		void set_title( const std::wstring &title );
		void set_size( int h, int w );
		void set_position( int x, int y );
		void set_rect( int x, int y, int w, int h );
		


		class GARTER_API _WND_;
	private:
		HWND m_hwnd;
		CallbackProc m_callproc;
		MSG m_msg;
		Event m_event;
	};
}