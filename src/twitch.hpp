#ifndef TWITCH_HPP
#define TWITCH_HPP

#include "types.hpp"
#include "abstractchatservice.hpp"
#include <QSettings>
#include <QNetworkAccessManager>
#include <QWebSocket>

class Twitch : public AbstractChatService
{
    Q_OBJECT
public:
    explicit Twitch(QSettings* settings, const QString& settingsGroupPath, QObject *parent = nullptr);
    bool isConnected() const override;

signals:

public slots:
    void setOAuthToken(QString token);
    void setNickOrLink(QString nickOrLink);
    void setChannelToConnect(QString channelNickOrLink);

private slots:
    void onReply(QNetworkReply *reply);
    void onIRCMessage(const QString& rawData);

private:
    void requestAuthorization();
    void reInitSocket();

    QSettings* _settings = nullptr;
    QString _settingsGroupPath;

    QNetworkAccessManager _manager;

    QWebSocket _socket;

    bool _isConnected = false;

    QString _oauthToken = "v6hnsio2s478ovrwi6y66jje8ccpak";
    QString _nick = "axe1_k";
    QString _channelToConnect = "axe1_k";//"saltybet";
};

#endif // TWITCH_HPP
