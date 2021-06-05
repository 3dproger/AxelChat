#ifndef TWITCH_HPP
#define TWITCH_HPP

#include "types.hpp"
#include "abstractchatservice.hpp"
#include "outputtofile.hpp"
#include <QSettings>
#include <QWebSocket>
#include <QTimer>

class Twitch : public AbstractChatService
{
    Q_OBJECT
    Q_PROPERTY(QUrl     requesGetAOuthTokenUrl      READ requesGetAOuthTokenUrl     CONSTANT)
    Q_PROPERTY(QString  oauthToken                  READ oauthToken                 WRITE setOAuthToken                 NOTIFY stateChanged)
    Q_PROPERTY(QString  userSpecifiedChannel        READ userSpecifiedChannel       WRITE setUserSpecifiedChannel       NOTIFY stateChanged)
    Q_PROPERTY(bool     isChannelNameUserSpecified  READ isChannelNameUserSpecified CONSTANT)

public:
    explicit Twitch(const QNetworkProxy& proxy, OutputToFile* outputToFile, QSettings* settings, const QString& settingsGroupPath, QObject *parent = nullptr);
    ~Twitch();
    ConnectionStateType connectionStateType() const override;
    QString stateDescription() const override;
    QUrl requesGetAOuthTokenUrl() const;
    QUrl chatUrl() const override;
    QUrl controlPanelUrl() const override;
    QUrl broadcastUrl() const override;

    bool isChannelNameUserSpecified() const;
    QString oauthToken() const { return _info.oauthToken; }
    QString userSpecifiedChannel() const { return _info.userSpecifiedChannel; }

    void setProxy(const QNetworkProxy& proxy) override;

signals:

public slots:
    void setUserSpecifiedChannel(QString userChannel);
    void setOAuthToken(QString token);

private slots:
    void onIRCMessage(const QString& rawData);
    void timeoutReconnect();

private:
    void reInitSocket();

    OutputToFile* _outputToFile = nullptr;

    QSettings* _settings = nullptr;
    QString _settingsGroupPath;

    QWebSocket _socket;

    TwitchInfo _info;
    QString _lastConnectedChannelName;

    QTimer _timerReconnect;
};

#endif // TWITCH_HPP
