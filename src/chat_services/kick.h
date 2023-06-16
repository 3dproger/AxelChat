#pragma once

#include "chatservice.h"
#include "models/message.h"
#include <QTimer>
#include <QWebSocket>
#include <QJsonValue>

class Kick : public ChatService
{
    Q_OBJECT
public:
    explicit Kick(QSettings& settings, const QString& settingsGroupPath, QNetworkAccessManager& network, QObject *parent = nullptr);

    ConnectionStateType getConnectionStateType() const override;
    QString getStateDescription() const override;

protected:
    void reconnectImpl() override;

private slots:
    void onWebSocketReceived(const QString &rawData);
    void send(const QJsonObject& object);
    void sendSubscribe(const QString& channelId, const QString& chatroomId);
    void requestChannelInfo(const QString& channelName);

private:
    static QString extractChannelName(const QString& stream);

    QNetworkAccessManager& network;
    QWebSocket socket;

    QTimer timerReconnect;
    QTimer timerPing;
    QTimer timerCheckPing;
};
