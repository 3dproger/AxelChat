#ifndef YOUTUBEINTERCEPTOR_HPP
#define YOUTUBEINTERCEPTOR_HPP

#include "abstractchatservice.hpp"
#include <QSettings>
#include <QNetworkAccessManager>
#include <QTimer>
#include <memory>

class YouTube : public AbstractChatService
{
    Q_OBJECT
    Q_PROPERTY(QString userSpecifiedLink                    READ userSpecifiedLink WRITE setLink    NOTIFY stateChanged)
    Q_PROPERTY(QString broadcastId                          READ broadcastId                        NOTIFY stateChanged)
    Q_PROPERTY(QUrl    broadcastLongUrl                     READ broadcastLongUrl                   NOTIFY stateChanged)
    Q_PROPERTY(bool    isBroadcastIdUserSpecified           READ isBroadcastIdUserSpecified         CONSTANT)
    Q_PROPERTY(bool    isShowMessagesBeforeConnectEnabled   READ isShowMessagesBeforeConnectEnabled WRITE setShowMessagesBeforeConnectEnabled    NOTIFY stateChanged)

public:
    explicit YouTube(const QNetworkProxy& proxy, QSettings& settings, const QString& settingsGroupPath = "youtube_interceptor", QObject *parent = nullptr);
    ~YouTube();
    int messagesReceived() const;

    QString broadcastId() const;
    QString userSpecifiedLink() const;
    bool isBroadcastIdUserSpecified() const;
    void reconnect();

    ConnectionStateType connectionStateType() const override;
    QString stateDescription() const override;
    QString detailedInformation() const override;
    int viewersCount() const override;
    QUrl broadcastUrl() const override;
    QUrl broadcastLongUrl() const;
    QUrl chatUrl() const override;
    QUrl controlPanelUrl() const override;
    Q_INVOKABLE static QUrl createResizedAvatarUrl(const QUrl& sourceAvatarUrl, int imageHeight);

    void setProxy(const QNetworkProxy& proxy) override;

    qint64 traffic() const override { return _traffic; }

    void setNeedRemoveBeforeAtAsCurrent();

    bool isShowMessagesBeforeConnectEnabled() const;
    void setShowMessagesBeforeConnectEnabled(bool enable);

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

    void tryAppedToText(QString& text, const QJsonObject& jsonObject, const QString& varName, bool bold) const;
    QString parseText(const QJsonObject& message) const;

    QColor intToColor(quint64 rawColor) const;

    QSettings& settings;
    const QString SettingsGroupPath;

    QTimer _timerRequestChat;
    QTimer _timerRequestStreamPage;

    QNetworkAccessManager _manager;

    int _messagesReceived = 0;

    int _badChatReplies = 0;
    int _badLivePageReplies = 0;

    AxelChat::YouTubeInfo _info;
    QDateTime _needRemoveBeforeAt;

    static void printData(const QString& tag, const QByteArray& data);

    const int _emojiPixelSize = 24;
    const int _stickerSize = 80;
    const int _badgePixelSize = 64;

    qint64 _traffic = 0;
};

#endif // YOUTUBEINTERCEPTOR_HPP
