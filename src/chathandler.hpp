#ifndef CHATMESSAGEHANDLER_HPP
#define CHATMESSAGEHANDLER_HPP

#include <QSettings>
#include <QMap>
#include <QDateTime>
#include "types.hpp"
#include "youtube.hpp"
#include "outputtofile.hpp"
#include "chatbot.hpp"
#include <QSound>
#include "cef.hpp"

class ChatHandler : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool connectedSome READ isConnectedSome NOTIFY connectedSomeChanged)
    Q_PROPERTY(bool enabledSoundNewMessage READ enabledSoundNewMessage WRITE setEnabledSoundNewMessage NOTIFY enabledSoundNewMessageChanged)

public:
    explicit ChatHandler(QSettings* settings, CefRefPtr<QtCefApp> cefApp, const QString& settingsGroup = "chat_handler", QObject *parent = nullptr);
    ~ChatHandler();
    MessageAuthor authorByChannelId(const QString& channelId) const;

    //bool removeChatMessageAction(int index);

    OutputToFile *outputToFile() const;
    YouTube *youTube() const;
    ChatBot *bot() const;
    ChatMessagesModel* messagesModel();

    static void declareQml();
    bool isConnectedSome();
    inline bool enabledSoundNewMessage() const { return _enabledSoundNewMessage; }
    void setEnabledSoundNewMessage(bool enabled);

    Q_INVOKABLE int authorMessagesSentCurrent(const QString& channelId) const;
    Q_INVOKABLE QUrl authorSizedAvatarUrl(const QString& channelId, int height) const;

signals:
    void messagesReceived(const ChatMessage& message, const MessageAuthor& author);
    void connectedSomeChanged();
    void enabledSoundNewMessageChanged();

public slots:
    void onReadyRead(const QList<ChatMessage>& messages, const QList<MessageAuthor>& authors);
    void sendTestMessage(const QString& text);

private slots:
    void onConnectedYouTube(QString broadcastId);
    void onDisconnectedYouTube(QString broadcastId);

private:
    void chatNotification(const QString& text);

    ChatMessagesModel _messagesModel;
    QMap<QString, MessageAuthor> _authors;

    QString _settingsGroupPath;
    QSettings*    _settings                 = nullptr;

    CefRefPtr<QtCefApp> _cefApp             = nullptr;
    YouTube* _youTube = nullptr;
    OutputToFile* _outputToFile             = nullptr;
    ChatBot* _bot                           = nullptr;

    bool _enabledSoundNewMessage = false;
    const QString _settingsEnabledSoundNewMessage = "enabledSoundNewMessage";
    QSound* _soundDefaultNewMessage = new QSound(":/resources/sound/ui/beep1.wav", this);

    bool _connectedSome = false;
};

#endif // CHATMESSAGEHANDLER_HPP
