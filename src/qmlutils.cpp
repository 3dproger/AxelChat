#include "qmlutils.hpp"
#include <QProcess>
#include <QApplication>
#include <QQmlEngine>
#include <QSysInfo>

QMLUtils::QMLUtils(QSettings* settings, const QString& settingsGroup, QObject *parent) :
    QObject(parent), _settings(settings), _settingsGroupPath(settingsGroup)
{
    if (_settings)
    {
        _enabledHardwareGraphicsAccelerator = settings->value(
                    _settingsGroupPath + "/" + _settingsEnabledHardwareGraphicsAccelerator,
                    _enabledHardwareGraphicsAccelerator).toBool();
    }

    if (_enabledHardwareGraphicsAccelerator)
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

void QMLUtils::restartApplication()
{
    QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
    qApp->quit();
}

bool QMLUtils::enabledHardwareGraphicsAccelerator() const
{
    return _enabledHardwareGraphicsAccelerator;
}

void QMLUtils::setEnabledHardwareGraphicsAccelerator(bool enabled)
{
    if (enabled != _enabledHardwareGraphicsAccelerator)
    {
        _enabledHardwareGraphicsAccelerator = enabled;

        if (_settings)
        {
            _settings->setValue(_settingsGroupPath + "/" + _settingsEnabledHardwareGraphicsAccelerator, enabled);
        }

        emit enabledHardwareGraphicsAcceleratorChanged();
    }
}

QString QMLUtils::buildCpuArchitecture() const
{
    return QSysInfo::buildCpuArchitecture();
}

void QMLUtils::setValue(const QString &key, const QVariant &value)
{
    if (!_settings)
    {
        qWarning() << "!_settings";
        return;
    }

    return _settings->setValue(_settingsGroupPath + "/" + key, value);
}

bool QMLUtils::valueBool(const QString& key, bool defaultValue)
{
    if (!_settings)
    {
        qWarning() << "!_settings";
        return defaultValue;
    }

    return _settings->value(_settingsGroupPath + "/" + key, defaultValue).toBool();
}

qreal QMLUtils::valueReal(const QString &key, qreal defaultValue)
{
    if (!_settings)
    {
        qWarning() << "!_settings";
        return defaultValue;
    }

    bool ok = false;
    const qreal result = _settings->value(_settingsGroupPath + "/" + key, defaultValue).toReal(&ok);
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
