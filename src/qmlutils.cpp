#include "qmlutils.h"
#include "utils.h"
#include "log/loghandler.h"
#include <QProcess>
#include <QApplication>
#include <QQmlEngine>
#include <QSysInfo>
#include <QWindow>
#include <QDesktopServices>

#ifdef Q_OS_WINDOWS
#include <windows.h>
#include <wininet.h>
#endif

namespace
{

static const QString SK_EnabledHardwareGraphicsAccelerator = "enabledHardwareGraphicsAccelerator";
static const QString SK_EnabledHighDpiScaling = "enabledHighDpiScaling";

}

QMLUtils* QMLUtils::_instance = nullptr;

QMLUtils::QMLUtils(QSettings& settings_, const QString& settingsGroup, QObject *parent)
    : QObject(parent)
    , settings(settings_)
    , SettingsGroupPath(settingsGroup)
{
    if (_instance)
    {
        qWarning() << "instance already initialized";
    }

    _instance = this;

    {
        if (enabledHardwareGraphicsAccelerator())
        {
            //AA_UseDesktopOpenGL == OpenGL для ПК (работает плохо)
            //AA_UseOpenGLES == OpenGL для мобильных платформ (работает лучше всего)
            //ToDo: неизвестно, что произойдёт, если не будет поддерживаться аппаратное ускорение устройством
            QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);
        }
        else
        {
            QCoreApplication::setAttribute(Qt::AA_UseSoftwareOpenGL);
        }
    }

    {
        if (enabledHighDpiScaling())
        {
            QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
        }
        else
        {
            QApplication::setAttribute(Qt::AA_DisableHighDpiScaling);
        }
    }
}

void QMLUtils::restartApplication()
{
    QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
    qApp->quit();
}

void QMLUtils::openLogDirectory()
{
    LogHandler::openDirectory();
}

bool QMLUtils::enabledHardwareGraphicsAccelerator() const
{
    return settings.value(SettingsGroupPath + "/" + SK_EnabledHardwareGraphicsAccelerator, true).toBool();
}

void QMLUtils::setEnabledHardwareGraphicsAccelerator(bool enabled)
{
    if (enabled != enabledHardwareGraphicsAccelerator())
    {
        settings.setValue(SettingsGroupPath + "/" + SK_EnabledHardwareGraphicsAccelerator, enabled);
        emit dataChanged();
    }
}

bool QMLUtils::enabledHighDpiScaling() const
{
    return settings.value(SettingsGroupPath + "/" + SK_EnabledHighDpiScaling, false).toBool();
}

void QMLUtils::setEnabledHighDpiScaling(bool enabled)
{
    if (enabled != enabledHighDpiScaling())
    {
        settings.setValue(SettingsGroupPath + "/" + SK_EnabledHighDpiScaling, enabled);
        emit dataChanged();
    }
}

QString QMLUtils::buildCpuArchitecture() const
{
    return QSysInfo::buildCpuArchitecture();
}

void QMLUtils::setValue(const QString &key, const QVariant &value)
{
    return settings.setValue(SettingsGroupPath + "/" + key, value);
}

bool QMLUtils::valueBool(const QString& key, bool defaultValue)
{
    return settings.value(SettingsGroupPath + "/" + key, defaultValue).toBool();
}

qreal QMLUtils::valueReal(const QString &key, qreal defaultValue)
{
    bool ok = false;
    const qreal result = settings.value(SettingsGroupPath + "/" + key, defaultValue).toReal(&ok);
    if (ok)
    {
        return result;
    }

    return defaultValue;
}

void QMLUtils::updateWindowStyle(QWindow* window) const
{
    if (!window)
    {
        return;
    }

    AxelChat::setDarkWindowFrame(window->winId());
}

void QMLUtils::declareQml()
{
    qmlRegisterUncreatableType<QMLUtils> ("AxelChat.QMLUtils",
                                         1, 0, "QMLUtils", "Type cannot be created in QML");
}

QMLUtils *QMLUtils::instance()
{
    if (!_instance)
    {
        qWarning() << "instance not initialized";
    }

    return _instance;
}

void QMLUtils::setQmlApplicationEngine(const QQmlApplicationEngine* qmlEngine_)
{
    qmlEngine = qmlEngine_;
}
