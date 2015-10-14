#include <windows.h>
#include <D2D1.h>
#include <d2d1_1helper.h>
#include <chrono>
#include "RenderFishGUI.hpp"

using namespace std;

ID2D1Factory* pD2DFactory = NULL; // Direct2D factory
ID2D1HwndRenderTarget* pRenderTarget = NULL; // Render target
ID2D1Bitmap *pBitmap;

RECT rc; // Render area
HWND g_Hwnd; // Window handle

static std::chrono::high_resolution_clock::time_point last_render_time;

VOID CreateD2DResource(HWND hWnd)
{
	if (pRenderTarget) {
		return;
	}

	HRESULT hr;
	HR(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pD2DFactory));

	// Obtain the size of the drawing area
	GetClientRect(hWnd, &rc);

	// Create a Direct2D render target
	hr = pD2DFactory->CreateHwndRenderTarget(
		D2D1::RenderTargetProperties(),
		D2D1::HwndRenderTargetProperties(hWnd, D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top)),
		&pRenderTarget);
	HR(hr);

	RenderFishGUI::CreateD2DResource(pD2DFactory, hWnd, pRenderTarget);
}


VOID Cleanup()
{
	RenderFishGUI::Cleanup();
	SAFE_RELEASE(pRenderTarget);
	SAFE_RELEASE(pD2DFactory);
}

void draw_texture() {
	pRenderTarget->DrawBitmap(pBitmap, D2D1::RectF(0, 0, 800, 600));
}

VOID Render() {

	pRenderTarget->BeginDraw();
	// Clear background color white
	pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

	RenderFishGUI::BeginFrame();

	draw_texture();

	static string buf("");
	RenderFishGUI::Label(buf.c_str());
	if (RenderFishGUI::Button(L"Button 1"))
		buf = "button 1 clicked";
	if (RenderFishGUI::Button(L"Button 2"))
		buf = "button 2 clicked";

	static float counter1 = 0;
	static int counter2 = 0;

	RenderFishGUI::Slider("float slider", &counter1, 0.f, 20.f);

	RenderFishGUI::Label(L"test label center", DWRITE_TEXT_ALIGNMENT_CENTER);
	RenderFishGUI::Label(L"test label right", DWRITE_TEXT_ALIGNMENT_TRAILING);

	if (RenderFishGUI::Button(L"warning")) {
		MessageBox(g_Hwnd, "warning", "warning", MB_OK);
	}

	RenderFishGUI::Button(L"This is a a a a a loooooooooooooooooooooooooooooooong word.");
	if (RenderFishGUI::Button(L"Render")) {
		// render
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
		RenderFishGUI::mouse_state.mouse_x = LOWORD(lParam);
		RenderFishGUI::mouse_state.mouse_y = HIWORD(lParam);
		break;
	case WM_LBUTTONDOWN:
		RenderFishGUI::mouse_state.mouse_down = true;
		break;
	case WM_LBUTTONUP:
		RenderFishGUI::mouse_state.mouse_down = false;
		break;
	case WM_DESTROY:
		//Cleanup();
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}

HRESULT load_bitmap_from_file(ID2D1Bitmap **ppBitmap, ID2D1RenderTarget *pRenderTarget, UINT destinationWidth = 800, UINT destinationHeight = 600) {
	HRESULT hr = S_OK;

	IWICImagingFactory *pIWICFactory;
	// http://stackoverflow.com/questions/29125216/cant-initialize-iwicimagingfactory-in-a-direct2d-project
	CoInitializeEx(NULL, COINIT_MULTITHREADED);
	hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pIWICFactory));
	HR(hr);
	
	IWICBitmapDecoder * p_decoder = nullptr;
	pIWICFactory->CreateDecoder(GUID_ContainerFormatBmp, NULL, &p_decoder);
	IWICBitmapFrameDecode *pSource = NULL;
	IWICStream *pStream = NULL;
	IWICFormatConverter *pConverter = NULL;
	IWICBitmapScaler *pScaler = NULL;
	hr = pIWICFactory->CreateDecoderFromFilename(
		L"test.bmp",
		NULL,
		GENERIC_READ,
		WICDecodeMetadataCacheOnLoad,
		&p_decoder );
	HR(hr);
	hr = p_decoder->GetFrame(0, &pSource);
	HR(hr);
	hr = pIWICFactory->CreateFormatConverter(&pConverter);

	// If a new width or height was specified, create an
	// IWICBitmapScaler and use it to resize the image.
	if (destinationWidth != 0 || destinationHeight != 0)
	{
		UINT originalWidth, originalHeight;
		hr = pSource->GetSize(&originalWidth, &originalHeight);
		if (SUCCEEDED(hr))
		{
			if (destinationWidth == 0)
			{
				FLOAT scalar = static_cast<FLOAT>(destinationHeight) / static_cast<FLOAT>(originalHeight);
				destinationWidth = static_cast<UINT>(scalar * static_cast<FLOAT>(originalWidth));
			}
			else if (destinationHeight == 0)
			{
				FLOAT scalar = static_cast<FLOAT>(destinationWidth) / static_cast<FLOAT>(originalWidth);
				destinationHeight = static_cast<UINT>(scalar * static_cast<FLOAT>(originalHeight));
			}

			hr = pIWICFactory->CreateBitmapScaler(&pScaler);
			if (SUCCEEDED(hr))
			{
				hr = pScaler->Initialize(
					pSource,
					destinationWidth,
					destinationHeight,
					WICBitmapInterpolationModeCubic
					);
			}
			if (SUCCEEDED(hr))
			{
				hr = pConverter->Initialize(
					pScaler,
					GUID_WICPixelFormat32bppPBGRA,
					WICBitmapDitherTypeNone,
					NULL,
					0.f,
					WICBitmapPaletteTypeMedianCut
					);
			}
		}
	}

	HR(hr);
	
	pRenderTarget->CreateBitmapFromWicBitmap(pConverter, nullptr, ppBitmap);

	SAFE_RELEASE(p_decoder);
	SAFE_RELEASE(pSource);
	SAFE_RELEASE(pStream);
	SAFE_RELEASE(pConverter);
	SAFE_RELEASE(pScaler);
	return hr;
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

	int window_width = 800 + 200 + 16;
	int window_height = 600 + 38;

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

	load_bitmap_from_file(&pBitmap, pRenderTarget, 800, 600);

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