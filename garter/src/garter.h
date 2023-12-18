// Zaher abdulatif abdurab babker (C) 2023-2024
#pragma once

#include <string.h>


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



namespace gt
{
	class Window
	{
	public:
		Window( const char * );

	private:
		HWND m_hwnd;
	};
}