#if defined(AXELCHAT_LIBRARY)
#include "axelchatlib_api.hpp"
#else
#include <QApplication>
#include "applicationinfo.hpp"
#include <QQmlApplicationEngine>
#include <QSplashScreen>
#include "chathandler.hpp"
#include "githubapi.hpp"
#include "clipboardqml.hpp"
#include "qmlutils.hpp"
#include "i18n.hpp"
#include <QIcon>
#include <QStandardPaths>
#include <QDir>
#include <QQmlContext>
#include "commandseditor.h"
#include "twitch.hpp"
#include "goodgame.h"
#include "tray.h"
#include <QQuickWindow>

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QCoreApplication::setApplicationName   (APP_INFO_PRODUCTNAME_STR);
    QCoreApplication::setOrganizationName  (APP_INFO_COMPANYNAME_STR);
    QCoreApplication::setOrganizationDomain(APP_INFO_COMPANYDOMAIN_STR);
    QCoreApplication::setApplicationVersion(APP_INFO_PRODUCTVERSION_STR);

    QSettings* settings = new QSettings();

    //QML Utils
    QMLUtils::declareQml();
    QMLUtils* qmlUtils = new QMLUtils(settings, "qml_utils");

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
    ChatHandler* chatHandler = new ChatHandler(settings);
    settings->setParent(chatHandler);

    qRegisterMetaType<size_t>("size_t");
    qRegisterMetaType<std::shared_ptr<QByteArray>>("std::shared_ptr<QByteArray>");

    //Update Checker
    GitHubApi::declareQml();
    GitHubApi* github = new GitHubApi(settings, chatHandler->proxy(), "update_checker", chatHandler);

    QQmlApplicationEngine engine;
    qmlUtils->setParent(&engine);
    chatHandler->setParent(&engine);
    i18n->setQmlApplicationEngine(&engine);

    engine.rootContext();

    //Clipboard
    ClipboardQml::declareQml();
    ClipboardQml* clipboard = new ClipboardQml(&engine);

    //Commands Editor Window
    CommandsEditor::declareQml();
    CommandsEditor* commandsEditor = new CommandsEditor(chatHandler->bot());

    //Tray
    Tray::declareQml();
    Tray* tray = new Tray(&engine);

    engine.rootContext()->setContextProperty("i18n",               i18n);
    engine.rootContext()->setContextProperty("chatHandler",        chatHandler);
    engine.rootContext()->setContextProperty("youTube",            chatHandler->youTube());
    engine.rootContext()->setContextProperty("twitch",             chatHandler->twitch());
    engine.rootContext()->setContextProperty("outputToFile",       chatHandler->outputToFile());
    engine.rootContext()->setContextProperty("chatBot",            chatHandler->bot());
    engine.rootContext()->setContextProperty("updateChecker",      github);
    engine.rootContext()->setContextProperty("clipboard",          clipboard);
    engine.rootContext()->setContextProperty("qmlUtils",           qmlUtils);
    engine.rootContext()->setContextProperty("messagesModel",      chatHandler->messagesModel());
    engine.rootContext()->setContextProperty("commandsEditor",     commandsEditor);
    engine.rootContext()->setContextProperty("tray",               tray);

    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    for (QObject* obj : engine.rootObjects())
    {
        QQuickWindow* window = dynamic_cast<QQuickWindow*>(obj);
        if (window)
        {
            //window->handle();
            break;
        }
    }

    splashScreen->close();
    delete splashScreen;
    splashScreen = nullptr;

    return app.exec();
}

#endif
