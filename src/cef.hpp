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

class ResponseDataInterceptor : public CefResponseFilter{
    const int BUFFER_SIZE_RESERVED = 32768;
public:
    ResponseDataInterceptor(CefRefPtr<QtCefApp> cefApp) : _cefApp(cefApp)
    {
        ReInit(0);
    }

    virtual bool InitFilter() OVERRIDE
    {
        CEF_REQUIRE_IO_THREAD();
        return true;
    }

    std::shared_ptr<QByteArray> GetInterceptedData() const
    {
        return _interceptedData;
    }

    uint64 GetResponseIdentifier() const
    {
        return _responseIdentifier;
    }

    void ReInit(uint64 responseIdentifier)
    {
        _responseIdentifier = responseIdentifier;
        _interceptedData->clear();
        _interceptedData->reserve(BUFFER_SIZE_RESERVED);
    }

    virtual FilterStatus Filter(void *data_in, size_t data_in_size, size_t &data_in_read, void *data_out, size_t data_out_size, size_t &data_out_written) OVERRIDE;
private:
    CefRefPtr<QtCefApp> _cefApp;
    uint64 _responseIdentifier = 0;
    std::shared_ptr<QByteArray> _interceptedData = std::shared_ptr<QByteArray>(new QByteArray());
    IMPLEMENT_REFCOUNTING(ResponseDataInterceptor);
};

class QtCefHandler: public CefClient,
                    public CefDisplayHandler,
                    public CefLifeSpanHandler,
                    public CefLoadHandler,
                    public CefRequestHandler,
                    public CefResourceRequestHandler {
public:
    explicit QtCefHandler(CefRefPtr<QtCefApp> cefApp);
    ~QtCefHandler();
    static QtCefHandler* GetInstance();
    virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() OVERRIDE { return this; }
    virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() OVERRIDE { return this; }
    virtual CefRefPtr<CefLoadHandler> GetLoadHandler() OVERRIDE { return this; }

    virtual CefRefPtr<CefRequestHandler> GetRequestHandler() OVERRIDE
    {
        CEF_REQUIRE_IO_THREAD();
        return this;
    }

    virtual CefRefPtr<CefResourceRequestHandler> GetResourceRequestHandler(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, bool is_navigation, bool is_download, const CefString &request_initiator, bool &disable_default_handling) OVERRIDE
    {
        CEF_REQUIRE_IO_THREAD();
        Q_UNUSED(browser);
        Q_UNUSED(frame);
        Q_UNUSED(request);
        Q_UNUSED(is_navigation);
        Q_UNUSED(is_download);
        Q_UNUSED(request_initiator);
        Q_UNUSED(disable_default_handling);
        return this;
    }

    virtual CefRefPtr<CefResponseFilter> GetResourceResponseFilter(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, CefRefPtr<CefResponse> response) OVERRIDE
    {
        CEF_REQUIRE_IO_THREAD();
        Q_UNUSED(browser);
        Q_UNUSED(frame);
        Q_UNUSED(request);
        Q_UNUSED(response);

        if (request && QString::fromStdWString(request->GetURL().ToWString()).contains("get_live_chat"))
        {
            _interceptor->ReInit(request->GetIdentifier());
            return _interceptor;
        }

        return nullptr;
    }

    virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;
    virtual bool DoClose(CefRefPtr<CefBrowser> browser) OVERRIDE;
    virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) OVERRIDE;
    virtual void OnLoadError(CefRefPtr<CefBrowser> browser,
                             CefRefPtr<CefFrame> frame,
                             ErrorCode errorCode,
                             const CefString& errorText,
                             const CefString& failedUrl) OVERRIDE;

    virtual void OnResourceLoadComplete(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, CefRefPtr<CefResponse> response, URLRequestStatus status, int64 received_content_length) OVERRIDE;

    void CloseAllBrowsers(bool force_close);
    bool IsClosing() const { return is_closing_; }

private:
    CefRefPtr<QtCefApp> _cefApp;
    // List of existing browser windows. Only accessed on the CEF UI thread.
    typedef std::list<CefRefPtr<CefBrowser>> BrowserList;
    BrowserList browser_list_;
    bool is_closing_;

    CefRefPtr<ResponseDataInterceptor> _interceptor = new ResponseDataInterceptor(_cefApp);

    IMPLEMENT_REFCOUNTING(QtCefHandler);
};

class QtCefApp : public QObject, public CefApp, public CefBrowserProcessHandler
{
    Q_OBJECT
public:
    explicit QtCefApp(QObject *parent = nullptr) : QObject(parent) {}

    virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override { return this; }
    virtual void OnContextInitialized() override;

    void OnDataReceived(std::shared_ptr<QByteArray> data);

    void setUrl(const QString& url);

signals:
    void dataReceived(std::shared_ptr<QByteArray> data);

protected:
    virtual void timerEvent(QTimerEvent *event) override;

private:
    CefRefPtr<CefBrowser> _browser = nullptr;

    IMPLEMENT_REFCOUNTING(QtCefApp);
};
