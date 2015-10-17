#include "RenderFishGUI.hpp"

enum Direction {
	direction_non = 0,
	direction_up = -1,
	direction_down = 1,
	direction_left = -2,
	driection_right = 2,
};

struct SideBarState {
	D2D1_RECT_F rect;
	//int left;
	//int right;
	//int width = 200;
	//int height;
	int x_margin_left = 10;
	int x_margin_right = 18;
	int y_margin = 5;
	int y_cell_height = 30;
	int y_start = 0;
	int y_filled = 0;

	int cell_numbers_last_draw = 0;
	int scroll_target_y = 0;

	int scroll_direction = direction_non;

	int avaliable_width() {
		return int(rect.right - rect.left) - x_margin_left - x_margin_right;
	}

	void update() {
		if (y_filled < scroll_target_y)
			y_start += 5;
	}
};

static SideBarState side_bar;

struct GUIState {
	int width;
	int height;
	int next_id = 0;

	void reset() {
		next_id = 0;
	}
};

static GUIState gui_state;

struct ButtonState {
	//int next_id = 0;
	int button_id_clicked = -1;
};

static ButtonState g_button;

HRESULT RenderFishGUI::CreateD2DResource(ID2D1Factory* pD2DFactory, HWND hWnd, ID2D1HwndRenderTarget* pRenderTarget)
{
	RenderFishGUI::pD2DFactory = pD2DFactory;
	RenderFishGUI::pRenderTarget = pRenderTarget;

	HRESULT hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&pDWriteFactory));
	HR(hr);

	RECT rc;
	GetClientRect(hWnd, &rc);
	gui_state.width = rc.right - rc.left;
	gui_state.height = rc.bottom - rc.top;

	HR(pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &pBlackBrush));
	HR(pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &pRedBrush));
	HR(pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Gray), &pGrayBrush));
	HR(pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &pWhiteBrush));
	HR(pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &pReuseableBrush));

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

	return S_OK;
}

void RenderFishGUI::Cleanup()
{
	SAFE_RELEASE(pBlackBrush);
	SAFE_RELEASE(pRedBrush);
	SAFE_RELEASE(pGrayBrush);
	SAFE_RELEASE(pWhiteBrush);
	SAFE_RELEASE(pRedBrush);
	SAFE_RELEASE(pReuseableBrush);
	SAFE_RELEASE(pDWriteFactory);
	SAFE_RELEASE(pTexFormat);
	SAFE_RELEASE(pTextLayout);
}

void RenderFishGUI::BeginFrame()
{
	side_bar.cell_numbers_last_draw = gui_state.next_id;
	gui_state.reset();
	mouse_state.hot_item = -1;
	SideBar();
}

void RenderFishGUI::EndFrame()
{
	for (int i = 1; i <= gui_state.height / 30; ++i) {
		pRenderTarget->DrawLine(D2D1::Point2F(gui_state.width - (side_bar.rect.right - side_bar.rect.left), i * 30),
			D2D1::Point2F(gui_state.width, i * 30), pBlackBrush);
	}
}

void RenderFishGUI::BeginDialog()
{

}

void RenderFishGUI::SideBar(int width /*= 250*/)
{
	int window_width = gui_state.width;
	int window_height = gui_state.height;
	side_bar.rect = D2D1::RectF((float)window_width - width, 0.f, (float)window_width, (float)window_height);
	side_bar.y_filled = side_bar.y_start + side_bar.y_margin;

	fill_rect(window_width - width, 0, width, window_height, pGrayBrush);

	if (mouse_state.mouse_wheel_rotating) {
		// scrolling direction changed, stop scrolling immediately
		if (side_bar.scroll_direction * mouse_state.mouse_wheel_z_delta < 0)
			side_bar.scroll_target_y = side_bar.y_start;
		side_bar.scroll_direction = mouse_state.mouse_wheel_z_delta > 0 ? direction_up : direction_down;
		side_bar.scroll_target_y += mouse_state.mouse_wheel_z_delta;
		mouse_state.mouse_wheel_rotating = false;
	}

	const int scroll_pixels_per_frame = 30;
	if (abs(side_bar.scroll_target_y - side_bar.y_start) > scroll_pixels_per_frame)
		side_bar.y_start += (side_bar.scroll_target_y > side_bar.y_start) ? scroll_pixels_per_frame : -scroll_pixels_per_frame;
	if (side_bar.y_start > 0)
		side_bar.y_start = 0;
	
	// scroll bar
	const int scroll_bar_width = 2;
	pReuseableBrush->SetColor(D2D1::ColorF(0.6f, 0.6f, 0.6f));
	fill_rect(window_width - scroll_bar_width, 0, scroll_bar_width, window_height, pReuseableBrush);
	pReuseableBrush->SetColor(D2D1::ColorF(0.2f, 0.2f, 0.2f));
	if (side_bar.cell_numbers_last_draw * side_bar.y_cell_height > window_height) {
		int bar_y_start =  - 2;
		bar_y_start -= 1.0f * side_bar.y_start / (side_bar.cell_numbers_last_draw * side_bar.y_cell_height) * window_height;
		int bar_length = float(window_height) / (side_bar.cell_numbers_last_draw * side_bar.y_cell_height) * window_height;
		fill_rect(window_width - scroll_bar_width, bar_y_start, scroll_bar_width, bar_length, pReuseableBrush);
	}
}


void RenderFishGUI::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_MOUSEMOVE:
		mouse_state.pos_x = LOWORD(lParam);
		mouse_state.pos_y = HIWORD(lParam);
		break;
	case WM_LBUTTONDOWN:
		mouse_state.mouse_down = true;
		break;
	case WM_LBUTTONUP:
		mouse_state.mouse_down = false;
		break;
	case WM_MOUSEWHEEL:
		mouse_state.mouse_wheel_rotating = true;
		mouse_state.mouse_wheel_z_delta = (short)HIWORD(wParam);
		break;
	}
}

bool RenderFishGUI::Button(const WCHAR* label /*= nullptr*/)
{
	bool clicked = false;

	int id = gui_state.next_id ++;
	int x = (int)side_bar.rect.left + side_bar.x_margin_left, y = side_bar.y_filled + side_bar.y_margin;
	int width = side_bar.avaliable_width();
	int height = side_bar.y_cell_height;
	side_bar.y_filled += side_bar.y_margin * 2 + height;

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

bool RenderFishGUI::Button(const char* label)
{
	return Button(ToWString<const char*>(label).c_str());
}

void RenderFishGUI::Label(const WCHAR* text, GUIAlignment text_alignment /*= align_horiontally_center*/)
{
	int id = gui_state.next_id++;
	float width = 128.f, height = (float)side_bar.y_cell_height;
	float x = side_bar.rect.left + side_bar.x_margin_left, y = float(side_bar.y_filled + side_bar.y_margin);
	width = side_bar.avaliable_width();
	pTexFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

	DWRITE_TEXT_ALIGNMENT alignment = DWRITE_TEXT_ALIGNMENT_CENTER;
	if (text_alignment == align_horiontally_left)
		alignment = DWRITE_TEXT_ALIGNMENT_LEADING;
	else if (text_alignment == align_horiontally_right)
		alignment = DWRITE_TEXT_ALIGNMENT_TRAILING;
	pTexFormat->SetTextAlignment(alignment);
	SAFE_RELEASE(pTextLayout);
	HR(pDWriteFactory->CreateTextLayout(text, wcslen(text), pTexFormat, width, height, &pTextLayout));
	//DWRITE_TEXT_METRICS mertics;
	//pTextLayout->GetMetrics(&mertics);
	//if (height < mertics.height) height = mertics.height;
	//pTextLayout->SetMaxHeight(height);
	pRenderTarget->DrawTextLayout(D2D1::Point2F(x, y), pTextLayout, pBlackBrush);

	side_bar.y_filled += int(side_bar.y_margin * 2 + height);
}

void RenderFishGUI::Label(const char* text, GUIAlignment text_alignment /*= align_horiontally_center*/)
{
	Label(ToWString(text).c_str(), text_alignment);
}

void RenderFishGUI::NumberBox(int* val)
{
	int id = gui_state.next_id++;
	float width = 128.f, height = (float)side_bar.y_cell_height;
	float x = side_bar.rect.left + side_bar.x_margin_left, y = float(side_bar.y_filled + side_bar.y_margin);
	width = side_bar.avaliable_width();
	pTexFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	pTexFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
	SAFE_RELEASE(pTextLayout);
	auto text = L"10";
	HR(pDWriteFactory->CreateTextLayout(text, wcslen(text), pTexFormat, width, height, &pTextLayout));
	//DWRITE_TEXT_METRICS mertics;
	//pTextLayout->GetMetrics(&mertics);
	//if (height < mertics.height) height = mertics.height;
	//pTextLayout->SetMaxHeight(height);
	const int w = 50;
	const int h = 20;
	const int x_margin = 4;
	pRenderTarget->DrawRectangle(D2D1::RectF(x, y + (30 - h) / 2, x + w, y + (30+h)/2), pBlackBrush);
	if (mouse_in_region(x, y + (30 - h) / 2, w, h)) {
		pRenderTarget->FillRectangle(D2D1::RectF(x, y + (30 - h) / 2, x + w, y + (30 + h) / 2), pWhiteBrush);
		//pTextLayout->SetDrawingEffect()
		if (mouse_state.mouse_down) {
			pRenderTarget->FillRectangle(D2D1::RectF(x + x_margin, y + (30 - 15) / 2, x + 20, y + (30 + 15) / 2), pRedBrush);
		}
	}
	pRenderTarget->DrawTextLayout(D2D1::Point2F(x+x_margin, y), pTextLayout, pBlackBrush);

	side_bar.y_filled += int(side_bar.y_margin * 2 + height);
}

template<typename T>
void RenderFishGUI::Slider(const char* str, T *pVal, T min, T max)
{
	int id = gui_state.next_id++;
	Label(str);
	int width = side_bar.avaliable_width();
	int x = int(side_bar.rect.left) + side_bar.x_margin_left;
	//x = 10;
	int y = side_bar.y_filled;
	side_bar.y_filled += side_bar.y_cell_height;
	int y_cneter = y + side_bar.y_cell_height / 2;

	auto pBrush = pBlackBrush;
	float percent = float(*pVal - min) / float(max - min);
	const int region_height = 16;
	if (mouse_in_region(x, y_cneter - region_height / 2, width, region_height)) {
		pBrush = pWhiteBrush;
		if (mouse_state.mouse_down) {
			percent = float(T(float(mouse_state.pos_x - x) / width * (max - min))) / (max - min);
			*pVal = min + T((max - min) * percent);
		}
	}

	pRenderTarget->DrawRectangle(D2D1::RectF(float(x), float(y_cneter - 2 / 2), float(x + width), float(y_cneter + 2 / 2)), pBrush);
	D2D1_ELLIPSE circle{ D2D1_POINT_2F{ float(x + int(percent * width)), float(y + side_bar.y_cell_height / 2) }, 5.f, 5.f };
	pRenderTarget->FillEllipse(circle, pBrush);
}

void RenderFishGUI::draw_rect(int x, int y, int w, int h)
{
	pRenderTarget->DrawRectangle(D2D1::RectF(float(x), float(y), float(x + w), float(y + h)), pBlackBrush);
}

void RenderFishGUI::draw_rect(int x, int y, int w, int h, ID2D1SolidColorBrush* border_brush)
{
	pRenderTarget->DrawRectangle(D2D1::RectF(float(x), float(y), float(x + w), float(y + h)), border_brush);	
}

void RenderFishGUI::fill_rect(int x, int y, int w, int h, ID2D1SolidColorBrush* fill_brush)
{
	pRenderTarget->FillRectangle(D2D1::RectF(float(x), float(y), float(x + w), float(y + h)), fill_brush);
}

void RenderFishGUI::draw_rounded_rect(int x, int y, int w, int h, ID2D1SolidColorBrush* border_brush, ID2D1SolidColorBrush* fill_brush /*= nullptr*/)
{
	// Draw Rectangle
	//pRenderTarget->DrawRectangle(D2D1::RectF(float(x), float(y), float(x + w), float(y + h)), brush);
	if (fill_brush != nullptr)
		pRenderTarget->FillRoundedRectangle(
			D2D1::RoundedRect(D2D1::RectF(float(x), float(y), float(x + w), float(y + h)), 5.f, 5.f), fill_brush);
	pRenderTarget->DrawRoundedRectangle(
		D2D1::RoundedRect(D2D1::RectF(float(x), float(y), float(x + w), float(y + h)), 5.f, 5.f), border_brush);
}

//RECT RenderFishGUI::rc;
MouseState RenderFishGUI::mouse_state;
ID2D1Factory* RenderFishGUI::pD2DFactory(nullptr);
ID2D1HwndRenderTarget* RenderFishGUI::pRenderTarget(nullptr);
IDWriteFactory* RenderFishGUI::pDWriteFactory(nullptr);
IDWriteTextLayout * RenderFishGUI::pTextLayout(nullptr);
IDWriteTextFormat * RenderFishGUI::pTexFormat(nullptr);
ID2D1SolidColorBrush* RenderFishGUI::pRedBrush(nullptr);
ID2D1SolidColorBrush* RenderFishGUI::pReuseableBrush(nullptr);
ID2D1SolidColorBrush* RenderFishGUI::pWhiteBrush(nullptr);
ID2D1SolidColorBrush* RenderFishGUI::pGrayBrush(nullptr);
ID2D1SolidColorBrush* RenderFishGUI::pBlackBrush(nullptr);
