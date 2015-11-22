#include "RenderFish.hpp"
#include "World.hpp"
#include <windows.h>
#include <D2D1.h>
#include <d2d1_1helper.h>
#include <chrono>
#include "Scene.hpp"
#include "Light.hpp"
#include "RenderFishGUI.hpp"
#include "Renderer.hpp"
#include "Film.hpp"
#include "Camera.hpp"
#include "Integrator.hpp"
#include "WhittedIntegrator.hpp"
#include "SimpleSampler.hpp"
#include "RandomSampler.hpp"
#include "StratifiedSampler.hpp"

using namespace std;

const int width = 800;
const int height = 600;
World w;
Renderer* p_renderer;
Scene* p_scene;
ImageFilm* p_film;
ID2D1Factory* pD2DFactory = NULL; // Direct2D factory
ID2D1HwndRenderTarget* pRenderTarget = NULL; // Render target
ID2D1Bitmap *pBitmap;

RECT rc; // Render area
HWND g_Hwnd; // Window handle

float image_scale = 1.0f;

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
	static float local_image_scale = 1.0f;
	static vector<unsigned char> pixels;
	if (local_image_scale != image_scale) {
		pixels = p_film->color_buffer;
		for (int i = 0; i < pixels.size(); ++i)
			pixels[i] *= image_scale;
		pBitmap->CopyFromMemory(nullptr, &pixels[0], 4 * width);
		local_image_scale = image_scale;
	}
	pRenderTarget->DrawBitmap(pBitmap, D2D1::RectF(0, 0, 800, 600));
}

VOID Render() {

	pRenderTarget->BeginDraw();
	// Clear background color white
	pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

	draw_texture();
	RenderFishGUI::BeginFrame();

	static string buf("");
	RenderFishGUI::Label(buf.c_str());
	if (RenderFishGUI::Button(L"Button 1"))
		buf = "button 1 clicked";
	if (RenderFishGUI::Button(L"Button 2"))
		buf = "button 2 clicked";

	static float counter1 = 10.f;
	static int counter2 = 3;

	RenderFishGUI::Slider<float>("float slider", &counter1, 0.f, 20.f);
	RenderFishGUI::Slider<int>("int slider", &counter2, 0, 10);

	//static int number1 = 10;
	//RenderFishGUI::draw_number_box(&number1);

	RenderFishGUI::Label(L"test label center", align_horiontally_center);
	RenderFishGUI::Label(L"test label right", align_horiontally_right);

	if (RenderFishGUI::Button(L"warning")) {
		MessageBox(0, "warning", "warning", MB_OK);
	}

	RenderFishGUI::Button(L"This is a a a a a loooooooooooooooooooooooooooooooong word.");

	RenderFishGUI::Button(L"Region");
	RenderFishGUI::Button(L"Cast a ray");

	RenderFishGUI::EndFrame();

	if (RenderFishGUI::Button(L"Render")) {
		//w.render_scene();
		//pBitmap->CopyFromMemory(nullptr, &w.color_buffer[0], 4 * width);
		p_renderer->render(p_scene);
		pBitmap->CopyFromMemory(nullptr, &p_film->color_buffer[0], 4 * width);
	}
	
	RenderFishGUI::Slider<float>("image scale", &image_scale, 0.f, 10.f);
	
	last_render_time = std::chrono::high_resolution_clock::now();

	HR(pRenderTarget->EndDraw());
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	RenderFishGUI::WndProc(hwnd, message, wParam, lParam);
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
	
	World w;
	w.build(width, height);

	KdTree * kdtree = w.kdTree;
	
	Transform light_trans = translate(0, 4, 0);
	vector<Light*> lights;
	lights.push_back(new PointLight(light_trans, Spectrum(1.0f)));

	Scene scene(kdtree, lights);
	p_scene = &scene;

	Transform camera_trans = inverse(look_at(Point(0, 2.5, 2.5), Point(0, 0, -1)));
	//Transform camera_trans = inverse(look_at(Point(0, 1, 1), Point(0, 0, 0)));
	//Transform camera_trans = inverse(translate(0, 3.5, 5) * rotate(-30, Vec3::axis_x));
	float fov = 60.f;
	//auto proj = perspective(fov, 1e-2f, 1000.f);
	float crop[] = { 0, 1, 0, 1 };
	ImageFilm film(800, 600, new BoxFilter(1.f, 1.f), crop, "D:\\image_film.bmp", true);
	p_film = &film;
	float aspectratio = float(film.x_resolution) / float(film.y_resolution);
	float screen[] = { -aspectratio, aspectratio, -1.f, 1.f };
	if (aspectratio < 1.f) {
		screen[0] = -1.f;
		screen[1] =  1.f;
		screen[2] = -1.f / aspectratio;
		screen[3] = 1.f / aspectratio;
	}
	PerspectiveCamera camera(camera_trans, screen, fov, &film);

	//Transform ortho_proj = orthographic(1e-2f, 1000.f);baidu
	//OrthoCamera camera2(camera_trans, ortho_proj, crop, &film);

	SimpleSampler sampler(0, 800, 0, 600);
	//RandomSampler sampler(0, 800, 0, 600, 8);
	//StratifiedSampler sampler(0, 800, 0, 600, 1, 1, true);
	//WhittedIntegrator si;
	//DepthIntegrator si;
	//NormalIntegrator si;
	NDotLIntegrator si;

	SamplerRenderer renderer(&sampler, &camera, &si, nullptr);
	p_renderer = &renderer;
	renderer.render(&scene);
	
	//w.render_scene();
	pBitmap->CopyFromMemory(nullptr, &film.color_buffer[0], 4 * width);

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