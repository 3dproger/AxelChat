#ifndef ABSTRACTCHATSERVICE_H
#define ABSTRACTCHATSERVICE_H

#include "types.hpp"
#include <QObject>

class AbstractChatService : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool    connected                    READ isConnected                     NOTIFY connectedChanged)
    Q_PROPERTY(QUrl    broadcastUrl                 READ broadcastUrl                    NOTIFY linkChanged)
    Q_PROPERTY(QUrl    chatUrl                      READ chatUrl                         NOTIFY linkChanged)
    Q_PROPERTY(QUrl    controlPanelUrl              READ controlPanelUrl                 NOTIFY linkChanged)

public:
    explicit AbstractChatService(QObject *parent = nullptr)
        : QObject(parent) { }

    virtual bool isConnected() const = 0;
    virtual QUrl chatUrl() const { return QString(); }
    virtual QUrl controlPanelUrl() const { return QString(); }
    virtual QUrl broadcastUrl() const { return QString(); }

signals:
    void linkChanged();
    void stateChanged();
    void connectedChanged();
    void readyRead(const QList<ChatMessage>& messages, const QList<MessageAuthor>& authors);
    void connected(QString name);
    void disconnected(QString name);

};

#endif // ABSTRACTCHATSERVICE_H
