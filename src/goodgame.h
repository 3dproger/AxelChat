#ifndef GOODGAME_H
#define GOODGAME_H

#include "abstractchatservice.hpp"
#include "types.hpp"
#include <QObject>
#include <QWebSocket>
#include <QTimer>
#include <QJsonDocument>
#include <QSettings>

class GoodGame : public AbstractChatService
{
    Q_OBJECT
public:
    explicit GoodGame(const QNetworkProxy& proxy, QSettings& settings, const QString& SettingsGroupPath, QObject *parent = nullptr);
    ~GoodGame();

    void setProxy(const QNetworkProxy &proxy) override;
    ConnectionStateType connectionStateType() const override;
    QString stateDescription() const override;
    QString detailedInformation() const override;
    int viewersCount() const override;

signals:

private slots:
    void onWebSocketReceived(const QString& rawData);
    void sendToWebSocket(const QJsonDocument& data);
    void timeoutReconnect();

    void requestAuth();
    void requestGetChannelHistory();

private:

    void reInitSocket();

    QWebSocket _socket;

    QSettings& settings;
    const QString SettingsGroupPath;

    AxelChat::GoodGameInfo _info;
    QString _lastConnectedChannelName;

    QTimer _timerReconnect;
};

#endif // GOODGAME_H
