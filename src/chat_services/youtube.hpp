#ifndef YOUTUBEINTERCEPTOR_HPP
#define YOUTUBEINTERCEPTOR_HPP

#include "models/chatmessage.h"
#include "chatservice.hpp"
#include <QSettings>
#include <QNetworkAccessManager>
#include <QTimer>
#include <memory>

class YouTube : public ChatService
{
    Q_OBJECT
public:
    explicit YouTube(QSettings& settings, const QString& settingsGroupPath, QNetworkAccessManager& network, QObject *parent = nullptr);
    ~YouTube();
    int messagesReceived() const;

    void setBroadcastLink(const QString &link) override;
    QString getBroadcastLink() const override;

    void reconnect() override;

    ConnectionStateType getConnectionStateType() const override;
    QString getStateDescription() const override;
    int getViewersCount() const override;
    QUrl getBroadcastUrl() const override;
    QUrl getChatUrl() const override;
    QUrl getControlPanelUrl() const override;
    Q_INVOKABLE static QUrl createResizedAvatarUrl(const QUrl& sourceAvatarUrl, int imageHeight);

    AxelChat::YouTubeInfo getInfo() const;

private slots:
    void onTimeoutRequestChat();
    void onReplyChatPage();
    void onReplyChannelLivePage();

    void onTimeoutRequestStreamPage();
    void onReplyStreamPage();

private:
    QString extractBroadcastId(const QString& link) const;
    void parseActionsArray(const QJsonArray& array, const QByteArray& data);
    bool parseViews(const QByteArray& rawData);
    void processBadChatReply();
    void processBadLivePageReply();

    void tryAppedToText(QList<ChatMessage::Content*>& contents, const QJsonObject& jsonObject, const QString& varName, bool bold) const;
    void parseText(const QJsonObject& message, QList<ChatMessage::Content*>& contents) const;

    QColor intToColor(quint64 rawColor) const;

    QSettings& settings;
    const QString SettingsGroupPath;
    QNetworkAccessManager& network;

    QTimer _timerRequestChat;
    QTimer _timerRequestStreamPage;

    int _messagesReceived = 0;

    int _badChatReplies = 0;
    int _badLivePageReplies = 0;

    AxelChat::YouTubeInfo _info;

    static void printData(const QString& tag, const QByteArray& data);

    const int _emojiPixelSize = 24;
    const int _stickerSize = 80;
    const int _badgePixelSize = 64;
};

#endif // YOUTUBEINTERCEPTOR_HPP
