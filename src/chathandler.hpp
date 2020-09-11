#ifndef CHATMESSAGEHANDLER_HPP
#define CHATMESSAGEHANDLER_HPP

#include <QSettings>
#include <QMap>
#include <QDateTime>
#include "types.hpp"
#include "youtubeinterceptor.hpp"
#include "outputtofile.hpp"
#include "chatbot.hpp"
#include <QSound>

class ChatHandler : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool connectedSome READ isConnectedSome NOTIFY connectedSomeChanged)
    Q_PROPERTY(bool enabledSoundNewMessage READ enabledSoundNewMessage WRITE setEnabledSoundNewMessage NOTIFY enabledSoundNewMessageChanged)

public:
    explicit ChatHandler(QSettings* settings, const QString& settingsGroup = "chat_handler", QObject *parent = nullptr);
    ~ChatHandler();
    MessageAuthor authorByChannelId(const QString& channelId) const;

    //bool removeChatMessageAction(int index);

    OutputToFile *outputToFile() const;
    YouTubeInterceptor *youTubeInterceptor() const;
    ChatBot *bot() const;
    ChatMessagesModel* messagesModel();

    static void declareQml();
    bool isConnectedSome();
    inline bool enabledSoundNewMessage() const { return _enabledSoundNewMessage; }
    void setEnabledSoundNewMessage(bool enabled);

    Q_INVOKABLE int authorMessagesSentCurrent(const QString& channelId) const;

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

    YouTubeInterceptor* _youTubeInterceptor = nullptr;
    OutputToFile* _outputToFile             = nullptr;
    ChatBot* _bot                           = nullptr;

    bool _enabledSoundNewMessage = false;
    const QString _settingsEnabledSoundNewMessage = "enabledSoundNewMessage";
    QSound* _soundDefaultNewMessage = new QSound(":/resources/sound/ui/beep1.wav", this);

    bool _connectedSome = false;
};

#endif // CHATMESSAGEHANDLER_HPP
