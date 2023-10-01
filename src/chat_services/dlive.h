#pragma once

#include "chatservice.h"
#include "models/message.h"
#include "models/author.h"
#include <QWebSocket>
#include <QTimer>

class DLive : public ChatService
{
    Q_OBJECT
public:
    explicit DLive(QSettings& settings, const QString& settingsGroupPathParent, QNetworkAccessManager& network, cweqt::Manager& web, QObject *parent = nullptr);

    ConnectionStateType getConnectionStateType() const override;
    QString getStateDescription() const override;

protected:
    void reconnectImpl() override;

private slots:
    void send(const QString& type, const QJsonObject& payload = QJsonObject(), const int64_t id = -1);
    void sendStart();

    void onWebSocketReceived(const QString& text);

    void requestChatRoom(const QString& channelName);

    void parseMessages(const QJsonArray& jsonMessages);
    QPair<std::shared_ptr<Message>, std::shared_ptr<Author>> parseMessage(const QJsonObject& json);

private:
    struct Info
    {
        QString userName;
    };

    static QString extractChannelName(const QString& stream);
    static QJsonObject generateQuery(const QString& operationName, const QMap<QString, QJsonValue>& variables, const QString& query = QString());

    QNetworkAccessManager& network;
    QWebSocket socket;

    Info info;
};
