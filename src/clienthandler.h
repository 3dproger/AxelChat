#ifndef CEF_TESTS_CEFSIMPLE_SIMPLE_HANDLER_H_
#define CEF_TESTS_CEFSIMPLE_SIMPLE_HANDLER_H_

#include "include/cef_client.h"

#include <list>
class CEFWebView;
class MainWindow;

class ClientHandler :	public CefClient,
								public CefLifeSpanHandler,
								public CefLoadHandler,
								public CefDisplayHandler
{
public:
	ClientHandler(MainWindow* view);
	~ClientHandler();

	CefRefPtr<CefBrowser> GetBrowser() const;

	// Request that all existing browser windows close.
	void CloseAllBrowsers(bool force_close);
	bool IsClosing() const { return m_isClosing; }

	// CefClient methods:
	virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() override { return this; }
	virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return this; }
	virtual CefRefPtr<CefLoadHandler> GetLoadHandler() override { return this; }

	// CefDisplayHandler methods:
	virtual void OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title) override;
	virtual void OnAddressChange(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString &url) override;

	// CefLifeSpanHandler methods:
	virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
	virtual bool DoClose(CefRefPtr<CefBrowser> browser) override;
	virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

	// CefLoadHandler methods:
	virtual void OnLoadError(CefRefPtr<CefBrowser> browser,
									CefRefPtr<CefFrame> frame,
									ErrorCode errorCode,
									const CefString& errorText,
									const CefString& failedUrl) override;


private:
	bool							m_isClosing;

	typedef std::list<CefRefPtr<CefBrowser> > BrowserList;
	BrowserList					m_slaveBrowserList;
	CefRefPtr<CefBrowser>	m_browser;
	MainWindow*					m_view;

        IMPLEMENT_REFCOUNTING(ClientHandler);
};

#endif  // CEF_TESTS_CEFSIMPLE_SIMPLE_HANDLER_H_
