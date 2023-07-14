#include "Manager.h"
#include <QCoreApplication>
#include <QFileInfo>
#include <QDebug>

namespace
{

static const QStringList AvailableResourceTypes =
{
    "MAIN_FRAME",
    "SUB_FRAME",
    "STYLESHEET",
    "SCRIPT",
    "IMAGE",
    "FONT_RESOURCE",
    "SUB_RESOURCE",
    "MEDIA",
    "WORKER",
    "SHARED_WORKER",
    "PREFETCH",
    "FAVICON",
    "XHR",
    "PING",
    "SERVICE_WORKER",
    "CSP_REPORT",
    "PLUGIN_RESOURCE",
    "NAVIGATION_PRELOAD_MAIN_FRAME",
    "NAVIGATION_PRELOAD_SUB_FRAME",
};

static QString boolToValue(const bool value)
{
    return value ? "true" : "false";
}

static bool getParamStr(const QMap<QString, QString>& parameters, const QString& name, QString& value)
{
    if (!parameters.contains(name))
    {
        qWarning() << Q_FUNC_INFO << "not found parameter" << name;
        return false;
    }

    value = parameters.value(name);
    return true;
}

static bool getParamInt64(const QMap<QString, QString>& parameters, const QString& name, int64_t& value)
{
    if (!parameters.contains(name))
    {
        qWarning() << Q_FUNC_INFO << "not found parameter" << name;
        return false;
    }

    const QString str = parameters.value(name);

    bool ok = false;
    const auto tmp = str.toLongLong(&ok);
    if (!ok)
    {
        qWarning() << Q_FUNC_INFO << "failed to convert parameter" << name << "=" << str << "to integer";
        return false;
    }

    value = tmp;

    return true;
}

static bool getParamInt(const QMap<QString, QString>& parameters, const QString& name, int& value)
{
    if (!parameters.contains(name))
    {
        qWarning() << Q_FUNC_INFO << "not found parameter" << name;
        return false;
    }

    const QString str = parameters.value(name);

    bool ok = false;
    const auto tmp = str.toInt(&ok);
    if (!ok)
    {
        qWarning() << Q_FUNC_INFO << "failed to convert parameter" << name << "=" << str << "to integer";
        return false;
    }

    value = tmp;

    return true;
}

}

namespace cweqt
{

const QStringList &Manager::getAvailableResourceTypes()
{
    return AvailableResourceTypes;
}

bool Manager::isExecutableExists() const
{
    return QFileInfo::exists(executablePath);
}

Manager::Manager(const QString& scrapperExecutablePath, QObject *parent)
    : QObject{parent}
    , executablePath(scrapperExecutablePath)
{
    connect(&messanger, QOverload<const Messanger::Message&>::of(&Messanger::messageReceived), this, [this](const Messanger::Message& message)
    {
        const QString& type = message.getType();
        const QMap<QString, QString>& params = message.getParameters();
        const QByteArray& data = message.getData();

        if (type == "browser-opened")
        {
            QString url;
            if (!getParamStr(params, "url", url))
            {
                return;
            }

            int64_t messageId = 0;
            if (!getParamInt64(params, "message-id", messageId))
            {
                return;
            }

            int browserId = 0;
            if (!getParamInt(params, "browser-id", browserId))
            {
                return;
            }

            std::shared_ptr<Browser> browser(new Browser(*this, browserId, url, true, this));

            browsers[browserId] = browser;

            emit browserOpened(browser);
        }
        else if (type == "initialized")
        {
            _initialized = true;
            emit initialized();
        }
        else if (type == "browser-closed")
        {
            int browserId = 0;
            if (!getParamInt(params, "id", browserId))
            {
                return;
            }

            auto it = browsers.find(browserId);
            if (it == browsers.end())
            {
                qWarning() << Q_FUNC_INFO << "unknown browser id" << browserId;
                return;
            }

            std::shared_ptr<Browser>& browser = it->second;

            if (browser)
            {
                browser->setClosed();
            }
            else
            {
                qWarning() << Q_FUNC_INFO << "browser is null";
            }

            browsers.erase(it);
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "unknown message type" << type;
        }
    });
}

void Manager::openBrowser(const QUrl &url, const Browser::Settings& settings)
{
    if (!isInitialized())
    {
        qWarning() << Q_FUNC_INFO << "not initialized";
        return;
    }

    messanger.send(Messanger::Message(
                       "browser-open",
                        {
                            { "url", url.toString() },
                            { "visible", boolToValue(settings.visible) },
                            { "show-responses", boolToValue(settings.showResponses) },
                        }), process);
}

void Manager::closeBrowser(const int id)
{
    if (!isInitialized())
    {
        qWarning() << Q_FUNC_INFO << "not initialized";
        return;
    }

    messanger.send(Messanger::Message(
                        "browser-close",
                        {
                            { "id", QString("%1").arg(id) },
                        }), process);
}

bool Manager::isInitialized() const
{
    return _initialized;
}

void Manager::startProcess()
{
    stopProcess();

    if (!isExecutableExists())
    {
        qWarning() << Q_FUNC_INFO << "executable" << executablePath << "not exists";
        return;
    }

    process = new QProcess(this);
    
    connect(process, &QProcess::readyRead, this, &Manager::onReadyRead);
    connect(process, &QProcess::stateChanged, this, [this](const QProcess::ProcessState state)
    {
        if (state == QProcess::ProcessState::NotRunning)
        {
            process->deleteLater();
            process = nullptr;
        }
    });

    process->start(executablePath, QStringList());
}

void Manager::stopProcess()
{
    _initialized = false;

    if (process)
    {
        //TODO: send exit

        process->terminate();
        process = nullptr;
    }

    responses.clear();
}

void Manager::onReadyRead()
{
    if (!process)
    {
        return;
    }

    while (process->canReadLine())
    {
        messanger.parseLine(process->readLine());

        if (!process)
        {
            break;
        }
    }
}

}
