#ifndef YOUTUBEINTERCEPTOR_HPP
#define YOUTUBEINTERCEPTOR_HPP

#include "outputtofile.hpp"
#include "cef.hpp"
#include "abstractchatservice.hpp"
#include <QSettings>

class YouTube : public AbstractChatService
{
    Q_OBJECT
    Q_PROPERTY(QString userSpecifiedLink            READ userSpecifiedLink WRITE setLink    NOTIFY linkChanged)
    Q_PROPERTY(QString broadcastId                  READ broadcastId                        NOTIFY linkChanged)
    Q_PROPERTY(QUrl    broadcastLongUrl             READ broadcastLongUrl                   NOTIFY linkChanged)
    Q_PROPERTY(bool    isBroadcastIdUserSpecified   READ isBroadcastIdUserSpecified         CONSTANT)

public:
    explicit YouTube(OutputToFile* outputToFile, QSettings* settings, CefRefPtr<QtCefApp> cefApp, const QString& settingsGroupPath = "youtube_interceptor", QObject *parent = nullptr);
    ~YouTube();
    int messagesReceived() const;

    bool isBroadcastIdUserSpecified() const;
    void reconnect();
    ConnectionStateType connectionStateType() const override;
    QString broadcastId() const;
    QString userSpecifiedLink() const;
    QUrl broadcastUrl() const override;
    QUrl broadcastLongUrl() const;
    QUrl chatUrl() const override;
    QUrl controlPanelUrl() const override;
    Q_INVOKABLE static QUrl createResizedAvatarUrl(const QUrl& sourceAvatarUrl, int imageHeight);

public slots:
    void setLink(QString link);
    void onDataReceived(std::shared_ptr<QByteArray> data);

private:
    QString extractBroadcastId(const QString& link) const;
    void parseActionsArray(const QJsonArray& array, const QByteArray& data);
    void parseHTML(std::shared_ptr<QByteArray> data);
    OutputToFile* _outputToFile = nullptr;

    QSettings* _settings = nullptr;
    QString _settingsGroupPath;

    int _messagesReceived = 0;

    CefRefPtr<QtCefApp> _cefApp = nullptr;

    YouTubeInfo _info;

    const QString _settingsKeyUserSpecifiedLink = "user_specified_link";

    static void printData(const QString& tag, const QByteArray& data);
};

#endif // YOUTUBEINTERCEPTOR_HPP
