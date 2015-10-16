#include "RenderFishGUI.hpp"

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

	int avaliable_width() {
		return int(rect.right - rect.left) - x_margin_left - x_margin_right;
	}
};

static SideBarState g_side_bar;

struct GUIState {
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

	GetClientRect(hWnd, &rc);

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
	g_side_bar.cell_numbers_last_draw = gui_state.next_id;
	gui_state.reset();
	mouse_state.hot_item = -1;
	SideBar();
}

void RenderFishGUI::EndFrame()
{
}

void RenderFishGUI::BeginDialog()
{

}

void RenderFishGUI::SideBar(int width /*= 250*/)
{
	int window_width = rc.right - rc.left;
	int window_height = rc.bottom - rc.top;
	g_side_bar.rect = D2D1::RectF((float)window_width - width, rc.top, (float)window_width, rc.bottom);
	g_side_bar.y_filled = g_side_bar.y_start + g_side_bar.y_margin;

	pRenderTarget->FillRectangle(D2D1::RectF(float(window_width - width), 0.f, (float)window_width, float(rc.bottom - rc.top)), pGrayBrush);

	if (mouse_state.mouse_wheel_rotating) {
		g_side_bar.y_start += mouse_state.mouse_wheel_z_delta;
	}
	if (g_side_bar.y_start > 0)
		g_side_bar.y_start = 0;
	mouse_state.mouse_wheel_rotating = false;

	// scroll bar
	pReuseableBrush->SetColor(D2D1::ColorF(0.6f, 0.6f, 0.6f));
	pRenderTarget->FillRectangle(D2D1::RectF(float(rc.right - 2), rc.top, float(rc.right), rc.bottom), pReuseableBrush);
	pReuseableBrush->SetColor(D2D1::ColorF(0.2f, 0.2f, 0.2f));
	if (g_side_bar.cell_numbers_last_draw * g_side_bar.y_cell_height > window_height) {
		int bar_y_start = rc.top - 2;
		bar_y_start -= 1.0f * g_side_bar.y_start / (g_side_bar.cell_numbers_last_draw * g_side_bar.y_cell_height) * window_height;
		int bar_length = float(window_height) / (g_side_bar.cell_numbers_last_draw * g_side_bar.y_cell_height) * window_height;
		pRenderTarget->FillRectangle(D2D1::RectF(float(rc.right - 2), rc.top + bar_y_start, float(rc.right), rc.top + bar_y_start + bar_length), pReuseableBrush);
	}
}


bool RenderFishGUI::Button(const WCHAR* label /*= nullptr*/)
{
	bool clicked = false;

	int id = gui_state.next_id ++;
	int x = (int)g_side_bar.rect.left + g_side_bar.x_margin_left, y = g_side_bar.y_filled + g_side_bar.y_margin;
	int width = g_side_bar.avaliable_width();
	int height = g_side_bar.y_cell_height;
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

bool RenderFishGUI::Button(const char* label)
{
	return Button(ToWString<const char*>(label).c_str());
}

void RenderFishGUI::Label(const WCHAR* text, DWRITE_TEXT_ALIGNMENT text_alignment /*= DWRITE_TEXT_ALIGNMENT_LEADING*/)
{
	int id = gui_state.next_id++;
	float width = 128.f, height = (float)g_side_bar.y_cell_height;
	float x = g_side_bar.rect.left + g_side_bar.x_margin_left, y = float(g_side_bar.y_filled + g_side_bar.y_margin);
	width = g_side_bar.avaliable_width();
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

void RenderFishGUI::Label(const char* text, DWRITE_TEXT_ALIGNMENT text_alignment /*= DWRITE_TEXT_ALIGNMENT_LEADING*/)
{
	Label(ToWString(text).c_str(), text_alignment);
}

void RenderFishGUI::NumberBox(int* val)
{
	int id = gui_state.next_id++;
	float width = 128.f, height = (float)g_side_bar.y_cell_height;
	float x = g_side_bar.rect.left + g_side_bar.x_margin_left, y = float(g_side_bar.y_filled + g_side_bar.y_margin);
	width = g_side_bar.avaliable_width();
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

	g_side_bar.y_filled += int(g_side_bar.y_margin * 2 + height);
}

template<typename T>
void RenderFishGUI::Slider(const char* str, T *pVal, T min, T max)
{
	int id = gui_state.next_id++;
	Label(str);
	int width = g_side_bar.avaliable_width();
	int x = int(g_side_bar.rect.left) + g_side_bar.x_margin_left;
	//x = 10;
	int y = g_side_bar.y_filled;
	g_side_bar.y_filled += g_side_bar.y_cell_height;
	int y_cneter = y + g_side_bar.y_cell_height / 2;

	auto pBrush = pBlackBrush;
	float percent = float(*pVal - min) / float(max - min);
	const int region_height = 16;
	if (mouse_in_region(x, y_cneter - region_height / 2, width, region_height)) {
		pBrush = pWhiteBrush;
		if (mouse_state.mouse_down) {
			percent = float(T(float(mouse_state.mouse_x - x) / width * (max - min))) / (max - min);
			*pVal = min + T((max - min) * percent);
		}
	}

	pRenderTarget->DrawRectangle(D2D1::RectF(float(x), float(y_cneter - 2 / 2), float(x + width), float(y_cneter + 2 / 2)), pBrush);
	D2D1_ELLIPSE circle{ D2D1_POINT_2F{ float(x + int(percent * width)), float(y + g_side_bar.y_cell_height / 2) }, 5.f, 5.f };
	pRenderTarget->FillEllipse(circle, pBrush);
}

void RenderFishGUI::draw_rounded_rect(int x, int y, int w, int h, ID2D1SolidColorBrush* brush, ID2D1SolidColorBrush* fill_brush /*= nullptr*/)
{
	// Draw Rectangle
	//pRenderTarget->DrawRectangle(D2D1::RectF(float(x), float(y), float(x + w), float(y + h)), brush);
	if (fill_brush != nullptr)
		pRenderTarget->FillRoundedRectangle(
			D2D1::RoundedRect(D2D1::RectF(float(x), float(y), float(x + w), float(y + h)), 5.f, 5.f), fill_brush);
	pRenderTarget->DrawRoundedRectangle(
		D2D1::RoundedRect(D2D1::RectF(float(x), float(y), float(x + w), float(y + h)), 5.f, 5.f), brush);
}

RECT RenderFishGUI::rc;
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
