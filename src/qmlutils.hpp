#ifndef QMLUTILS_HPP
#define QMLUTILS_HPP

#include <QObject>
#include <QSettings>

class QMLUtils : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool enabledHardwareGraphicsAccelerator READ enabledHardwareGraphicsAccelerator WRITE setEnabledHardwareGraphicsAccelerator NOTIFY valueChanged)
    Q_PROPERTY(QString buildCpuArchitecture READ buildCpuArchitecture CONSTANT)

public:
    explicit QMLUtils(QSettings& settings, const QString& settingsGroup, QObject *parent = nullptr);
    static void declareQml();

    Q_INVOKABLE static void restartApplication();
    bool enabledHardwareGraphicsAccelerator() const;
    void setEnabledHardwareGraphicsAccelerator(bool enabled);

    QString buildCpuArchitecture() const;

    Q_INVOKABLE void setValue(const QString& key, const QVariant& value);

    Q_INVOKABLE bool valueBool(const QString& key, bool defaultValue);
    Q_INVOKABLE qreal valueReal(const QString& key, qreal defaultValue);

signals:
    void valueChanged();

private:
    QSettings& settings;
    const QString SettingsGroupPath = "qml_utils";
};

#endif // QMLUTILS_HPP
