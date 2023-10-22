#pragma once

#include "setting.h"
#include "SponsorshipManager.h"
#include "chat_services/chatservice.h"
#include <QNetworkAccessManager>
#include <QElapsedTimer>

class BackendManager : public QObject
{
    Q_OBJECT
public:
    SponsorshipManager sponsorship;

    explicit BackendManager(QSettings& settings, const QString& settingsGroupPathParent, QNetworkAccessManager& network, QObject *parent = nullptr);

    static BackendManager* getInstance();

public slots:
    void setUsedLanguage(const QString& language);
    void setUsedWebEngineVersion(const QString& version, const QString& cefVersion);
    void addUsedFeature(const QString& feature);
    void setService(const ChatService& service);

    void onBeforeQuit();

private slots:
    void sendSessionUsage();
    void sendServices();

private:
    QJsonObject getJsonMachine() const;
    QJsonObject getJsonApp() const;

    QNetworkAccessManager& network;
    Setting<QString> instanceHash;

    QElapsedTimer usageDuration;
    const QDateTime startTime;

    QSet<QString> usedFeatures;
    QString usedLanguage;
    QString usedWebVersion;
    QString usedCefVersion;

    QMap<QString, QJsonObject> services;

    QTimer timerCanSendUsage;
    QTimer timerSendServices;
};