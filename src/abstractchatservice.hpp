#ifndef ABSTRACTCHATSERVICE_H
#define ABSTRACTCHATSERVICE_H

#include "types.hpp"
#include <QObject>

class ChatHandler;

class AbstractChatService : public QObject
{
    Q_OBJECT

public:
    enum ConnectionStateType {
        NotConnected = 10,
        Connecting = 20,
        Connected = 30
    };
    Q_ENUM(ConnectionStateType)

    Q_PROPERTY(QUrl                 broadcastUrl                 READ broadcastUrl                    NOTIFY stateChanged)
    Q_PROPERTY(QUrl                 chatUrl                      READ chatUrl                         NOTIFY stateChanged)
    Q_PROPERTY(QUrl                 controlPanelUrl              READ controlPanelUrl                 NOTIFY stateChanged)

    Q_PROPERTY(ConnectionStateType  connectionStateType          READ connectionStateType             NOTIFY stateChanged)
    Q_PROPERTY(QString              stateDescription             READ stateDescription                NOTIFY stateChanged)
    Q_PROPERTY(QString              detailedInformation          READ detailedInformation             NOTIFY detailedInformationChanged)

    Q_PROPERTY(int                  viewersCount                 READ viewersCount                    NOTIFY stateChanged)

    Q_PROPERTY(qint64               traffic                      READ traffic                    NOTIFY stateChanged)

    explicit AbstractChatService(QObject *parent = nullptr)
        : QObject(parent)
    { }

    virtual QUrl chatUrl() const { return QString(); }
    virtual QUrl controlPanelUrl() const { return QString(); }
    virtual QUrl broadcastUrl() const { return QString(); }

    virtual ConnectionStateType connectionStateType() const = 0;
    virtual QString stateDescription() const  = 0;
    virtual QString detailedInformation() const = 0;
    virtual QString getNameLocalized() const = 0;

    virtual int viewersCount() const = 0;

    virtual qint64 traffic() const { return -1; }

    virtual void reconnect() = 0;

#ifdef QT_QUICK_LIB
    static void declareQML()
    {
        //qmlRegisterUncreatableType<ConnectionStateType>("AxelChat.ConnectionStateType", 1, 0, "ConnectionStateType", "Type cannot be created in QML");
        qRegisterMetaType<ConnectionStateType>("AbstractChatService::ConnectionStateType");
        qmlRegisterUncreatableType<MessageAuthor>("AxelChat.AbstractChatService",
                                     1, 0, "AbstractChatService", "Type cannot be created in QML");
    }
#endif

signals:
    void stateChanged();
    void detailedInformationChanged();
    void readyRead(QList<ChatMessage>& messages);
    void connected(QString name);
    void disconnected(QString name);
    void avatarDiscovered(const QString& channelId, const QUrl& url);
    void needSendNotification(const QString& text);

protected:
    void sendNotification(const QString& text)
    {
        emit needSendNotification(text);
    }
};

#endif // ABSTRACTCHATSERVICE_H
