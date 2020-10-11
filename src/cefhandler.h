#pragma once

#include <QObject>
#include "include/cef_app.h"
#include "include/internal/cef_types_wrappers.h"
#include "include/cef_client.h"

#include "include/cef_client.h"
#include "include/cef_urlrequest.h"
#include <list>

#include <QDebug>

class QtCefHandler : public CefClient,
                      public CefDisplayHandler,
                      public CefLifeSpanHandler,
                      public CefLoadHandler,
                      public CefRequestHandler,
                      public CefResourceRequestHandler,
                      public CefResponseFilter
{
public:
 explicit QtCefHandler(bool use_views);
 ~QtCefHandler();

 // Provide access to the single global instance of this object.
 static QtCefHandler* GetInstance();

 virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() OVERRIDE {
   return this;
 }
 virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() OVERRIDE {
   return this;
 }
 virtual CefRefPtr<CefLoadHandler> GetLoadHandler() OVERRIDE { return this; }

 virtual CefRefPtr<CefRequestHandler> GetRequestHandler() OVERRIDE { return this; }

 virtual CefRefPtr<CefResourceRequestHandler> GetResourceRequestHandler(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, bool is_navigation, bool is_download, const CefString &request_initiator, bool &disable_default_handling) OVERRIDE { return this; }

 virtual CefRefPtr<CefResponseFilter> GetResourceResponseFilter(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, CefRefPtr<CefResponse> response) OVERRIDE { return this; }


 // CefDisplayHandler methods:
 virtual void OnTitleChange(CefRefPtr<CefBrowser> browser,
                            const CefString& title) OVERRIDE;

 // CefLifeSpanHandler methods:
 virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;
 virtual bool DoClose(CefRefPtr<CefBrowser> browser) OVERRIDE;
 virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) OVERRIDE;

 // CefLoadHandler methods:
 virtual void OnLoadError(CefRefPtr<CefBrowser> browser,
                          CefRefPtr<CefFrame> frame,
                          ErrorCode errorCode,
                          const CefString& errorText,
                          const CefString& failedUrl) OVERRIDE;

 virtual bool OnResourceResponse(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, CefRefPtr<CefResponse> response) OVERRIDE;

 virtual void OnResourceLoadComplete(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, CefRefPtr<CefResponse> response, URLRequestStatus status, int64 received_content_length) OVERRIDE;

 virtual bool InitFilter() OVERRIDE { return true; }
 virtual FilterStatus Filter(void *data_in, size_t data_in_size, size_t &data_in_read, void *data_out, size_t data_out_size, size_t &data_out_written) OVERRIDE;


 // Request that all existing browser windows close.
 void CloseAllBrowsers(bool force_close);

 bool IsClosing() const { return is_closing_; }

private:
 // True if the application is using the Views framework.
 const bool use_views_;

 // List of existing browser windows. Only accessed on the CEF UI thread.
 typedef std::list<CefRefPtr<CefBrowser>> BrowserList;
 BrowserList browser_list_;

 bool is_closing_;

 // Include the default reference counting implementation.
 IMPLEMENT_REFCOUNTING(QtCefHandler); 
};

class QtCefApp : public QObject, public CefApp, public CefBrowserProcessHandler
{
    Q_OBJECT
public:
    explicit QtCefApp(QObject *parent = nullptr);

    virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override { return this; }
    virtual void OnContextInitialized() override;

    private:
        IMPLEMENT_REFCOUNTING(QtCefApp);

signals:

protected:
        virtual void timerEvent(QTimerEvent *event) override;
};
