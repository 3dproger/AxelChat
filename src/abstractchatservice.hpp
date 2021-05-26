#ifndef ABSTRACTCHATSERVICE_H
#define ABSTRACTCHATSERVICE_H

#include "types.hpp"
#include <QObject>

class AbstractChatService : public QObject
{
    Q_OBJECT

public:
    enum ConnectionStateType {
        NotConnected = Qt::UserRole + 1,
        Connecting,
        Connected
    };
    Q_ENUM(ConnectionStateType)

    Q_PROPERTY(QUrl                 broadcastUrl                 READ broadcastUrl                    NOTIFY linkChanged)
    Q_PROPERTY(QUrl                 chatUrl                      READ chatUrl                         NOTIFY linkChanged)
    Q_PROPERTY(QUrl                 controlPanelUrl              READ controlPanelUrl                 NOTIFY linkChanged)

    Q_PROPERTY(ConnectionStateType  connectionStateType          READ connectionStateType             NOTIFY stateChanged)
    Q_PROPERTY(QString              stateDescription             READ stateDescription                NOTIFY stateChanged)



    explicit AbstractChatService(QObject *parent = nullptr)
        : QObject(parent) { }

    //virtual bool isConnected() const = 0;
    virtual QUrl chatUrl() const { return QString(); }
    virtual QUrl controlPanelUrl() const { return QString(); }
    virtual QUrl broadcastUrl() const { return QString(); }

    virtual ConnectionStateType connectionStateType() const = 0;
    virtual QString stateDescription() const { return QString(); }

    static void declareQML()
    {
        //qmlRegisterUncreatableType<ConnectionStateType>("AxelChat.ConnectionStateType", 1, 0, "ConnectionStateType", "Type cannot be created in QML");
        qRegisterMetaType<ConnectionStateType>("AbstractChatService::ConnectionStateType");
        qmlRegisterUncreatableType<MessageAuthor>("AxelChat.AbstractChatService",
                                     1, 0, "AbstractChatService", "Type cannot be created in QML");
    }

signals:
    void linkChanged();
    void stateChanged();
    void connectedChanged();
    void readyRead(const QList<ChatMessage>& messages, const QList<MessageAuthor>& authors);
    void connected(QString name);
    void disconnected(QString name);

};

#endif // ABSTRACTCHATSERVICE_H
