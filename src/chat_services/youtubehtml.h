#pragma once

#include "chatservice.h"
#include <QNetworkAccessManager>
#include <QTimer>
#include <memory>

class YouTubeHtml : public ChatService
{
    Q_OBJECT
public:
    explicit YouTubeHtml(ChatManager& manager, QSettings& settings, const QString& settingsGroupPathParent, QNetworkAccessManager& network, cweqt::Manager& web, QObject *parent = nullptr);
    
    ConnectionState getConnectionState() const override;
    QString getMainError() const override;

protected:
    void reconnectImpl() override;

private slots:
    void onTimeoutRequestChat();
    void onReplyChatPage();

    void onTimeoutRequestStreamPage();
    void onReplyStreamPage();

private:
    void processBadChatReply();
    void processBadLivePageReply();

    QColor intToColor(quint64 rawColor) const;

    QNetworkAccessManager& network;

    QTimer timerRequestChat;
    QTimer timerRequestStreamPage;

    int badChatReplies = 0;
    int badLivePageReplies = 0;
};
