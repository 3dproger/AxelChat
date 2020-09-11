#pragma once

#include <QString>
#include <QDateTime>
#include <QUrl>
#include <QAbstractListModel>
#include <QQmlEngine>

class ChatMessage;
class MessageAuthor{
    Q_GADGET
public:
    friend class ChatHandler;

    MessageAuthor() { };

    inline bool valid() const;
    inline QString channelId() const
    {
        return _channelId;
    }
    inline QString name() const
    {
        return _name;
    }
    inline QUrl avatarUrl() const
    {
        return _avatarUrl;
    }
    inline bool isVerified() const
    {
        return _isVerified;
    }
    inline bool isChatOwner() const
    {
        return _isChatOwner;
    }
    inline bool isChatSponsor() const
    {
        return _isChatSponsor;
    }
    inline bool isChatModerator() const
    {
        return _isChatModerator;
    }
    inline QUrl customBadgeUrl() const
    {
        return _customBadgeUrl;
    }

    inline QUrl pageUrl() const
    {
        return _pageUrl;
    }

    inline int messagesSentCurrent() const
    {
        return _messagesSentCurrent;
    }

    static MessageAuthor createFromYouTube(const QString& name,
                                const QString& channelId,
                                const QUrl& avatarUrl,
                                const QUrl badgeUrl,
                                const bool isVerified,
                                const bool isChatOwner,
                                const bool isChatSponsor,
                                const bool isChatModerator);

    static const MessageAuthor& softwareAuthor();
    static const MessageAuthor& testMessageAuthor();

    static void declareQML()
    {
        qmlRegisterUncreatableType<MessageAuthor>("AxelChat.MessageAuthor",
                                     1, 0, "MessageAuthor", "Type cannot be created in QML");
    }

private:
    friend class ChatMessage;

    bool _valid = false;

    QString _channelId;
    QString _name;
    QUrl _pageUrl;
    QUrl _avatarUrl;
    QUrl _customBadgeUrl;
    bool _isVerified      = false;
    bool _isChatOwner     = false;
    bool _isChatSponsor   = false;
    bool _isChatModerator = false;
    bool _isDonation      = false;
    int _messagesSentCurrent = 0;

    static MessageAuthor _softwareAuthor;
    static MessageAuthor _testMessageAuthor;
};

Q_DECLARE_METATYPE(MessageAuthor);

class ChatMessage{
    Q_GADGET
public:
    friend class ChatMessagesModel;
    ChatMessage() { }

    enum Type
    {
        Unknown,
        SoftwareNotification,
        TestMessage,
        YouTube,
        Twitch
    };
    Q_ENUMS(Type)

    static ChatMessage createYouTube(const QString& text,
                              const QString& id,
                              const QDateTime& publishedAt,
                              const QDateTime& receivedAt,
                              const MessageAuthor& author);

    static ChatMessage createSoftwareNotification(const QString& text);
    static ChatMessage createTestMessage         (const QString& text);

    inline QString id() const
    {
        return _id;
    }
    inline QString text() const
    {
        return _text;
    }
    inline bool isBotCommand() const
    {
        return _isBotCommand;
    }
    inline Type type() const
    {
        return _type;
    }
    inline QDateTime publishedAt() const
    {
        return _publishedAt;
    }
    inline QDateTime receivedAt() const
    {
        return _receivedAt;
    }
    inline MessageAuthor author() const
    {
        return _author;
    }
    inline void setIsBotCommand(bool isBotCommand) const
    {
        _isBotCommand = isBotCommand;
    }
    inline uint64_t idNum() const
    {
        return _idNum;
    }

    bool valid() const;

    static void declareQML()
    {
        qmlRegisterUncreatableType<ChatMessage>("AxelChat.ChatMessage",
                                     1, 0, "ChatMessage", "Type cannot be created in QML");
    }

private:
    bool _valid = false;

    uint64_t _idNum = 0;
    QString _id;
    QString _text;

    mutable bool _isBotCommand = false;

    Type _type = Type::Unknown;

    QDateTime _publishedAt;
    QDateTime _receivedAt;

    MessageAuthor _author;
};

Q_DECLARE_METATYPE(ChatMessage)

class ChatMessagesModel : public QAbstractListModel
{
    Q_OBJECT
public:
    ChatMessagesModel(QObject *parent = 0);

    enum ChatMessageRoles {
        MessageId = Qt::UserRole + 1,
        MessageText,
        MessageType,
        MessagePublishedAt,
        MessageReceivedAt,
        MessageIsBotCommand,

        AuthorChannelId,
        AuthorPageUrl,
        AuthorName,
        AuthorAvatarUrl,
        AuthorCustomBadgeUrl,
        AuthorIsVerified,
        AuthorIsChatOwner,
        AuthorChatSponsor,
        AuthorChatModerator
    };

    QHash<int, QByteArray> roleNames() const override {
        return _roleNames;
    }

    void append(const QList<ChatMessage>& messages);
    bool contains(const QString& id);
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    static QVariant dataByRole(const ChatMessage& message, int role);
    QVariant dataByNumId(const uint64_t &idNum, int role);
    bool removeRows(int position, int rows, const QModelIndex &parent = QModelIndex()) override;
    uint64_t lastIdNum() const;

private:
    static const QHash<int, QByteArray> _roleNames;
    QList<QVariant*> _data;//*data
    QHash<QString, QVariant*> _dataById;//message_id, *data
    QHash<QVariant*, QString> _idByData;//*data, message_id
    QHash<uint64_t, QVariant*> _dataByIdNum;//idNum, *data

    const int _maxSize  = 10000;

    uint64_t _lastIdNum = 0;
};

