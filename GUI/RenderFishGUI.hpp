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


struct MouseState {
	int mouse_x;
	int mouse_y;

	int hot_item;
	//int active_item;
	bool mouse_down;
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
	static void Label(const WCHAR* text, DWRITE_TEXT_ALIGNMENT text_alignment = DWRITE_TEXT_ALIGNMENT_LEADING);
	static void Label(const char* text, DWRITE_TEXT_ALIGNMENT text_alignment = DWRITE_TEXT_ALIGNMENT_LEADING);
	static void NumberBox(int* val);
	
	static void SideBar(int width = 250);



	template<typename T>
	static void Slider(const char* str, T *pVal, T min, T max);

	//static void Slider(const char* str, float *pVal, float min, float max);

private:
	RenderFishGUI();

	static RECT rc; // Render area
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


	static void draw_rounded_rect(int x, int y, int w, int h, ID2D1SolidColorBrush* brush, ID2D1SolidColorBrush* fill_brush = nullptr);

	inline static bool mouse_in_region(int x, int y, int w, int h) {
		if (mouse_state.mouse_x < x || mouse_state.mouse_y < y ||
			mouse_state.mouse_x >= x + w || mouse_state.mouse_y >= y + h)
			return false;
		return true;
	}
};

template void RenderFishGUI::Slider<float>(const char* str, float *pVal, float min, float max);
template void RenderFishGUI::Slider<int>(const char* str, int *pVal, int min, int max);
template void RenderFishGUI::Slider<double>(const char* str, double *pVal, double min, double max);
