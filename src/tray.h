#ifndef TRAY_H
#define TRAY_H

#include <QObject>
#include <QSystemTrayIcon>
#include <QQmlEngine>
#include <QMenu>

class Tray : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool ignoredMouse WRITE setIgnoredMouse)

public:
    explicit Tray(QObject *parent = nullptr);
    static void declareQml()
    {
        qmlRegisterUncreatableType<Tray>("AxelChat.Tray", 1, 0, "Tray", "Type cannot be created in QML");
    }

    void setIgnoredMouse(bool ignored);

signals:
    void triggered(const QString& actionName);

private:
    QSystemTrayIcon* tray = nullptr;
    bool _ignoreMouse = false;

    QAction* _ignoreMouseAction = nullptr;
};

#endif // TRAY_H
