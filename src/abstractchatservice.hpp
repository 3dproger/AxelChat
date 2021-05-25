#ifndef ABSTRACTCHATSERVICE_H
#define ABSTRACTCHATSERVICE_H

#include "types.hpp"
#include <QObject>

class AbstractChatService : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool    connected                  READ isConnected                     NOTIFY connectedChanged)

public:
    explicit AbstractChatService(QObject *parent = nullptr)
        : QObject(parent) { }

    virtual bool isConnected() const = 0;

signals:
    void stateChanged();
    void connectedChanged();
    void readyRead(const QList<ChatMessage>& messages, const QList<MessageAuthor>& authors);
    void connected(QString name);
    void disconnected(QString name);

};

#endif // ABSTRACTCHATSERVICE_H
