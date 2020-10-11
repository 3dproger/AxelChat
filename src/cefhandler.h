#pragma once

#include <QObject>
#include "include/cef_app.h"
#include "include/internal/cef_types_wrappers.h"
#include "include/cef_client.h"
#include "include/wrapper/cef_helpers.h"
#include "include/cef_client.h"
#include "include/cef_urlrequest.h"
#include <list>

#include <QDebug>

class QtCefApp;

class QtCefHandler: public CefClient,
                    public CefDisplayHandler,
                    public CefLifeSpanHandler,
                    public CefLoadHandler,
                    public CefRequestHandler,
                    public CefResourceHandler,
                    public CefResourceRequestHandler,
                    public CefResponseFilter
{
public:
 explicit QtCefHandler(CefRefPtr<QtCefApp> cefApp, bool use_views);
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

 virtual CefRefPtr<CefResourceRequestHandler> GetResourceRequestHandler(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, bool is_navigation, bool is_download, const CefString &request_initiator, bool &disable_default_handling) OVERRIDE
 {
     CEF_REQUIRE_IO_THREAD();
     return this;
 }

 virtual CefRefPtr<CefResponseFilter> GetResourceResponseFilter(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, CefRefPtr<CefResponse> response) OVERRIDE { return this; }

 virtual CefRefPtr<CefResourceHandler> GetResourceHandler(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request) OVERRIDE
 {
     //CEF_REQUIRE_IO_THREAD();
     //return this;
     return nullptr;
 }

 virtual void GetResponseHeaders(CefRefPtr<CefResponse> response, int64 &response_length, CefString &redirectUrl) OVERRIDE
 {
     response->SetStatus(200);
     response_length = -1;
 }

 virtual bool ReadResponse(void* data_out, int bytes_to_read, int& bytes_read, CefRefPtr<CefCallback> callback) OVERRIDE;
 //virtual bool Read(void *data_out, int bytes_to_read, int &bytes_read, CefRefPtr<CefResourceReadCallback> callback) OVERRIDE;
 virtual void Cancel() OVERRIDE {}

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

 virtual bool Open(CefRefPtr<CefRequest> request, bool &handle_request, CefRefPtr<CefCallback> callback) OVERRIDE
 {
     qDebug() << request->GetReferrerURL().ToWString();
     handle_request = false;
     if (callback)
        callback->Continue();
     return true;
 }

 virtual bool InitFilter() OVERRIDE { return true; }
 virtual FilterStatus Filter(void *data_in, size_t data_in_size, size_t &data_in_read, void *data_out, size_t data_out_size, size_t &data_out_written) OVERRIDE;

 // Request that all existing browser windows close.
 void CloseAllBrowsers(bool force_close);

 bool IsClosing() const { return is_closing_; }

private:

 // True if the application is using the Views framework.
 const bool use_views_;

 CefRefPtr<QtCefApp> _cefApp;
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

    void OnDataReceived(void *data, size_t data_size);

private:
    IMPLEMENT_REFCOUNTING(QtCefApp);

signals:
    void dataReceived(void *data, size_t data_size);

protected:
        virtual void timerEvent(QTimerEvent *event) override;
};
