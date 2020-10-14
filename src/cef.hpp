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
                    public CefResourceRequestHandler,
                    public CefResponseFilter {
public:
    explicit QtCefHandler(CefRefPtr<QtCefApp> cefApp, bool useViews);
    ~QtCefHandler();
    static QtCefHandler* GetInstance();
    virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() OVERRIDE { return this; }
    virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() OVERRIDE { return this; }
    virtual CefRefPtr<CefLoadHandler> GetLoadHandler() OVERRIDE { return this; }
    virtual CefRefPtr<CefRequestHandler> GetRequestHandler() OVERRIDE { return this; }
    virtual CefRefPtr<CefResourceRequestHandler> GetResourceRequestHandler(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, bool is_navigation, bool is_download, const CefString &request_initiator, bool &disable_default_handling) OVERRIDE
    {
        Q_UNUSED(browser);
        Q_UNUSED(frame);
        Q_UNUSED(request);
        Q_UNUSED(is_navigation);
        Q_UNUSED(is_download);
        Q_UNUSED(request_initiator);
        Q_UNUSED(disable_default_handling);
        CEF_REQUIRE_IO_THREAD();
        return this;
    }

    virtual CefRefPtr<CefResponseFilter> GetResourceResponseFilter(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, CefRefPtr<CefResponse> response) OVERRIDE
    {
        Q_UNUSED(browser);
        Q_UNUSED(frame);
        Q_UNUSED(request);
        Q_UNUSED(response);
        return this;
    }

    virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;
    virtual bool DoClose(CefRefPtr<CefBrowser> browser) OVERRIDE;
    virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) OVERRIDE;
    virtual void OnLoadError(CefRefPtr<CefBrowser> browser,
                             CefRefPtr<CefFrame> frame,
                             ErrorCode errorCode,
                             const CefString& errorText,
                             const CefString& failedUrl) OVERRIDE;

    virtual ReturnValue OnBeforeResourceLoad(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, CefRefPtr<CefRequestCallback> callback) OVERRIDE;
    virtual void OnResourceLoadComplete(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, CefRefPtr<CefResponse> response, URLRequestStatus status, int64 received_content_length) OVERRIDE;

    virtual bool InitFilter() OVERRIDE
    {
        return true;
    }

    virtual FilterStatus Filter(void *data_in, size_t data_in_size, size_t &data_in_read, void *data_out, size_t data_out_size, size_t &data_out_written) OVERRIDE;

    void CloseAllBrowsers(bool force_close);
    bool IsClosing() const { return is_closing_; }

private:
    const bool _useViews = true;

    CefRefPtr<QtCefApp> _cefApp;
    // List of existing browser windows. Only accessed on the CEF UI thread.
    typedef std::list<CefRefPtr<CefBrowser>> BrowserList;
    BrowserList browser_list_;

    QByteArray _buffer;
    bool _enableBuffer = false;

    bool is_closing_;

    IMPLEMENT_REFCOUNTING(QtCefHandler);
};

class QtCefApp : public QObject, public CefApp, public CefBrowserProcessHandler
{
    Q_OBJECT
public:
    explicit QtCefApp(QObject *parent = nullptr) : QObject(parent) {}

    virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override { return this; }
    virtual void OnContextInitialized() override;

    void OnDataReceived(const QByteArray& data);

signals:
    void dataReceived(const QByteArray& data);

protected:
    virtual void timerEvent(QTimerEvent *event) override;

private:
    IMPLEMENT_REFCOUNTING(QtCefApp);
};
