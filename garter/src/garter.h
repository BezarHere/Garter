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
		SysKeyDown = WM_SYSKEYDOWN,
		SysKeyUp = WM_SYSKEYUP,

		Exit = (WORD) - 1,
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
			// first 15 bits is the scanecode, wich is hardware dependent
			// last bit is the previouse state, 1 if was pressed and 0 otherwise
			WORD state;
			// keycode given by windows
			WORD keycode;

			inline bool previouse_state() const noexcept {
				return state & 0x8000;
			}

			inline WORD scancode() const noexcept {
				return state & ~0x8000;
			}

		} keypress;

	};
	
	class GARTER_API Window;
	typedef void(*CallbackProc)(const Window *, EventType, const Event *);

	class GARTER_API Window
	{
	public:
		Window( const std::wstring &title, CallbackProc proc );
		void poll();

		inline HWND get_hwnd() const noexcept {
			return m_hwnd;
		}

		inline CallbackProc get_callback() const noexcept {
			return m_callproc;
		}

		inline void set_callback( CallbackProc proc ) noexcept {
			m_callproc = proc;
		}


		class GARTER_API _WND_;
	private:
		HWND m_hwnd;
		CallbackProc m_callproc;
		MSG m_msg;
		Event m_event;
	};
}