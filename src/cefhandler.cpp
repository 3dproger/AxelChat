#include "cefhandler.h"

#include <string>

#include "include/cef_app.h"
#include "include/cef_parser.h"
#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_helpers.h"
#include "include/wrapper/cef_closure_task.h"

#include "include/cef_browser.h"
#include "include/views/cef_browser_view_delegate.h"
#include "include/views/cef_view.h"

namespace {

QtCefHandler* g_instance = nullptr;

// Returns a data: URI with the specified contents.
std::string GetDataURI(const std::string& data, const std::string& mime_type) {
  return "data:" + mime_type + ";base64," +
         CefURIEncode(CefBase64Encode(data.data(), data.size()), false)
             .ToString();
}

}  // namespace

QtCefHandler::QtCefHandler(bool use_views)
    : use_views_(use_views), is_closing_(false) {
  DCHECK(!g_instance);
  g_instance = this;
}

QtCefHandler::~QtCefHandler() {
  g_instance = nullptr;
}

// static
QtCefHandler* QtCefHandler::GetInstance() {
    return g_instance;
}

void QtCefHandler::OnTitleChange(CefRefPtr<CefBrowser> browser,
                                  const CefString& title) {
  CEF_REQUIRE_UI_THREAD();

  //qDebug() << title.ToString16();

  if (use_views_) {
    // Set the title of the window using the Views framework.
    CefRefPtr<CefBrowserView> browser_view =
        CefBrowserView::GetForBrowser(browser);
    if (browser_view) {
      CefRefPtr<CefWindow> window = browser_view->GetWindow();
      if (window)
        window->SetTitle(title);
    }
  } else {
    // Set the title of the window using platform APIs.
    //PlatformTitleChange(browser, title);
  }
}

void QtCefHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  //qDebug() << "OnAfterCreated";

  // Add to the list of existing browsers.
  browser_list_.push_back(browser);
}

bool QtCefHandler::DoClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  // Closing the main window requires special handling. See the DoClose()
  // documentation in the CEF header for a detailed destription of this
  // process.
  if (browser_list_.size() == 1) {
    // Set a flag to indicate that the window close should be allowed.
    is_closing_ = true;
  }

  // Allow the close. For windowed browsers this will result in the OS close
  // event being sent.
  return false;
}

void QtCefHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  //qDebug() << "OnBeforeClose";

  // Remove from the list of existing browsers.
  BrowserList::iterator bit = browser_list_.begin();
  for (; bit != browser_list_.end(); ++bit) {
    if ((*bit)->IsSame(browser)) {
      browser_list_.erase(bit);
      break;
    }
  }

  if (browser_list_.empty()) {
    // All browser windows have closed. Quit the application message loop.
    CefQuitMessageLoop();
  }
}

void QtCefHandler::OnLoadError(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                ErrorCode errorCode,
                                const CefString& errorText,
                                const CefString& failedUrl) {
  CEF_REQUIRE_UI_THREAD();

  // Don't display an error for downloaded files.
  if (errorCode == ERR_ABORTED)
    return;

  // Display a load error message using a data: URI.
  std::stringstream ss;
  ss << "<html><body bgcolor=\"white\">"
        "<h2>Failed to load URL "
     << std::string(failedUrl) << " with error " << std::string(errorText)
     << " (" << errorCode << ").</h2></body></html>";

  frame->LoadURL(GetDataURI(ss.str(), "text/html"));

  qDebug() << "ERROR (" << errorCode << "): " << errorText.ToWString();
}

bool QtCefHandler::OnResourceResponse(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, CefRefPtr<CefResponse> response)
{
    //qDebug() << request->GetReferrerURL().ToWString();

    return false;
}

void QtCefHandler::OnResourceLoadComplete(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, CefRefPtr<CefResponse> response, CefResourceRequestHandler::URLRequestStatus status, int64 received_content_length)
{
    //qDebug() << request->GetReferrerURL().ToWString();
    //response->
}

CefResponseFilter::FilterStatus QtCefHandler::Filter(void *data_in, size_t data_in_size, size_t &data_in_read, void *data_out, size_t data_out_size, size_t &data_out_written)
{
    DCHECK((data_in_size == 0U && !data_in) || (data_in_size > 0U && data_in));
    DCHECK_EQ(data_in_read, 0U);
    DCHECK(data_out);
    DCHECK_GT(data_out_size, 0U);
    DCHECK_EQ(data_out_written, 0U);

    // All data will be read.
    data_in_read = data_in_size;

    // Write out the contents unchanged.
    if (data_in_read > data_out_size)
    {
        data_out_written = data_out_size;
    }
    else
    {
        data_out_written = data_in_read;
    }

    if (data_out_written > 0)
    {
        memcpy(data_out, data_in, data_out_written);
    }

    qDebug() << "=========================\n" << QByteArray((const char*)data_out, 400/*data_in_size*/);

    return RESPONSE_FILTER_DONE;
}

void QtCefHandler::CloseAllBrowsers(bool force_close) {
  if (!CefCurrentlyOn(TID_UI)) {
    // Execute on the UI thread.
    CefPostTask(TID_UI, base::Bind(&QtCefHandler::CloseAllBrowsers, this,
                                   force_close));
    return;
  }

  if (browser_list_.empty())
    return;

  BrowserList::const_iterator it = browser_list_.begin();
  for (; it != browser_list_.end(); ++it)
      (*it)->GetHost()->CloseBrowser(force_close);
}

namespace {

// When using the Views framework this object provides the delegate
// implementation for the CefWindow that hosts the Views-based browser.
class SimpleWindowDelegate : public CefWindowDelegate {
 public:
  explicit SimpleWindowDelegate(CefRefPtr<CefBrowserView> browser_view)
      : browser_view_(browser_view) {}

  void OnWindowCreated(CefRefPtr<CefWindow> window) OVERRIDE {
    // Add the browser view and show the window.
    window->AddChildView(browser_view_);
    window->Show();

    // Give keyboard focus to the browser view.
    browser_view_->RequestFocus();
  }

  void OnWindowDestroyed(CefRefPtr<CefWindow> window) OVERRIDE {
    browser_view_ = nullptr;
  }

  bool CanClose(CefRefPtr<CefWindow> window) OVERRIDE {
    // Allow the window to close if the browser says it's OK.
    CefRefPtr<CefBrowser> browser = browser_view_->GetBrowser();
    if (browser)
      return browser->GetHost()->TryCloseBrowser();
    return true;
  }

  CefSize GetPreferredSize(CefRefPtr<CefView> view) OVERRIDE {
    return CefSize(800, 600);
  }

 private:
  CefRefPtr<CefBrowserView> browser_view_;

  IMPLEMENT_REFCOUNTING(SimpleWindowDelegate);
  DISALLOW_COPY_AND_ASSIGN(SimpleWindowDelegate);
};

class SimpleBrowserViewDelegate : public CefBrowserViewDelegate {
 public:
  SimpleBrowserViewDelegate() {}

  bool OnPopupBrowserViewCreated(CefRefPtr<CefBrowserView> browser_view,
                                 CefRefPtr<CefBrowserView> popup_browser_view,
                                 bool is_devtools) OVERRIDE {
    // Create a new top-level Window for the popup. It will show itself after
    // creation.
    CefWindow::CreateTopLevelWindow(
        new SimpleWindowDelegate(popup_browser_view));

    // We created the Window.
    return true;
  }

 private:
  IMPLEMENT_REFCOUNTING(SimpleBrowserViewDelegate);
  DISALLOW_COPY_AND_ASSIGN(SimpleBrowserViewDelegate);
};

}  // namespace

QtCefApp::QtCefApp(QObject *parent) : QObject(parent)
{
    //startTimer(200);
}

void QtCefApp::OnContextInitialized()
{
    CEF_REQUIRE_UI_THREAD();

    CefRefPtr<CefCommandLine> command_line =
        CefCommandLine::GetGlobalCommandLine();

    const bool enable_chrome_runtime =
        command_line->HasSwitch("enable-chrome-runtime");

  #if defined(OS_WIN) || defined(OS_LINUX)
    // Create the browser using the Views framework if "--use-views" is specified
    // via the command-line. Otherwise, create the browser using the native
    // platform framework. The Views framework is currently only supported on
    // Windows and Linux.
    const bool use_views = command_line->HasSwitch("use-views");
  #else
    const bool use_views = false;
  #endif

    // SimpleHandler implements browser-level callbacks.
    CefRefPtr<QtCefHandler> handler(new QtCefHandler(use_views));

    // Specify CEF browser settings here.
    CefBrowserSettings browser_settings;

    std::string url;

    // Check if a "--url=" value was provided via the command-line. If so, use
    // that instead of the default URL.
    url = command_line->GetSwitchValue("url");
    if (url.empty())
    {
        //url = "https://www.youtube.com/watch?v=5qap5aO4i9A";
        url = "https://www.youtube.com/live_chat?v=5qap5aO4i9A";
    }


    if (use_views && !enable_chrome_runtime)
    {
      // Create the BrowserView.
        qDebug() << "Need create window!";
      CefRefPtr<CefBrowserView> browser_view = CefBrowserView::CreateBrowserView(
          handler, url, browser_settings, nullptr, nullptr,
          new SimpleBrowserViewDelegate());

      // Create the Window. It will show itself after creation.
      CefWindow::CreateTopLevelWindow(new SimpleWindowDelegate(browser_view));
    }
    else
    {
      // Information used when creating the native window.
      CefWindowInfo window_info;
      //window_info.width = 300;
      //window_info.height = 300;

  #if defined(OS_WIN)
      // On Windows we need to specify certain flags that will be passed to
      // CreateWindowEx().
      window_info.SetAsPopup(NULL, "cefsimple");
  #endif

      // Create the first browser window.
      CefBrowserHost::CreateBrowser(window_info,
                                          handler,
                                          url,
                                          browser_settings,
                                          nullptr,
                                          nullptr);
    }
}

void QtCefApp::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);
    CefDoMessageLoopWork();
}
