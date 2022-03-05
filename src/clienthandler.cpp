#include "clienthandler.h"
#include "mainwindow.h"

#include <sstream>
#include <string>

#include "include/base/cef_bind.h"
#include "include/cef_app.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"


ClientHandler::ClientHandler(MainWindow* view)
	 : m_isClosing(false),
		m_browser(nullptr),
		m_view(view)
{
}

ClientHandler::~ClientHandler()
{
}


CefRefPtr<CefBrowser> ClientHandler::GetBrowser() const
{
	CEF_REQUIRE_UI_THREAD();

	return m_browser;
}

void ClientHandler::CloseAllBrowsers(bool force_close)
{
	CEF_REQUIRE_UI_THREAD();

	if (!CefCurrentlyOn(TID_UI))
	{
		// Execute on the UI thread.
		CefPostTask(TID_UI,
		base::Bind(&ClientHandler::CloseAllBrowsers, this, force_close));
		return;
	}

	if (m_slaveBrowserList.empty())
		return;

	BrowserList::const_iterator it = m_slaveBrowserList.begin();
	for (; it != m_slaveBrowserList.end(); ++it)
		(*it)->GetHost()->CloseBrowser(force_close);
}

/*------------------------------ CefDisplayHandler ---------------------------*/

void ClientHandler::OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString &title)
{
	CEF_REQUIRE_UI_THREAD();
	assert(m_view);

	m_view->setWindowTitle(QString::fromStdString(title.ToString()));
}

void ClientHandler::OnAddressChange(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString &url)
{
	CEF_REQUIRE_UI_THREAD();
	assert(m_view);

	m_view->updateLineEditUrl(QString::fromStdString(url.ToString()));
}

void ClientHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
	CEF_REQUIRE_UI_THREAD();

	if(!m_browser.get())
		m_browser = browser;
	else if (browser->IsPopup())
		m_slaveBrowserList.push_back(browser);
}

/*------------------------------ CefLifeSpanHandler --------------------------*/

bool ClientHandler::DoClose(CefRefPtr<CefBrowser> /*browser*/)
{
	CEF_REQUIRE_UI_THREAD();

	if(m_slaveBrowserList.empty())
		m_isClosing = true;

	return false;
}

void ClientHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
	CEF_REQUIRE_UI_THREAD();

	BrowserList::iterator bit = m_slaveBrowserList.begin();
	for (; bit != m_slaveBrowserList.end(); ++bit)
	{
		if ((*bit)->IsSame(browser))
		{
			m_slaveBrowserList.erase(bit);
			break;
		}
	}
}

/*------------------------------ CefLoadHandler ------------------------------*/

void ClientHandler::OnLoadError(	CefRefPtr<CefBrowser>,
											CefRefPtr<CefFrame> frame,
											ErrorCode errorCode,
											const CefString& errorText,
											const CefString& failedUrl)
{
	CEF_REQUIRE_UI_THREAD();

	// Don't display an error for downloaded files.
	if (errorCode == ERR_ABORTED)
	 return;

	// Display a load error message.
	std::stringstream ss;
	ss << "<html><body bgcolor=\"white\">"
		  "<h2>Failed to load URL " << std::string(failedUrl) <<
		  " with error " << std::string(errorText) << " (" << errorCode <<
		  ").</h2></body></html>";
	frame->LoadString(ss.str(), failedUrl);
}

