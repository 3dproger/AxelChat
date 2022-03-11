#include "qmlutils.hpp"
#include <QProcess>
#include <QApplication>
#include <QQmlEngine>
#include <QSysInfo>

namespace
{

static const QString SK_EnabledHardwareGraphicsAccelerator = "enabledHardwareGraphicsAccelerator";

}

QMLUtils::QMLUtils(QSettings& settings_, const QString& settingsGroup, QObject *parent)
    : QObject(parent)
    , settings(settings_)
    , SettingsGroupPath(settingsGroup)
{
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
}

void QMLUtils::restartApplication()
{
    QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
    qApp->quit();
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
        emit valueChanged();
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

void QMLUtils::declareQml()
{
    qmlRegisterUncreatableType<QMLUtils> ("AxelChat.QMLUtils",
                                             1, 0, "QMLUtils", "Type cannot be created in QML");
}
