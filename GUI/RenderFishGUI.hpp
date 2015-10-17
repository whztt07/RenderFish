#pragma once
#include <D2D1.h>
#include <d2d1_1helper.h>
#include <memory>
#include <dwrite.h>
#include <sstream>
#include <string>
#include <Wincodec.h>

using namespace std;

#define SAFE_RELEASE(P) if(P){P->Release() ; P = NULL ;}
#define HR(hr) \
	if (FAILED(hr)) { MessageBoxA(NULL, __FILE__, "Error", MB_OK); exit(1); }

template<typename T>
inline std::wstring ToWString(const T& s)
{
	std::wostringstream oss;
	oss << s;
	return oss.str();
}

template<typename T>
inline T ToString(const std::wstring& s)
{
	T x;
	std::wistringstream iss(s);
	iss >> x;
	return x;
}

template<typename T>
class com_ptr {
private:
	T* ptr;
public:
	com_ptr(T* ptr) : ptr(ptr) {
	}

	~com_ptr() {
		SAFE_RELEASE(ptr);
	}
};

enum GUIAlignment {
	align_horiontally_left = 0,
	align_horiontally_center = 1,
	align_horiontally_right = 2,
	align_vertically_top = 4,
	align_vertically_center = 8,
	align_vertically_bottom = 16,
	align_center = align_horiontally_center | align_vertically_center
};

struct MouseState {
	int pos_x = -1;
	int pos_y = -1;

	int hot_item = -1;
	//int active_item;
	bool mouse_down = false;

	bool mouse_wheel_rotating = false;
	short mouse_wheel_z_delta = 0;
};

class RenderFishGUI
{
public:

	static MouseState mouse_state;

	~RenderFishGUI() {};

	static HRESULT CreateD2DResource(ID2D1Factory* pD2DFactory, HWND hWnd, ID2D1HwndRenderTarget* pRenderTarget);
	static void Cleanup();

	static void BeginFrame();
	static void EndFrame();
	static void BeginDialog();
	static bool Button(const WCHAR* label = nullptr);
	static bool Button(const char* label);
	static void Label(const WCHAR* text, GUIAlignment text_alignment = align_horiontally_center);
	static void Label(const char* text, GUIAlignment text_alignment = align_horiontally_center);
	static void NumberBox(int* val);
	static void SideBar(int width = 250);

	static void WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	template<typename T>
	static void Slider(const char* str, T *pVal, T min, T max);

	//static void Slider(const char* str, float *pVal, float min, float max);

private:
	RenderFishGUI();

	//static RECT rc; // Render area
	static ID2D1Factory*			pD2DFactory;
	static ID2D1HwndRenderTarget*	pRenderTarget; // Render target
	static IDWriteFactory*			pDWriteFactory;
	static ID2D1SolidColorBrush*	pBlackBrush; // A black brush, reflect the line color
	static ID2D1SolidColorBrush*	pGrayBrush;
	static ID2D1SolidColorBrush*	pWhiteBrush;
	static ID2D1SolidColorBrush*	pRedBrush;
	static ID2D1SolidColorBrush*	pReuseableBrush;
	static IDWriteTextFormat *		pTexFormat;
	static IDWriteTextLayout *		pTextLayout;

	static void draw_rect(int x, int y, int w, int h);
	static void draw_rect(int x, int y, int w, int h, ID2D1SolidColorBrush* border_brush);
	static void fill_rect(int x, int y, int w, int h, ID2D1SolidColorBrush* fill_brush);
	static void draw_rounded_rect(int x, int y, int w, int h, ID2D1SolidColorBrush* border_brush, ID2D1SolidColorBrush* fill_brush = nullptr);

	inline static bool mouse_in_region(int x, int y, int w, int h) {
		return (x < mouse_state.pos_x && mouse_state.pos_x < x + w) &&
			(y < mouse_state.pos_y && mouse_state.pos_y < y + h);
	}
};

template void RenderFishGUI::Slider<float>(const char* str, float *pVal, float min, float max);
template void RenderFishGUI::Slider<int>(const char* str, int *pVal, int min, int max);
template void RenderFishGUI::Slider<double>(const char* str, double *pVal, double min, double max);
