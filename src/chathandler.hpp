#ifndef CHATMESSAGEHANDLER_HPP
#define CHATMESSAGEHANDLER_HPP

#include "types.hpp"
#include "youtube.hpp"
#include "twitch.hpp"
#include "goodgame.h"
#include <QSettings>
#include <QMap>
#include <QDateTime>
#include <QNetworkProxy>
#include <memory>

#ifdef QT_MULTIMEDIA_LIB
#include <QSound>
#endif

#ifndef AXELCHAT_LIBRARY
#include "outputtofile.hpp"
#include "chatbot.hpp"
#endif

class ChatHandler : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int  connectedCount                   READ connectedCount                                                             NOTIFY connectedCountChanged)
    Q_PROPERTY(int  viewersTotalCount                READ viewersTotalCount                                                          NOTIFY viewersTotalCountChanged)
    Q_PROPERTY(bool enabledSoundNewMessage           READ enabledSoundNewMessage           WRITE setEnabledSoundNewMessage           NOTIFY enabledSoundNewMessageChanged)
    Q_PROPERTY(bool enabledClearMessagesOnLinkChange READ enabledClearMessagesOnLinkChange WRITE setEnabledClearMessagesOnLinkChange NOTIFY enabledClearMessagesOnLinkChangeChanged)

    Q_PROPERTY(bool    proxyEnabled       READ proxyEnabled       WRITE setProxyEnabled       NOTIFY proxyChanged)
    Q_PROPERTY(QString proxyServerAddress READ proxyServerAddress WRITE setProxyServerAddress NOTIFY proxyChanged)
    Q_PROPERTY(int     proxyServerPort    READ proxyServerPort    WRITE setProxyServerPort    NOTIFY proxyChanged)

public:
    explicit ChatHandler(QSettings& settings, QNetworkAccessManager& network, QObject *parent = nullptr);
    MessageAuthor getAuthorByChannelId(const QString& channelId) const;

    YouTube& getYoutube();
    Twitch& getTwitch() const;
    GoodGame& getGoodGame() const;
    ChatMessagesModel& getMessagesModel();

#ifndef AXELCHAT_LIBRARY
    OutputToFile& getOutputToFile() const;
    ChatBot& getBot() const;
#endif

#ifdef QT_QUICK_LIB
    static void declareQml();
#endif

    inline bool enabledSoundNewMessage() const { return _enabledSoundNewMessage; }
    void setEnabledSoundNewMessage(bool enabled);

    inline bool enabledClearMessagesOnLinkChange() const { return _enabledClearMessagesOnLinkChange; }
    void setEnabledClearMessagesOnLinkChange(bool enabled);

    int connectedCount() const;
    int viewersTotalCount() const;

    Q_INVOKABLE int authorMessagesSentCurrent(const QString& channelId) const;
    Q_INVOKABLE QUrl authorSizedAvatarUrl(const QString& channelId, int height) const;

    void setProxyEnabled(bool enabled);
    inline bool proxyEnabled() const { return _enabledProxy; }

    void setProxyServerAddress(QString address);
    inline QString proxyServerAddress() const { return _proxy.hostName(); }
    void setProxyServerPort(int port);
    inline int proxyServerPort() const { return _proxy.port(); }

    QNetworkProxy proxy() const;

signals:
    void connectedCountChanged();
    void viewersTotalCountChanged();
    void enabledSoundNewMessageChanged();
    void enabledClearMessagesOnLinkChangeChanged();
    void proxyChanged();
    void messagesDataChanged();

public slots:
    void onReadyRead(QList<ChatMessage>& messages);
    void sendTestMessage(const QString& text);
    void sendNotification(const QString& text);
    void playNewMessageSound();
    void onAvatarDiscovered(const QString& channelId, const QUrl& url);
    void clearMessages();
    void onStateChanged();

#ifndef AXELCHAT_LIBRARY
    void openProgramFolder();
#endif

private slots:
    void onConnected(QString name);
    void onDisconnected(QString name);

private:
    void updateProxy();
    void addService(AbstractChatService& service);

    ChatMessagesModel messagesModel;
    QMap<QString, MessageAuthor> authors;

    QSettings& settings;
    QNetworkAccessManager& network;

    YouTube* youtube                       = nullptr;
    Twitch* twitch                         = nullptr;
    GoodGame* goodGame                     = nullptr;
    QList<AbstractChatService*> services;

#ifndef AXELCHAT_LIBRARY
    OutputToFile* _outputToFile             = nullptr;
    ChatBot* _bot                           = nullptr;
#endif

    bool _enabledSoundNewMessage = false;
    bool _enabledClearMessagesOnLinkChange = false;

#ifdef QT_MULTIMEDIA_LIB
    std::unique_ptr<QSound> _soundDefaultNewMessage = std::unique_ptr<QSound>(new QSound(":/resources/sound/ui/beep1.wav"));
#endif

    bool _enabledProxy = false;
    QNetworkProxy _proxy = QNetworkProxy(QNetworkProxy::ProxyType::Socks5Proxy/*HttpProxy*/);
};

#endif // CHATMESSAGEHANDLER_HPP
