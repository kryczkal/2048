#include "app_2048.h"
#include <stdexcept>
#include <dwmapi.h>

std::wstring const app_2048::s_class_name{ L"2048 Window" };

bool app_2048::register_class()
{
	WNDCLASSEX desc{};
	if (GetClassInfoExW(m_hinstance, s_class_name.c_str(), &desc) != 0) {
		return true;
	};

	desc = {
		.cbSize = sizeof(WNDCLASSEXW),
		.lpfnWndProc = window_proc_static,
		.hInstance = m_hinstance,
		.hCursor = LoadCursorW(nullptr, L"IDC_ARROW"),
		.hbrBackground = CreateSolidBrush(RGB(250,247,238)),
		.lpszClassName = s_class_name.c_str(),
	};
	return RegisterClassExW(&desc);
}

LRESULT app_2048::window_proc_static(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	app_2048* app = nullptr;
	if (message == WM_NCCREATE)
	{
		app = static_cast<app_2048*>(reinterpret_cast<LPCREATESTRUCTW>(lparam)->lpCreateParams); // gets the "this" set in create_window
		SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));
	}
	else {
		app = reinterpret_cast<app_2048*>(GetWindowLongPtrW(window, GWLP_USERDATA));
	}
	LRESULT res = app ?
		app->window_proc(window, message, wparam, lparam) :
		DefWindowProcW(window, message, wparam, lparam);
	if (message == WM_NCDESTROY) {
		SetWindowLongPtrW(window, GWLP_USERDATA, 0);
	}
	return res;
}

LRESULT app_2048::window_proc(
	HWND window,
	UINT message,
	WPARAM wparam,
	LPARAM lparam)
{
	switch (message) {
	case WM_WINDOWPOSCHANGED:
		on_window_move(window, reinterpret_cast<LPWINDOWPOS>(lparam));
		return 0;
	case WM_CTLCOLORSTATIC:
		return reinterpret_cast<INT_PTR>(m_field_brush);
		// Death
	case WM_CLOSE:
		DestroyWindow(m_hwnd_main);
		return 0;
	case WM_DESTROY:
		if (window == m_hwnd_main) {
			PostQuitMessage(EXIT_SUCCESS);
		}
		return 0;
	}
	return DefWindowProcW(window, message, wparam, lparam);
}

HWND app_2048::create_window(DWORD style, HWND parent, DWORD ex_style)
{
	RECT size{ 0, 0, board::width, board::height };
	AdjustWindowRectEx(&size, style, false, 0); // Make the size i give the real size, so take into account the bars, borders, etc..
	HWND window = CreateWindowExW(
		ex_style,
		s_class_name.c_str(),
		L"2048",
		style,
		CW_USEDEFAULT,
		0,
		size.right - size.left,
		size.bottom - size.top,
		parent,
		nullptr,
		m_hinstance,
		this
	);

	for (auto& f : m_board.fields()) {
		CreateWindowExW(
			0,
			L"STATIC",
			nullptr,
			WS_CHILD | WS_VISIBLE | SS_CENTER,
			f.position.left,
			f.position.top,
			f.position.right - f.position.left,
			f.position.bottom - f.position.top,
			window,
			nullptr,
			m_hinstance,
			nullptr
		);
	}

	return window;
}

void app_2048::on_window_move(HWND window, LPWINDOWPOS params)
{
	HWND other = (window == m_hwnd_main) ? m_hwnd_popup : m_hwnd_main;

	RECT other_rect;
	GetWindowRect(other, &other_rect);
	SIZE other_size{
		.cx = other_rect.right - other_rect.left,
		.cy = other_rect.bottom - other_rect.top
	};
	RECT my_rect;
	GetWindowRect(window, &my_rect);

	POINT my_window_center{
		.x = my_rect.left + (my_rect.right - my_rect.left) / 2,
		.y = my_rect.top + (my_rect.bottom - my_rect.top) / 2
	};

	POINT screen_center{
		.x = m_screen_size.x / 2,
		.y = m_screen_size.y / 2
	};

	POINT new_pos{
		.x = 2 * screen_center.x - my_window_center.x,
		.y = 2 * screen_center.y - my_window_center.y,
	};

	
	if (new_pos.x == other_rect.left && new_pos.y == other_rect.top) return;
	SetWindowPos(other, nullptr, new_pos.x, new_pos.y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
	update_transparency();
}

void app_2048::update_transparency()
{
	RECT main_rect, popup_rect, intersection;
	DwmGetWindowAttribute(m_hwnd_main, DWMWA_EXTENDED_FRAME_BOUNDS, &main_rect, sizeof(RECT));
	DwmGetWindowAttribute(m_hwnd_popup, DWMWA_EXTENDED_FRAME_BOUNDS, &popup_rect, sizeof(RECT));
	IntersectRect(&intersection, &main_rect, &popup_rect);
	BYTE alpha = IsRectEmpty(&intersection) ? 255 : 255 * 50 / 100;
	SetLayeredWindowAttributes(m_hwnd_popup, 0, alpha, LWA_ALPHA);
}



app_2048::app_2048(HINSTANCE instance) : 
		m_hinstance{ instance }, m_hwnd_main{},
		m_field_brush{CreateSolidBrush(RGB(204,192,174))},
		m_screen_size{ GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN) }
{
	register_class();
	DWORD main_style = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_BORDER | WS_MINIMIZEBOX;
	DWORD popup_style = WS_OVERLAPPED | WS_CAPTION;

	m_hwnd_main = create_window(main_style);
	m_hwnd_popup = create_window(popup_style, m_hwnd_main, WS_EX_LAYERED);
	SetLayeredWindowAttributes(m_hwnd_popup, 0, 255, LWA_ALPHA);
}

int app_2048::run(int show_command)
{
	ShowWindow(m_hwnd_main, show_command);
	ShowWindow(m_hwnd_popup, SW_SHOWNA);
	MSG msg{};
	BOOL result = TRUE;
	while ((result = GetMessageW(&msg, nullptr, 0, 0)) != 0) {
		if (result == -1)
			return EXIT_FAILURE;
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
	return EXIT_SUCCESS;
}
