#ifndef GOODGAME_H
#define GOODGAME_H

#include "chatservice.hpp"
#include "types.hpp"
#include <QObject>
#include <QWebSocket>
#include <QTimer>
#include <QJsonDocument>
#include <QSettings>

class GoodGame : public ChatService
{
    Q_OBJECT
public:
    explicit GoodGame(QSettings& settings, const QString& SettingsGroupPath, QNetworkAccessManager& network, QObject *parent = nullptr);
    ~GoodGame();

    ConnectionStateType connectionStateType() const override;
    QString stateDescription() const override;
    int viewersCount() const override;
    void reconnect() override;
    void setBroadcastLink(const QString &link) override;
    QString getBroadcastLink() const override;

signals:

private slots:
    void onWebSocketReceived(const QString& rawData);
    void sendToWebSocket(const QJsonDocument& data);
    void timeoutReconnect();

    void requestAuth();
    void requestGetChannelHistory();

private:

    QWebSocket _socket;

    QSettings& settings;
    const QString SettingsGroupPath;
    QNetworkAccessManager& network;

    AxelChat::GoodGameInfo _info;
    QString _lastConnectedChannelName;

    QTimer _timerReconnect;
};

#endif // GOODGAME_H
