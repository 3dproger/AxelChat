#ifndef QMLUTILS_HPP
#define QMLUTILS_HPP

#include <QObject>
#include <QSettings>

class QMLUtils : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool enabledHardwareGraphicsAccelerator
               READ enabledHardwareGraphicsAccelerator
               WRITE setEnabledHardwareGraphicsAccelerator
               NOTIFY enabledHardwareGraphicsAcceleratorChanged)
    Q_PROPERTY(QString buildCpuArchitecture
               READ buildCpuArchitecture)
public:
    explicit QMLUtils(QSettings* settings, const QString& settingsGroup, QObject *parent = nullptr);
    static void declareQml();

    Q_INVOKABLE static void restartApplication();
    bool enabledHardwareGraphicsAccelerator() const;
    void setEnabledHardwareGraphicsAccelerator(bool enabled);

    QString buildCpuArchitecture() const;

signals:
    void enabledHardwareGraphicsAcceleratorChanged();

private:
    QSettings* _settings       = nullptr;
    QString _settingsGroupPath = "qml_utils";

    QString _settingsEnabledHardwareGraphicsAccelerator = "enabledHardwareGraphicsAccelerator";
    bool _enabledHardwareGraphicsAccelerator = true;
};

#endif // QMLUTILS_HPP
