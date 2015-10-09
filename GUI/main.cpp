#include <windows.h>
#include <D2D1.h>// header for Direct2D
#include <d2d1_1helper.h>
#include <chrono>

#define SAFE_RELEASE(P) if(P){P->Release() ; P = NULL ;}
#define HR(hr) \
	if (FAILED(hr)) { MessageBox(g_Hwnd, __FILE__, "Error", MB_OK); exit(1); }

ID2D1Factory* pD2DFactory = NULL; // Direct2D factory
IDWriteFactory* pDWriteFactory = NULL;
ID2D1HwndRenderTarget* pRenderTarget = NULL; // Render target
ID2D1SolidColorBrush* pBlackBrush = NULL; // A black brush, reflect the line color
ID2D1SolidColorBrush* pGrayBrush = NULL;
ID2D1SolidColorBrush* pRedBrush = NULL;
IDWriteTextFormat * pTexFormat = NULL;

RECT rc; // Render area
HWND g_Hwnd; // Window handle

struct UIState {
	int mouse_x;
	int mouse_y;

	int hot_item;
	int active_item;
	bool mouse_down;
};

static UIState ui_state = { 0, 0, 0, 0, 0 };

static std::chrono::high_resolution_clock::time_point last_render_time;

VOID CreateD2DResource(HWND hWnd)
{
	if (pRenderTarget) {
		return;
	}
	
	HRESULT hr;
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pD2DFactory);
	if (FAILED(hr))
	{
		MessageBox(hWnd, "Create D2D factory failed!", "Error", MB_OK);
		return;
	}

	HR(-1);

	// Obtain the size of the drawing area
	GetClientRect(hWnd, &rc);

	// Create a Direct2D render target
	hr = pD2DFactory->CreateHwndRenderTarget(
		D2D1::RenderTargetProperties(),
		D2D1::HwndRenderTargetProperties(
			hWnd,
			D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top)
			),
		&pRenderTarget
		);
	if (FAILED(hr))
	{
		MessageBox(hWnd, "Create render target failed!", "Error", 0);
		return;
	}

	// Create a brush
	hr = pRenderTarget->CreateSolidColorBrush(
		D2D1::ColorF(D2D1::ColorF::Black),
		&pBlackBrush
		);
	if (FAILED(hr))
	{
		MessageBox(hWnd, "Create brush failed!", "Error", 0);
		return;
	}

	hr = pRenderTarget->CreateSolidColorBrush(
		D2D1::ColorF(D2D1::ColorF::Red),
		&pRedBrush
		);
	if (FAILED(hr))
	{
		MessageBox(hWnd, "Create brush failed!", "Error", 0);
		return;
	}

	hr = pRenderTarget->CreateSolidColorBrush(
		D2D1::ColorF(D2D1::ColorF::Gray),
		&pGrayBrush
		);
	if (FAILED(hr))
	{
		MessageBox(hWnd, "Create brush failed!", "Error", 0);
		return;
	}
}

VOID draw_rounded_rect(int x, int y, int w, int h, ID2D1SolidColorBrush* brush, ID2D1SolidColorBrush* fill_brush = nullptr)
{
	pRenderTarget->BeginDraw();

	// Clear background color white
	pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

	// Draw Rectangle
	//pRenderTarget->DrawRectangle(D2D1::RectF(float(x), float(y), float(x + w), float(y + h)), brush);
	if (fill_brush != nullptr)
		pRenderTarget->FillRoundedRectangle(
			D2D1::RoundedRect(D2D1::RectF(float(x), float(y), float(x + w), float(y + h)), 5.f, 5.f), fill_brush);
	pRenderTarget->DrawRoundedRectangle(
		D2D1::RoundedRect(D2D1::RectF(float(x), float(y), float(x + w), float(y + h)), 5.f, 5.f), brush);


	HRESULT hr = pRenderTarget->EndDraw();

	if (FAILED(hr))
	{
		MessageBox(NULL, "Draw failed!", "Error", 0);
		return;
	}
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
	if (ui_state.mouse_x < x || ui_state.mouse_y < y ||
		ui_state.mouse_x >= x+w || ui_state.mouse_y >= y + h)
		return false;
	return true;
}

int button(int id, int x, int y, const char* label = "")
{
	if (mouse_in_region(x, y, 64, 48)) {
		ui_state.hot_item = id;
		if (ui_state.mouse_down) {
			ui_state.active_item = id;
			draw_rounded_rect(x, y, 64, 48, pBlackBrush, pBlackBrush);
		}
		else {
			draw_rounded_rect(x, y, 64, 48, pBlackBrush, pGrayBrush);
		}
		if (ui_state.active_item == 0 && ui_state.mouse_down)
			ui_state.active_item = id;
	}
	else {
		draw_rounded_rect(x, y, 64, 48, pBlackBrush);
	}
	//pRenderTarget->BeginDraw();
	//pRenderTarget->DrawTextA("hello", 6, );
	//pRenderTarget->EndDraw();


	if (ui_state.mouse_down == 0 && ui_state.hot_item == id && ui_state.active_item == id)
		return 1;
	return 0;
}

VOID Render() {
	if (button(1, 10, 10)) {
	}
	last_render_time = std::chrono::high_resolution_clock::now();
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_PAINT:
		Render();
		ValidateRect(g_Hwnd, NULL);
		return 0;

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
		ui_state.mouse_x = LOWORD(lParam);
		ui_state.mouse_y = HIWORD(lParam);
		break;
	case WM_LBUTTONDOWN:
		ui_state.mouse_down = true;
		//Render();
		break;
	case WM_LBUTTONUP:
		ui_state.mouse_down = false;
		//Render();
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

	g_Hwnd = CreateWindowEx(NULL,
		"Direct2D", // window class name
		"Draw Rectangle", // window caption
		WS_OVERLAPPEDWINDOW, // window style
		CW_USEDEFAULT, // initial x position
		CW_USEDEFAULT, // initial y position
		600, // initial x size
		600, // initial y size
		NULL, // parent window handle
		NULL, // window menu handle
		hInstance, // program instance handle
		NULL); // creation parameters

	ShowWindow(g_Hwnd, iCmdShow);
	UpdateWindow(g_Hwnd);

	CreateD2DResource(g_Hwnd);

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