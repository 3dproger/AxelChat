#pragma once

#include "chatservice.h"
#include <QSettings>
#include <QNetworkAccessManager>
#include <QTimer>

class Telegram : public ChatService
{
    Q_OBJECT
public:
    Telegram(QSettings& settings, const QString& settingsGroupPath, QNetworkAccessManager& network, QObject *parent = nullptr);

    void reconnect() override;
    ConnectionStateType getConnectionStateType() const override;
    QString getStateDescription() const override;

private:
    void requestChat();
    void processBadChatReply();
    void parseUpdates(const QJsonArray& updates);
    void parseMessage(const QJsonObject& jsonMessage, QList<Message>& messages, QList<Author>& authors);
    void requestUserPhoto(const QString& authorId, const int64_t& userId);
    void requestPhotoFileInfo(const QString& authorId, const QString& fileId);

    QSettings& settings;
    QNetworkAccessManager& network;

    struct Info
    {
        int64_t botUserId = 0;
        QString botUserName;
        int badChatReplies = 0;
    };

    Info info;

    QTimer timerRequestChat;

    Setting<bool> allowPrivateChat;

    QSet<int64_t> usersPhotoUpdated;
};
