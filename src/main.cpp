#if defined(AXELCHAT_LIBRARY)
#include "shared_library_interface.hpp"
#else
#include <QApplication>
#include "constants.hpp"
#include <QQmlApplicationEngine>
#include <QSplashScreen>
#include "chathandler.hpp"
#include "githubapi.hpp"
#include "clipboardqml.hpp"
#include "qmlutils.hpp"
#include "i18n.hpp"
#include "cefhandler.h"
#include <QIcon>
#include <QStandardPaths>
#include <QDir>
#include <QQmlContext>

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QCoreApplication::setApplicationName   (AxelChat::APPLICATION_NAME);
    QCoreApplication::setOrganizationName  (AxelChat::ORGANIZATION_NAME);
    QCoreApplication::setOrganizationDomain(AxelChat::ORGANIZATION_DOMAIN);
    QCoreApplication::setApplicationVersion(AxelChat::APPLICATION_VERSION);

    QSettings* settings = new QSettings();

    //QML Utils
    QMLUtils::declareQml();
    QMLUtils* qmlUtils = new QMLUtils(settings, "qml_utils");

    //CEF
    // Enable High-DPI support on Windows 7 or newer.
    CefEnableHighDPISupport();

    void* sandbox_info = nullptr;

  #if defined(CEF_USE_SANDBOX)
    // Manage the life span of the sandbox information object. This is necessary
    // for sandbox support on Windows. See cef_sandbox_win.h for complete details.
    CefScopedSandboxInfo scoped_sandbox;
    sandbox_info = scoped_sandbox.sandbox_info();
  #endif

#ifdef Q_OS_WIN
    CefMainArgs main_args((HINSTANCE)GetModuleHandle(0));
#endif

    // Parse command-line arguments for use in this method.
    CefRefPtr<CefCommandLine> command_line = CefCommandLine::CreateCommandLine();
    command_line->InitFromString(::GetCommandLineW());

    // Specify CEF global settings here.
    CefSettings cefSettings;

    if (command_line->HasSwitch("enable-chrome-runtime")) {
      // Enable experimental Chrome runtime. See issue #2969 for details.
      cefSettings.chrome_runtime = true;
    }

  #if !defined(CEF_USE_SANDBOX)
    cefSettings.no_sandbox = true;
  #endif

    // SimpleApp implements application-level callbacks for the browser process.
    // It will create the first browser instance in OnContextInitialized() after
    // CEF has initialized.
    CefRefPtr<QtCefApp> cefApp(new QtCefApp());


    // Initialize CEF.
    CefInitialize(main_args, cefSettings, cefApp.get(), sandbox_info);
    CefDoMessageLoopWork();

    //Qt
    QApplication app(argc, argv);

    QSplashScreen* splashScreen = new QSplashScreen(QPixmap(":/icon.ico"));
    splashScreen->show();

    //Window icon
    app.setWindowIcon(QIcon(":/icon.ico"));

    //Translations
    I18n::declareQml();
    I18n* i18n = new I18n(settings, "i18n", nullptr, &app);
    Q_UNUSED(i18n);

    //Settings
    QString settingsPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);

    QDir dir(settingsPath);
    if (!dir.exists() && !settingsPath.isEmpty())
    {
        if (!dir.mkpath(settingsPath))
        {
            settingsPath = "";
        }
    }

    //ChatHandler
    ChatHandler::declareQml();
    ChatHandler* chatHandler = new ChatHandler(settings, "chat_handler");
    settings->setParent(chatHandler);

    qRegisterMetaType<size_t>("size_t");
    QObject::connect(cefApp, &QtCefApp::dataReceived, chatHandler->youTube(), &YouTube::onDataReceived);

    //Update Checker
    GitHubApi::declareQml();
    GitHubApi* github = new GitHubApi(settings, "update_checker", chatHandler);

    QQmlApplicationEngine engine;
    qmlUtils->setParent(&engine);
    chatHandler->setParent(&engine);
    i18n->setQmlApplicationEngine(&engine);

    //Clipboard
    ClipboardQml::declareQml();
    ClipboardQml* clipboard = new ClipboardQml(&engine);

    engine.rootContext()->setContextProperty("i18n",               i18n);
    engine.rootContext()->setContextProperty("chatHandler",        chatHandler);
    engine.rootContext()->setContextProperty("youTube",            chatHandler->youTube());
    engine.rootContext()->setContextProperty("outputToFile",       chatHandler->outputToFile());
    engine.rootContext()->setContextProperty("chatBot",            chatHandler->bot());
    engine.rootContext()->setContextProperty("updateChecker",      github);
    engine.rootContext()->setContextProperty("clipboard",          clipboard);
    engine.rootContext()->setContextProperty("qmlUtils",           qmlUtils);
    engine.rootContext()->setContextProperty("messagesModel",      chatHandler->messagesModel());

    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    splashScreen->close();
    delete splashScreen;
    splashScreen = nullptr;

    cefApp->moveToThread(engine.thread());
    cefApp->startTimer(200);

    int returnCode = app.exec();

    CefShutdown();

    return returnCode;
}

#endif
