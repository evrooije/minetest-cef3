/*
Minetest
Copyright (C) 2017 Beerholder, Emiel van Rooijen <evrooije@outlook.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <malloc.h>

#include "include/cef_app.h"
#include "include/cef_client.h"
#include "include/cef_render_handler.h"

#include "mt_cef.h"

#include "irrlicht.h" // createDevice
#include "irrlichttypes_extrabloated.h"

/* MinetestBrowser */
MinetestBrowser::MinetestBrowser() {
	;
}

MinetestBrowser::~MinetestBrowser() {
	;
}

MinetestBrowser* m_browser = NULL;
int skippy = 0;

// static
MinetestBrowser* MinetestBrowser::GetInstance()
{
	return m_browser;
}

// static
bool MinetestBrowser::IsInitialized()
{
	return (m_browser != NULL);
}

// static
void MinetestBrowser::Initialize()
{
	skippy = 0;
	m_browser = new MinetestBrowser();

/*    char  cefarg0[] = "./cefclient";
    char  cefarg1[] = "enable-gpu";
    char* cefargv[] = { &cefarg0[0], &cefarg1[0], NULL };
    int   cefargc   = (int)(sizeof(cefargv) / sizeof(cefargv[0])) - 1;

	CefMainArgs args(cefargc, cefargv);
*/
	CefMainArgs args;
	CefSettings settings;

	settings.no_sandbox = true;
	settings.single_process = false;
	settings.multi_threaded_message_loop = false;
	settings.windowless_rendering_enabled = true;
	settings.command_line_args_disabled = true;
//	settings.windowless_frame_rate = 30;
//	settings.log_severity = LOGSEVERITY_VERBOSE;
	settings.log_severity = LOGSEVERITY_DISABLE;
	CefString(&settings.log_file).FromASCII("cef.log");
	CefString(&settings.browser_subprocess_path).FromASCII("./cefclient");

	// Aaaaaand ... GO!
	CefInitialize(args, settings, nullptr, nullptr);

}

// static
void MinetestBrowser::Shutdown()
{
	GetInstance()->CloseWebPages();
	CefShutdown();
}

// static
void MinetestBrowser::Update()
{
    if (skippy >= 1)
    {
    	if (GetInstance()->m_webPages.begin() != GetInstance()->m_webPages.end()) {
			for (auto iter = GetInstance()->m_webPages.begin(); iter != GetInstance()->m_webPages.end(); ++iter){
				auto cur = iter->first; // pointer to Node
				if (GetInstance()->m_webPages[cur] != NULL) {
					GetInstance()->m_webPages[cur]->ProcessEvents();
				}
			}
			CefDoMessageLoopWork();
    	}
		skippy = 0;
    }
    skippy++;
}

/* ITexture methods */
WebPage* MinetestBrowser::CreateWebPage(std::string name, video::IVideoDriver *driver,
	video::ITexture *texture, v2s32 geom)
{

	WebPage* webPage = new WebPage(name, driver, texture, geom.X, geom.Y);
	m_webPages[name] = webPage;

	return webPage;
}

WebPage* MinetestBrowser::CreateWebPage(std::string name, video::IVideoDriver *driver,
	video::ITexture *texture, v2s32 geom, std::string url)
{
	WebPage* webPage = CreateWebPage(name, driver, texture, geom);
	webPage->Open(url);

	return webPage;
}

WebPage* MinetestBrowser::CreateWebPage(std::string name, video::IVideoDriver *driver,
	video::ITexture *texture, core::dimension2d<u32> dimension)
{
	WebPage* webPage = new WebPage(name, driver, texture, dimension.Width, dimension.Height);
	m_webPages[name] = webPage;

	return webPage;
}

WebPage* MinetestBrowser::CreateWebPage(std::string name, video::IVideoDriver *driver,
	video::ITexture *texture, core::dimension2d<u32> dimension, std::string url)
{
	WebPage* webPage = CreateWebPage(name, driver, texture, dimension);
	webPage->Open(url);

	return webPage;
}

/* IImage methods */
WebPage* MinetestBrowser::CreateWebPage(std::string name, video::IVideoDriver *driver,
	video::IImage *image, core::dimension2d<u32> dimension)
{
	WebPage* webPage = new WebPage(name, driver, image,
        dimension.Width, dimension.Height);
	m_webPages[name] = webPage;

	return webPage;
}

WebPage* MinetestBrowser::CreateWebPage(std::string name, video::IVideoDriver *driver,
	video::IImage *image, core::dimension2d<u32> dimension, std::string url)
{
	WebPage* webPage = CreateWebPage(name, driver, image, dimension);
	webPage->Open(url);

	return webPage;
}

/* IGUIEnvironment methods */
WebPage* MinetestBrowser::CreateWebPage(std::string name, video::IVideoDriver *driver,
	irr::gui::IGUIEnvironment* guiEnv, v2s32 pos, v2s32 geom)
{
	video::ITexture *texture =
		driver->addTexture(core::dimension2d<u32>(geom.X, geom.Y),
		   io::path("browsertexture"),
		   irr::video::ECF_A8R8G8B8);

	irr::gui::IGUIImage* image = guiEnv->addImage(texture, core::position2d<s32>(pos.X, pos.Y));
	image->setName("browser");

	WebPage* webPage = new WebPage(name, driver, image, texture, geom.X, geom.Y);
	webPage->SetSize(geom.X, geom.Y);
	m_webPages[name] = webPage;

	return webPage;
}

WebPage* MinetestBrowser::CreateWebPage(std::string name, video::IVideoDriver *driver,
	irr::gui::IGUIEnvironment* guiEnv, v2s32 pos, v2s32 geom, std::string url)
{
	WebPage* webPage = CreateWebPage(name, driver, guiEnv, pos, geom);
	webPage->Open(url);

	return webPage;
}

WebPage* MinetestBrowser::GetWebPage(std::string name) {
    return m_webPages[name];
}

void MinetestBrowser::CloseWebPage(std::string name)
{
	auto iter = m_webPages.find(name);
	if (iter != m_webPages.end()) {
		WebPage* webPage = iter->second;
		m_webPages.erase(iter);

		if (webPage == NULL) return;

		webPage->Close();
		delete webPage;
	}
}

void MinetestBrowser::CloseWebPages() {
	for (auto iter = m_webPages.begin(); iter != m_webPages.end(); ++iter){
		auto cur = iter->first; // pointer to Node
		if (m_webPages[cur] != NULL) {
			m_webPages[cur]->Close();
		}
	}
}

/* WebPage */
WebPage::WebPage(std::string name, video::IVideoDriver *driver,
		irr::gui::IGUIImage *guiImage, video::ITexture *texture,
		int width, int height)
	: m_name(name)
	, m_width(width)
	, m_height(height)
	, m_driver(driver)
	, m_guiImage(guiImage)
	, m_texture(texture)
{
	MinetestCefRenderHandler* rh = new MinetestCefRenderHandler(m_driver, m_texture,
        width, height);
	m_client = new MinetestCefClient(rh);
}

WebPage::WebPage(std::string name, video::IVideoDriver *driver,
		video::IImage *image, int width, int height)
	: m_name(name)
	, m_width(width)
	, m_height(height)
	, m_driver(driver)
	, m_image(image)
{
	MinetestCefRenderHandler* rh = new MinetestCefRenderHandler(m_driver, m_image,
        width, height);
	m_client = new MinetestCefClient(rh);
}

WebPage::WebPage(std::string name, video::IVideoDriver *driver,
		video::ITexture *texture, int width, int height)
	: m_name(name)
	, m_width(width)
	, m_height(height)
	, m_driver(driver)
	, m_texture(texture)
{
	MinetestCefRenderHandler* rh = new MinetestCefRenderHandler(m_driver, m_texture,
        width, height);
	m_client = new MinetestCefClient(rh);
}

WebPage::~WebPage()
{
	;
}

void WebPage::PostEvent(WebPageEvent event)
{
	m_mutex.lock();
	m_events.push(event);
	m_mutex.unlock();
}

void WebPage::ProcessEvents()
{
	m_mutex.lock();
	if (!m_events.empty()) {
		WebPageEvent wpevent = m_events.front();
		m_events.pop();

		switch (wpevent.m_type) {
			case WPET_IRRLICHT:
				ProcessIrrlichtEvent(wpevent.m_event);
				break;
			case WPET_ACTION_CREATE:
				break;
			case WPET_ACTION_OPEN_URL:
				Open(wpevent.m_parameter);
				break;
			case WPET_ACTION_BACK:
				Back();
				break;
			case WPET_ACTION_FORWARD:
				Forward();
				break;
			case WPET_ACTION_CLOSE:
				break;
			default:
				break;
		}
	}
	m_mutex.unlock();
}

void WebPage::ProcessIrrlichtEvent(irr::SEvent event)
{
	switch (event.EventType) {
		case EET_MOUSE_INPUT_EVENT:
			switch (event.MouseInput.Event) {
				case EMIE_MOUSE_MOVED:
					OnMouseMoved(event.MouseInput.X, event.MouseInput.Y);
					break;
				case EMIE_LMOUSE_PRESSED_DOWN:
					OnLeftMousePressedDown(event.MouseInput.X, event.MouseInput.Y);
					break;
				case EMIE_LMOUSE_LEFT_UP:
					OnLeftMouseLeftUp(event.MouseInput.X, event.MouseInput.Y);
					break;
				case EMIE_LMOUSE_DOUBLE_CLICK:
					OnLeftMouseDoubleClick(event.MouseInput.X, event.MouseInput.Y);
					break;
				default:
					// The click took place on the browser, so no need to
					// let the system search for other potential candidates
					// to handle the event
					break;
			};
			break;
		case EET_KEY_INPUT_EVENT:
            if (event.KeyInput.PressedDown) {
                OnKeyPressed(event.KeyInput);
            }
			break;
		default:
			break;
	}
}

void WebPage::Open(std::string url)
{
	CefWindowInfo window_info;
	CefBrowserSettings cefBrowserSettings;
	cefBrowserSettings.windowless_frame_rate = 30;

	std::size_t windowHandle = 0;
	window_info.SetAsWindowless(windowHandle, false);

	m_cefbrowser = CefBrowserHost::CreateBrowserSync(window_info, m_client.get(), url, cefBrowserSettings, NULL);
}

void WebPage::Close()
{
	m_cefbrowser->GetHost()->CloseBrowser(false);
	if (m_guiImage != NULL) {
		m_guiImage->remove();
	}
}

/*
  VKEY_BROWSER_BACK = 0xA6,
  VKEY_BROWSER_FORWARD = 0xA7,
*/

void WebPage::Back()
{
    CefKeyEvent e;

	e.windows_key_code = 0xA6;
	// e.native_key_code = keycode;
	e.type = KEYEVENT_RAWKEYDOWN;
	m_cefbrowser->GetHost()->SendKeyEvent(e);
	e.type = KEYEVENT_KEYUP;
	m_cefbrowser->GetHost()->SendKeyEvent(e);
}

void WebPage::Forward()
{
    CefKeyEvent e;

	e.windows_key_code = 0xA7;
	// e.native_key_code = keycode;
	e.type = KEYEVENT_RAWKEYDOWN;
	m_cefbrowser->GetHost()->SendKeyEvent(e);
	e.type = KEYEVENT_KEYUP;
	m_cefbrowser->GetHost()->SendKeyEvent(e);
}

void WebPage::SetSize(int width, int height)
{
	m_width = width;
	m_height = height;
}

void WebPage::OnMouseMoved(s32 x, s32 y)
{
    m_cefbrowser->GetHost()->SetFocus(true);
    CefMouseEvent e;
    // We need to scale and offset the x, y that is to be sent to the browser!
    e.x = x - m_guiImage->getAbsolutePosition().UpperLeftCorner.X;
    e.y = y - m_guiImage->getAbsolutePosition().UpperLeftCorner.Y;
    m_cefbrowser->GetHost()->SendMouseMoveEvent(e, false);
}

void WebPage::OnLeftMousePressedDown(s32 x, s32 y)
{
    m_cefbrowser->GetHost()->SetFocus(true);
    CefMouseEvent e;
    // We need to scale and offset the x, y that is to be sent to the browser!
    e.x = x - m_guiImage->getAbsolutePosition().UpperLeftCorner.X;
    e.y = y - m_guiImage->getAbsolutePosition().UpperLeftCorner.Y;
    m_cefbrowser->GetHost()->SendMouseClickEvent(e, MBT_LEFT, false, 1);
}

void WebPage::OnLeftMouseLeftUp(s32 x, s32 y)
{
    m_cefbrowser->GetHost()->SetFocus(true);
    CefMouseEvent e;
    // We need to scale and offset the x, y that is to be sent to the browser!
    e.x = x - m_guiImage->getAbsolutePosition().UpperLeftCorner.X;
    e.y = y - m_guiImage->getAbsolutePosition().UpperLeftCorner.Y;
    m_cefbrowser->GetHost()->SendMouseClickEvent(e, MBT_LEFT, true, 1);
}

void WebPage::OnLeftMouseDoubleClick(s32 x, s32 y)
{
    m_cefbrowser->GetHost()->SetFocus(true);
    CefMouseEvent e;
    // We need to scale and offset the x, y that is to be sent to the browser!
    e.x = x - m_guiImage->getAbsolutePosition().UpperLeftCorner.X;
    e.y = y - m_guiImage->getAbsolutePosition().UpperLeftCorner.Y;
    m_cefbrowser->GetHost()->SendMouseClickEvent(e, MBT_LEFT, false, 2);
    m_cefbrowser->GetHost()->SendMouseClickEvent(e, MBT_LEFT, true, 2);
}

void WebPage::OnMouseWheel(s32 x, s32 y, f32 delta)
{
    m_cefbrowser->GetHost()->SetFocus(true);
    CefMouseEvent e;
    e.x = x - m_guiImage->getAbsolutePosition().UpperLeftCorner.X;
    e.y = y - m_guiImage->getAbsolutePosition().UpperLeftCorner.Y;

    m_cefbrowser->GetHost()->SendMouseWheelEvent(e, 0, delta * 50);
}

void WebPage::OnKeyPressed(SEvent::SKeyInput keyinput)
{
    m_cefbrowser->GetHost()->SetFocus(true);
    CefKeyEvent e;

    wchar_t key = keyinput.Char;
    EKEY_CODE keycode = keyinput.Key;

    if (key == 0 && keycode == 0) {
    } else if ((key < 32 || key > 126) && key != 13) { //65 (upper A) 97 (lower a) - 90 (upper Z) 122 (lower z)
        e.windows_key_code = keycode;
        // e.native_key_code = keycode;
        e.type = KEYEVENT_RAWKEYDOWN;
        m_cefbrowser->GetHost()->SendKeyEvent(e);
        e.type = KEYEVENT_KEYUP;
        m_cefbrowser->GetHost()->SendKeyEvent(e);
/*    } else if () { // Modifiers
        switch (key) {
            case 160:
                e.modifiers |= 1 << 1; // Shift
            case 161:
                e.modifiers |= 1 << 1;
            case 162:
                e.modifiers |= 1 << 2; // Control
            case 163:
                e.modifiers |= 1 << 2;
            case 164:
                e.modifiers |= 1 << 3; // Alt
            case 165:
                e.modifiers |= 1 << 3;
            default:
            e.type = KEYEVENT_RAWKEYDOWN;
            m_cefbrowser->GetHost()->SendKeyEvent(e);
        }*/
    } else if (key >= 160 && key <= 165) {
    } else {
        e.windows_key_code = key;
        e.character = key;
        e.type = KEYEVENT_CHAR;

        // The next segment may need to be changed to a variable
        // in the class, but not sure if this is needed for the
        // browser or not. To be investigated
        if (keyinput.Shift) {
            e.modifiers |= 1 << 1; // Shift
        }
        if (keyinput.Control) {
            e.modifiers |= 1 << 2; // Control
        }

        m_cefbrowser->GetHost()->SendKeyEvent(e);
    }
}

MinetestCefRenderHandler::MinetestCefRenderHandler(video::IVideoDriver *driver,
        video::ITexture *texture, int width, int height)
	: m_driver(driver)
	, m_texture(texture)
	, m_width(width)
	, m_height(height)
{}

MinetestCefRenderHandler::MinetestCefRenderHandler(video::IVideoDriver *driver,
        video::IImage *image, int width, int height)
	: m_driver(driver)
	, m_image(image)
	, m_width(width)
	, m_height(height)
{}

void MinetestCefRenderHandler::SetSize(int width, int height)
{
	m_width = width;
	m_height = height;
}

bool MinetestCefRenderHandler::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect)
{
    rect = CefRect(0, 0, m_width, m_height);
    return true;
}

void MinetestCefRenderHandler::OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList &dirtyRects, const void *buffer, int width, int height)
{
/*
 * x = 1 y = 1, w = 4, h = 2
 * Width
 * 00000000
 * 01111100
 * 01111100
 * 00000000
 *
 * Skip y * width
 * Repeat for height number of times
 *   Copy offset x to offset x + width
 *   Skip width -
 */
	if (dirtyRects.empty()) return;

	if (width <= 2 && height <= 2) {
		// Ignore really small buffer sizes while the widget is starting up.
		return;
	}

	std::vector<int>::size_type sz = dirtyRects.size();

	irr::u8* data;

	if (m_texture != NULL) {
		data = (irr::u8*) m_texture->lock();
	} else if (m_image != NULL) {
		data = (irr::u8*) m_image->lock();
	} else {
		return;
	}

	for (unsigned i = 0; i < sz; i++) {
		CefRect rect = dirtyRects[i];
		if (rect.x == 0 && rect.y == 0 && width == rect.width && height == rect.height) {
			memcpy(data, buffer, width * height * 4);
		} else {
			// These calculations are critical. Do NOT make a mistake here or you WILL
			// overwrite memory in use by other parts of the game, causing RANDOM
			// crashes that seem totally UNRELATED to the below code
			unsigned yoffset = rect.y * width * 4;
			unsigned xoffset = rect.x * 4;
			unsigned skip = (width - rect.width) * 4;
			unsigned curpos = yoffset + xoffset;
			for (int j = 0; j < rect.height; j++) {
				// Danger, Will Robinson!
				memcpy(((uint8_t*)data) + curpos, ((uint8_t*)buffer) + curpos, rect.width * 4);
				curpos = curpos + (rect.width * 4) + skip;
			}
		}
	}

	if (m_texture != NULL) {
		m_texture->unlock();
	} else if (m_image != NULL) {
		m_image->unlock();
	}

}

MinetestCefClient::MinetestCefClient(MinetestCefRenderHandler *renderHandler)
	: m_renderHandler(renderHandler)
{}

CefRefPtr<CefRenderHandler> MinetestCefClient::GetRenderHandler()
{
	return m_renderHandler;
}
