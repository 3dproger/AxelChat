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
    Q_PROPERTY(QString userSpecifiedLink                    READ userSpecifiedLink WRITE setLink    NOTIFY stateChanged)
    Q_PROPERTY(QString broadcastId                          READ broadcastId                        NOTIFY stateChanged)
    Q_PROPERTY(QUrl    broadcastLongUrl                     READ broadcastLongUrl                   NOTIFY stateChanged)
    Q_PROPERTY(bool    isBroadcastIdUserSpecified           READ isBroadcastIdUserSpecified         CONSTANT)

public:
    explicit YouTube(QSettings& settings, const QString& settingsGroupPath, QNetworkAccessManager& network, QObject *parent = nullptr);
    ~YouTube();
    int messagesReceived() const;

    QString broadcastId() const;
    QString userSpecifiedLink() const;
    bool isBroadcastIdUserSpecified() const;
    void reconnect() override;

    ConnectionStateType connectionStateType() const override;
    QString stateDescription() const override;
    QString detailedInformation() const override;
    int viewersCount() const override;
    QUrl broadcastUrl() const override;
    QUrl broadcastLongUrl() const;
    QUrl chatUrl() const override;
    QUrl controlPanelUrl() const override;
    Q_INVOKABLE static QUrl createResizedAvatarUrl(const QUrl& sourceAvatarUrl, int imageHeight);

    qint64 traffic() const override { return _traffic; }

    AxelChat::YouTubeInfo getInfo() const;

public slots:
    void setLink(QString link);

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

    qint64 _traffic = 0;
};

#endif // YOUTUBEINTERCEPTOR_HPP
