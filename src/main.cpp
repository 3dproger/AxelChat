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
    QCoreApplication::setApplicationName   (APP_INFO_PRODUCTNAME_STR);
    QCoreApplication::setOrganizationName  (APP_INFO_COMPANYNAME_STR);
    QCoreApplication::setOrganizationDomain(APP_INFO_COMPANYDOMAIN_STR);
    QCoreApplication::setApplicationVersion(APP_INFO_PRODUCTVERSION_STR);

    QSettings settings;

    QMLUtils::declareQml();
    QMLUtils qmlUtils(settings, "qml_utils");

    QApplication app(argc, argv);

    QSplashScreen* splashScreen = new QSplashScreen(QPixmap(":/icon.ico"));
    splashScreen->show();

    QNetworkAccessManager network;

    app.setWindowIcon(QIcon(":/icon.ico"));

    I18n::declareQml();
    I18n i18n(settings, "i18n", nullptr);
    Q_UNUSED(i18n);

    QString settingsPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);

    QDir dir(settingsPath);
    if (!dir.exists() && !settingsPath.isEmpty())
    {
        if (!dir.mkpath(settingsPath))
        {
            settingsPath = "";
        }
    }

    ChatHandler::declareQml();
    ChatHandler chatHandler(settings, network);

    qRegisterMetaType<size_t>("size_t");
    qRegisterMetaType<std::shared_ptr<QByteArray>>("std::shared_ptr<QByteArray>");

    GitHubApi::declareQml();
    GitHubApi github(settings, "update_checker", network);

    QQmlApplicationEngine engine;

    i18n.setQmlApplicationEngine(&engine);

    engine.rootContext();

    ClipboardQml::declareQml();
    ClipboardQml clipboard;

    CommandsEditor::declareQml();
    CommandsEditor commandsEditor(chatHandler.getBot());

    Tray::declareQml();
    Tray tray(&engine);

    engine.rootContext()->setContextProperty("i18n",               &i18n);
    engine.rootContext()->setContextProperty("chatHandler",        &chatHandler);
    engine.rootContext()->setContextProperty("youTube",            &chatHandler.getYoutube());
    engine.rootContext()->setContextProperty("twitch",             &chatHandler.getTwitch());
    engine.rootContext()->setContextProperty("outputToFile",       &chatHandler.getOutputToFile());
    engine.rootContext()->setContextProperty("chatBot",            &chatHandler.getBot());
    engine.rootContext()->setContextProperty("updateChecker",      &github);
    engine.rootContext()->setContextProperty("clipboard",          &clipboard);
    engine.rootContext()->setContextProperty("qmlUtils",           &qmlUtils);
    engine.rootContext()->setContextProperty("messagesModel",      &chatHandler.getMessagesModel());
    engine.rootContext()->setContextProperty("commandsEditor",     &commandsEditor);
    engine.rootContext()->setContextProperty("tray",               &tray);

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

    return app.exec();
}

#endif
