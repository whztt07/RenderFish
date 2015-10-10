#include "RenderFish.hpp"
#include "World.hpp"
#include <windows.h>
#include <D2D1.h>// header for Direct2D
#include <d2d1_1helper.h>
#include <dwrite.h>
#include <chrono>
#include <Wincodec.h>
#include <sstream>
using namespace std;

const int width = 800;
const int height = 600;
World w;

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

#define SAFE_RELEASE(P) if(P){P->Release() ; P = NULL ;}
#define HR(hr) \
	if (FAILED(hr)) { MessageBoxA(g_Hwnd, __FILE__, "Error", MB_OK); exit(1); }

ID2D1Factory* pD2DFactory = NULL; // Direct2D factory
IDWriteFactory* pDWriteFactory = NULL;
ID2D1HwndRenderTarget* pRenderTarget = NULL; // Render target
ID2D1SolidColorBrush* pBlackBrush = NULL; // A black brush, reflect the line color
ID2D1SolidColorBrush* pGrayBrush = NULL;
ID2D1SolidColorBrush* pWhiteBrush = NULL;
ID2D1SolidColorBrush* pRedBrush = NULL;
IDWriteTextFormat * pTexFormat = NULL;
IDWriteTextLayout * pTextLayout = NULL;
ID2D1Bitmap *pBitmap;

RECT rc; // Render area
HWND g_Hwnd; // Window handle

struct MouseState {
	int mouse_x;
	int mouse_y;

	int hot_item;
	//int active_item;
	bool mouse_down;
};
static MouseState mouse_state = { -1, -1, -1, false };

struct SideBar {
	D2D1_RECT_F rect;
	//int left;
	//int right;
	//int width = 200;
	//int height;
	int x_margin = 10;
	int y_margin = 5;
	int y_cell_height = 30;
	int y_filled = 0;
};
static SideBar g_side_bar;

struct GUIButton {
	int next_id = 0;
	int button_id_clicked = -1;
	void reset() {
		next_id = 0;
	}
};
static GUIButton g_button;


static std::chrono::high_resolution_clock::time_point last_render_time;

VOID CreateD2DResource(HWND hWnd)
{
	if (pRenderTarget) {
		return;
	}

	HRESULT hr;
	HR(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pD2DFactory));

	hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&pDWriteFactory));
	HR(hr);

	// Obtain the size of the drawing area
	GetClientRect(hWnd, &rc);

	// Create a Direct2D render target
	hr = pD2DFactory->CreateHwndRenderTarget(
		D2D1::RenderTargetProperties(),
		D2D1::HwndRenderTargetProperties(hWnd, D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top)),
		&pRenderTarget);
	HR(hr);

	// Create a brush
	HR(pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &pBlackBrush));
	HR(pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &pRedBrush));
	HR(pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Gray), &pGrayBrush));
	HR(pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &pWhiteBrush));

	HR(pDWriteFactory->CreateTextFormat(L"Consolas", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL, 14.f, L"", &pTexFormat));
	IDWriteInlineObject * trimming_sign = nullptr;
	pDWriteFactory->CreateEllipsisTrimmingSign(pTexFormat, &trimming_sign);
	DWRITE_TRIMMING trim;
	trim.granularity = DWRITE_TRIMMING_GRANULARITY_CHARACTER;
	trim.delimiter = 0;
	trim.delimiterCount = 0;
	pTexFormat->SetTrimming(&trim, trimming_sign);
	pTexFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

	HR(pDWriteFactory->CreateTextLayout(L"LooooooooooongWorld", 20, pTexFormat, 0, 0, &pTextLayout));
}

VOID draw_rounded_rect(int x, int y, int w, int h, ID2D1SolidColorBrush* brush, ID2D1SolidColorBrush* fill_brush = nullptr)
{
	// Draw Rectangle
	//pRenderTarget->DrawRectangle(D2D1::RectF(float(x), float(y), float(x + w), float(y + h)), brush);
	if (fill_brush != nullptr)
		pRenderTarget->FillRoundedRectangle(
			D2D1::RoundedRect(D2D1::RectF(float(x), float(y), float(x + w), float(y + h)), 5.f, 5.f), fill_brush);
	pRenderTarget->DrawRoundedRectangle(
		D2D1::RoundedRect(D2D1::RectF(float(x), float(y), float(x + w), float(y + h)), 5.f, 5.f), brush);
}

VOID Cleanup()
{
	SAFE_RELEASE(pRenderTarget);
	SAFE_RELEASE(pBlackBrush);
	SAFE_RELEASE(pD2DFactory);
	SAFE_RELEASE(pRedBrush);
	SAFE_RELEASE(pGrayBrush);
}

inline bool mouse_in_region(int x, int y, int w, int h) {
	if (mouse_state.mouse_x < x || mouse_state.mouse_y < y ||
		mouse_state.mouse_x >= x + w || mouse_state.mouse_y >= y + h)
		return false;
	return true;
}

bool button(const WCHAR* label = nullptr)
{
	bool clicked = false;

	int id = g_button.next_id;
	g_button.next_id++;
	int width = 128, height = g_side_bar.y_cell_height;
	int x = (int)g_side_bar.rect.left + g_side_bar.x_margin, y = g_side_bar.y_filled + g_side_bar.y_margin;
	width = int(g_side_bar.rect.right - g_side_bar.rect.left) - g_side_bar.x_margin * 2;
	g_side_bar.y_filled += g_side_bar.y_margin * 2 + height;

	if (mouse_in_region(x, y, width, height)) {
		mouse_state.hot_item = id;
		if (mouse_state.mouse_down) {
			//ui_state.active_item = id;
			draw_rounded_rect(x + 1, y + 1, width - 2, height - 2, pBlackBrush, pGrayBrush);
			g_button.button_id_clicked = id;
		}
		else {
			draw_rounded_rect(x, y, width, height, pBlackBrush, pWhiteBrush);
			if (g_button.button_id_clicked == id) {
				clicked = true;
				g_button.button_id_clicked = -1;
			}
		}
		//if (ui_state.active_item == -1 && ui_state.mouse_down)
		//	ui_state.active_item = id;
	}
	else {
		draw_rounded_rect(x, y, width, height, pBlackBrush);
	}
	if (label != nullptr) {
		float margin = 10;
		//pRenderTarget->DrawTextA(label, wcslen(label), pTexFormat, D2D1::RectF(x + margin, y + (height - 17) / 2.f, width, height), pBlackBrush);

		pTexFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
		pTexFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
		SAFE_RELEASE(pTextLayout);
		HR(pDWriteFactory->CreateTextLayout(label, wcslen(label), pTexFormat, (float)width - margin*2.f, (float)height, &pTextLayout));
		DWRITE_TEXT_METRICS mertics;
		pTextLayout->GetMetrics(&mertics);
		pRenderTarget->DrawTextLayout(D2D1::Point2F(x + margin, y + (height - height) / 2.f), pTextLayout, pBlackBrush);
	}

	//if (ui_state.mouse_down == 0 && ui_state.hot_item == id && ui_state.active_item == id)
	//	clicked = true;
	return clicked;
}

bool button(const char* label) {
	return button(ToWString<const char*>(label).c_str());
}

bool side_bar(int width = 200) {
	GetClientRect(g_Hwnd, &rc);
	int window_width = rc.right - rc.left;
	g_side_bar.rect = D2D1::RectF((float)window_width - width, 0.f, (float)window_width, float(rc.bottom - rc.top));
	g_side_bar.y_filled = g_side_bar.y_margin;
	pRenderTarget->FillRectangle(D2D1::RectF(float(window_width - width), 0.f, (float)window_width, float(rc.bottom - rc.top)), pGrayBrush);
	return true;
}

void label(const WCHAR* text, DWRITE_TEXT_ALIGNMENT text_alignment = DWRITE_TEXT_ALIGNMENT_LEADING) {
	float width = 128.f, height = (float)g_side_bar.y_cell_height;
	float x = g_side_bar.rect.left + g_side_bar.x_margin, y = float(g_side_bar.y_filled + g_side_bar.y_margin);
	width = (g_side_bar.rect.right - g_side_bar.rect.left) - g_side_bar.x_margin * 2;
	pTexFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	pTexFormat->SetTextAlignment(text_alignment);
	SAFE_RELEASE(pTextLayout);
	HR(pDWriteFactory->CreateTextLayout(text, wcslen(text), pTexFormat, width, height, &pTextLayout));
	//DWRITE_TEXT_METRICS mertics;
	//pTextLayout->GetMetrics(&mertics);
	//if (height < mertics.height) height = mertics.height;
	//pTextLayout->SetMaxHeight(height);
	pRenderTarget->DrawTextLayout(D2D1::Point2F(x, y), pTextLayout, pBlackBrush);

	g_side_bar.y_filled += int(g_side_bar.y_margin * 2 + height);
}

void draw_texture() {
	pRenderTarget->DrawBitmap(pBitmap, D2D1::RectF(0, 0, 800, 600));
}

VOID Render() {

	pRenderTarget->BeginDraw();
	// Clear background color white
	pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

	g_button.reset();
	//ui_state.active_item = -1;
	mouse_state.hot_item = -1;

	draw_texture();

	side_bar();

	if (button(L"Render")) {
		w.render_scene();
		pBitmap->CopyFromMemory(nullptr, &w.color_buffer[0], 4 * width);
	}
	last_render_time = std::chrono::high_resolution_clock::now();

	HR(pRenderTarget->EndDraw());
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_PAINT:
		Render();
		ValidateRect(g_Hwnd, NULL);
		return 0;

	case WM_SIZE:
		//info("resize");
		break;

	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case VK_ESCAPE:
			SendMessage(hwnd, WM_CLOSE, 0, 0);
			break;

		default:
			break;
		}
	}
	break;

	case WM_MOUSEMOVE:
		mouse_state.mouse_x = LOWORD(lParam);
		mouse_state.mouse_y = HIWORD(lParam);
		break;
	case WM_LBUTTONDOWN:
		mouse_state.mouse_down = true;
		break;
	case WM_LBUTTONUP:
		mouse_state.mouse_down = false;
		break;
	case WM_DESTROY:
		//Cleanup();
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
	WNDCLASSEX winClass;

	winClass.lpszClassName = "Direct2D";
	winClass.cbSize = sizeof(WNDCLASSEX);
	winClass.style = CS_HREDRAW | CS_VREDRAW;
	winClass.lpfnWndProc = WndProc;
	winClass.hInstance = hInstance;
	winClass.hIcon = NULL;
	winClass.hIconSm = NULL;
	winClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	winClass.hbrBackground = NULL;
	winClass.lpszMenuName = NULL;
	winClass.cbClsExtra = 0;
	winClass.cbWndExtra = 0;

	if (!RegisterClassEx(&winClass))
	{
		MessageBox(NULL, TEXT("This program requires Windows NT!"), "error", MB_ICONERROR);
		return 0;
	}

	int window_width = width + 200 + 16;
	int window_height = height + 38;

	g_Hwnd = CreateWindowEx(NULL,
		"Direct2D", // window class name
		"Draw Rectangle", // window caption
		WS_OVERLAPPEDWINDOW, // window style
		CW_USEDEFAULT, // initial x position
		CW_USEDEFAULT, // initial y position
		window_width, // initial x size
		window_height, // initial y size
		NULL, // parent window handle
		NULL, // window menu handle
		hInstance, // program instance handle
		NULL); // creation parameter

	CreateD2DResource(g_Hwnd);
	HRESULT hr;
	//load_bitmap_from_file(&pBitmap, pRenderTarget, 800, 600);
	//pRenderTarget->CreateBitmap(800 * 600, { { DXGI_FORMAT_R8G8B8A8_UNORM } });
	hr = pRenderTarget->CreateBitmap(       // <==================== Failed to create bitmap
		D2D1::SizeU(width, height),
		D2D1::BitmapProperties(
			D2D1::PixelFormat(
				DXGI_FORMAT_B8G8R8A8_UNORM,
				D2D1_ALPHA_MODE_IGNORE)
			),
		&pBitmap
		);
	HR(hr);

	log_system_init();
	
	//World w;
	w.build(width, height);
	
	w.render_scene();
	pBitmap->CopyFromMemory(nullptr, &w.color_buffer[0], 4 * width);

	ShowWindow(g_Hwnd, iCmdShow);
	UpdateWindow(g_Hwnd);

	MSG msg;
	ZeroMemory(&msg, sizeof(msg));

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - last_render_time).count();
		if (ms > 16.6f) {
			Render();
		}
	}

	Cleanup();
	return msg.wParam;
}