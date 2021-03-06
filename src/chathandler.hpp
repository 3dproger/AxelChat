#ifndef CHATMESSAGEHANDLER_HPP
#define CHATMESSAGEHANDLER_HPP

#include "types.hpp"
#include "youtube.hpp"
#include "outputtofile.hpp"
#include "chatbot.hpp"
#include "twitch.hpp"
#include <QSettings>
#include <QMap>
#include <QDateTime>
#include <QSound>
#include <QNetworkProxy>
#include <memory>

class ChatHandler : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool connectedSome                    READ isConnectedSome                                                            NOTIFY connectedSomeChanged)
    Q_PROPERTY(bool enabledSoundNewMessage           READ enabledSoundNewMessage           WRITE setEnabledSoundNewMessage           NOTIFY enabledSoundNewMessageChanged)
    Q_PROPERTY(bool enabledClearMessagesOnLinkChange READ enabledClearMessagesOnLinkChange WRITE setEnabledClearMessagesOnLinkChange NOTIFY enabledClearMessagesOnLinkChangeChanged)

    Q_PROPERTY(bool    proxyEnabled       READ proxyEnabled       WRITE setProxyEnabled       NOTIFY proxyChanged)
    Q_PROPERTY(QString proxyServerAddress READ proxyServerAddress WRITE setProxyServerAddress NOTIFY proxyChanged)
    Q_PROPERTY(int     proxyServerPort    READ proxyServerPort    WRITE setProxyServerPort    NOTIFY proxyChanged)

public:
    explicit ChatHandler(QSettings* settings, QObject *parent = nullptr);
    ~ChatHandler();
    MessageAuthor authorByChannelId(const QString& channelId) const;

    //bool removeChatMessageAction(int index);

    OutputToFile *outputToFile() const;
    YouTube *youTube() const;
    Twitch* twitch() const;
    ChatBot *bot() const;
    ChatMessagesModel* messagesModel();

    static void declareQml();
    bool isConnectedSome();
    inline bool enabledSoundNewMessage() const { return _enabledSoundNewMessage; }
    void setEnabledSoundNewMessage(bool enabled);

    inline bool enabledClearMessagesOnLinkChange() const { return _enabledClearMessagesOnLinkChange; }
    void setEnabledClearMessagesOnLinkChange(bool enabled);

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
    void messagesReceived(const ChatMessage& message, const MessageAuthor& author);
    void connectedSomeChanged();
    void enabledSoundNewMessageChanged();
    void enabledClearMessagesOnLinkChangeChanged();
    void proxyChanged();

public slots:
    void onReadyRead(const QList<ChatMessage>& messages, const QList<MessageAuthor>& authors);
    void sendTestMessage(const QString& text);
    void playNewMessageSound();

private slots:
    void onConnected(QString name);
    void onDisconnected(QString name);

private:
    void chatNotification(const QString& text);
    void updateProxy();

    ChatMessagesModel _messagesModel;
    QMap<QString, MessageAuthor> _authors;

    QSettings*    _settings                 = nullptr;

    YouTube* _youTube                       = nullptr;
    Twitch* _twitch                         = nullptr;
    OutputToFile* _outputToFile             = nullptr;
    ChatBot* _bot                           = nullptr;

    bool _enabledSoundNewMessage = false;
    bool _enabledClearMessagesOnLinkChange = false;
    std::unique_ptr<QSound> _soundDefaultNewMessage = std::unique_ptr<QSound>(new QSound(":/resources/sound/ui/beep1.wav"));

    bool _enabledProxy = false;
    QNetworkProxy _proxy = QNetworkProxy(QNetworkProxy::ProxyType::Socks5Proxy/*HttpProxy*/);
};

#endif // CHATMESSAGEHANDLER_HPP
