// Zaher abdulatif abdurab babker (C) 2023-2024
#pragma once

#include <string>


#ifndef _WINDOWS
#error GARTER is a windows specific library
#endif

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#ifdef GARTER_STATIC
	#define GARTER_API extern
#else
	#ifdef GARTER_EXPORTS
		#define GARTER_API __declspec(dllexport)
	#else
		#define GARTER_API __declspec(dllimport)
	#endif
#endif



namespace gart
{
	enum class EventType
	{
		Exit = -1,
	};

	struct Event
	{
		EventType type;
		union
		{

		};
	};
	
	class Window;
	typedef void(*EventCallback)(Window &, const Event &);

	class Window
	{
	public:
		Window( const std::wstring &title );

	private:
		HWND m_hwnd;
	};
}