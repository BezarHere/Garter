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
		Moved = WM_MOVE,
		Resized = WM_SIZE,
		Focused = WM_SETFOCUS,
		Paint = WM_PAINT,
		UnFocused = WM_KILLFOCUS,
		ShowWindow = WM_SHOWWINDOW,
		MouseMoved = WM_SETCURSOR,
		Minimized = WM_COMPACTING,
		UserChanged = WM_USERCHANGED,
		KeyDown = WM_KEYDOWN,
		KeyUp = WM_KEYUP,
		SysKeyDown = WM_SYSKEYDOWN,
		SysKeyUp = WM_SYSKEYUP,
		Exit = (WORD) - 1,
	};

	union Event
	{
		struct IPostionEvent
		{
			WORD x, y;
		} mousepos, windowpos;

		struct ISize
		{
			WORD w, h;
		} windowsize;

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