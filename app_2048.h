#pragma once

#include <string>
#include <Windows.h>

#include "board.h"

class app_2048
{
private:
	bool register_class();

	static std::wstring const s_class_name;

	HINSTANCE m_hinstance;
	HWND m_hwnd_main, m_hwnd_popup;

	board m_board;
	HBRUSH m_field_brush;
	POINT m_screen_size;

	static LRESULT CALLBACK window_proc_static(
		HWND window,
		UINT message,
		WPARAM wparam,
		LPARAM lparam
	);

	LRESULT window_proc(
		HWND window,
		UINT message,
		WPARAM wparam,
		LPARAM lparam
	);

	HWND create_window(DWORD style, HWND parent = nullptr, DWORD ex_style = 0);

	void on_window_move(HWND window, LPWINDOWPOS params);
	void update_transparency();

public:
	app_2048(HINSTANCE instance);

	int run(int show_command);
};