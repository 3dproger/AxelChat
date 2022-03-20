#ifndef TWITCH_HPP
#define TWITCH_HPP

#include "types.hpp"
#include "abstractchatservice.hpp"
#include <QSettings>
#include <QWebSocket>
#include <QTimer>
#include <QNetworkAccessManager>

class Twitch : public AbstractChatService
{
    Q_OBJECT
    Q_PROPERTY(QUrl     requesGetAOuthTokenUrl      READ requesGetAOuthTokenUrl     CONSTANT)
    Q_PROPERTY(QString  oauthToken                  READ oauthToken                 WRITE setOAuthToken                 NOTIFY stateChanged)
    Q_PROPERTY(QString  userSpecifiedChannel        READ userSpecifiedChannel       WRITE setUserSpecifiedChannel       NOTIFY stateChanged)
    Q_PROPERTY(bool     isChannelNameUserSpecified  READ isChannelNameUserSpecified CONSTANT)

public:
    explicit Twitch(const QNetworkProxy& proxy, QSettings& settings, const QString& settingsGroupPath, QObject *parent = nullptr);
    ~Twitch();
    ConnectionStateType connectionStateType() const override;
    QString stateDescription() const override;
    QString detailedInformation() const override;
    int viewersCount() const override;
    QUrl requesGetAOuthTokenUrl() const;
    QUrl chatUrl() const override;
    QUrl controlPanelUrl() const override;
    QUrl broadcastUrl() const override;

    bool isChannelNameUserSpecified() const;
    QString oauthToken() const { return _info.oauthToken; }
    QString userSpecifiedChannel() const { return _info.userSpecifiedChannel; }

    void setProxy(const QNetworkProxy& proxy) override;

    AxelChat::TwitchInfo getInfo() const;

signals:

public slots:
    void setUserSpecifiedChannel(QString userChannel);
    void setOAuthToken(QString token);

private slots:
    void sendIRCMessage(const QString& message);
    void onIRCMessage(const QString& rawData);

    void requestForAvatarsByChannelPage(const QString& channelLogin);
    void onReplyAvatarsByChannelPage();

    void requestForGlobalBadges();
    void requestForChannelBadges(const QString& broadcasterId);
    void onReplyBadges();

    void requestUserInfo(const QString& login);
    void onReplyUserInfo();

    void requestStreamInfo(const QString& login);
    void onReplyStreamInfo();

private:
    void parseBadgesJson(const QByteArray& data);

    struct MessageEmoteInfo
    {
        QString id;
        QList<QPair<int, int>> indexes;

        bool isValid() const
        {
            return !id.isEmpty() && !indexes.isEmpty();
        }
    };

    void reInitSocket();

    QSettings& settings;
    const QString SettingsGroupPath;

    QWebSocket _socket;

    AxelChat::TwitchInfo _info;
    QString _lastConnectedChannelName;

    QTimer _timerReconnect;
    QTimer _timerPing;
    QTimer _timerCheckPong;

    QTimer _timerUpdaetStreamInfo;

    QNetworkAccessManager _manager;

    QHash<QNetworkReply*, QString> repliesForAvatar; // <reply, channel_id>
    QHash<QString, QUrl> avatarsUrls; // <channel_name, avatar_url>
    QHash<QString, QString> _badgesUrls;
};

#endif // TWITCH_HPP
