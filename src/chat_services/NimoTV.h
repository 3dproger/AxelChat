#pragma once

#include "chatservice.h"
#include <QWebSocket>

class NimoTV : public ChatService
{
public:
    explicit NimoTV(ChatManager& manager, QSettings& settings, const QString& settingsGroupPathParent, QNetworkAccessManager& network, cweqt::Manager& web, QObject *parent = nullptr);

    ConnectionState getConnectionState() const override;
    QString getMainError() const override;

protected:
    void resetImpl() override;
    void connectImpl() override;

private slots:
    void onWebSocketReceived(const QByteArray& bin);

private:
    QNetworkAccessManager& network;
    QWebSocket socket;
};
