#ifndef YOUTUBEINTERCEPTOR_HPP
#define YOUTUBEINTERCEPTOR_HPP

#include "outputtofile.hpp"
#include "abstractchatservice.hpp"
#include <QSettings>
#include <QNetworkAccessManager>
#include <QTimer>
#include <memory>

class YouTube : public AbstractChatService
{
    Q_OBJECT
    Q_PROPERTY(QString userSpecifiedLink            READ userSpecifiedLink WRITE setLink    NOTIFY stateChanged)
    Q_PROPERTY(QString broadcastId                  READ broadcastId                        NOTIFY stateChanged)
    Q_PROPERTY(QUrl    broadcastLongUrl             READ broadcastLongUrl                   NOTIFY stateChanged)
    Q_PROPERTY(bool    isBroadcastIdUserSpecified   READ isBroadcastIdUserSpecified         CONSTANT)

public:
    explicit YouTube(const QNetworkProxy& proxy, OutputToFile* outputToFile, QSettings* settings, const QString& settingsGroupPath = "youtube_interceptor", QObject *parent = nullptr);
    ~YouTube();
    int messagesReceived() const;

    QString broadcastId() const;
    QString userSpecifiedLink() const;
    bool isBroadcastIdUserSpecified() const;
    void reconnect();

    ConnectionStateType connectionStateType() const override;
    QString stateDescription() const override;
    QUrl broadcastUrl() const override;
    QUrl broadcastLongUrl() const;
    QUrl chatUrl() const override;
    QUrl controlPanelUrl() const override;
    Q_INVOKABLE static QUrl createResizedAvatarUrl(const QUrl& sourceAvatarUrl, int imageHeight);

    void setProxy(const QNetworkProxy& proxy) override;

public slots:
    void setLink(QString link);

private slots:
    void onTimeoutRequestChat();
    void onReply(QNetworkReply *reply);

private:
    QString extractBroadcastId(const QString& link) const;
    void parseActionsArray(const QJsonArray& array, const QByteArray& data);
    OutputToFile* _outputToFile = nullptr;

    QSettings* _settings = nullptr;
    QString _settingsGroupPath;

    QTimer _timerRequestChat;
    QNetworkAccessManager _manager;

    int _messagesReceived = 0;

    YouTubeInfo _info;

    static void printData(const QString& tag, const QByteArray& data);
};

#endif // YOUTUBEINTERCEPTOR_HPP
