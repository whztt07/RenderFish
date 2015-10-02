#ifndef RTWINDOW_H
#define RTWINDOW_H

#include <windows.h>

class Window
{
private:
	int screen_w, screen_h;
	static int screen_exit;
	int screen_mx = 0, screen_my = 0, screen_mb = 0;
	static int screen_keys[512];
	HWND screen_handle = NULL;
	HDC screen_dc = NULL;
	HBITMAP screen_hb = NULL;
	HBITMAP screen_ob = NULL;
	unsigned char * screen_fb = NULL;
	long screen_pitch = 0;

public:
	int screen_init(int w, int h, const char *title);
	int screen_close(void);
	void screen_dispatch(void);
	void screen_update(void);

	inline void set_pixel(int x, int y, int r, int g, int b) {
		int p = (y * screen_w + x) * 4;
		screen_fb[p++] = b;
		screen_fb[p++] = g;
		screen_fb[p++] = r;
	}

	void run() {
		while (screen_exit == 0 && screen_keys[VK_ESCAPE] == 0)
		{
			screen_dispatch();
			screen_update();
			static float elapse = 1.0f / 30 * 1000;
			Sleep(DWORD(elapse));
		}
	}

private:
	static LRESULT screen_events(HWND, UINT, WPARAM, LPARAM);
};

#endif
