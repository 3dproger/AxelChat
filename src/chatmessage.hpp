#pragma once

#include <QString>
#include <QDateTime>
#include <QUrl>
#include <QAbstractListModel>
#include <QColor>
#include <QSet>

#ifdef QT_QUICK_LIB
#include <QQmlEngine>
#endif

class ChatMessage;
class MessageAuthor{
    Q_GADGET
public:
    friend class ChatHandler;
    friend class ChatMessagesModel;

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
    inline QStringList twitchBadgesUrls() const
    {
        return _twitchBadges.values();
    }
    inline QUrl pageUrl() const
    {
        return _pageUrl;
    }

    inline int messagesSentCurrent() const
    {
        return _messagesSentCurrent;
    }

    inline QColor nicknameColor() const
    {
        return _nicknameColor;
    }

    static MessageAuthor createFromYouTube(const QString& name,
                                           const QString& channelId,
                                           const QUrl& avatarUrl,
                                           const QUrl badgeUrl,
                                           const bool isVerified,
                                           const bool isChatOwner,
                                           const bool isChatSponsor,
                                           const bool isChatModerator);

    static MessageAuthor createFromTwitch(const QString& name,
                                          const QString& channelId,
                                          const QUrl& avatarUrl,
                                          const QColor& nicknameColor,
                                          const QMap<QString, QString>& badges);

    static MessageAuthor createFromGoodGame(const QString& name,
                                          const QString& userId,
                                          const int userGroup);

    static const MessageAuthor& softwareAuthor();
    static const MessageAuthor& testMessageAuthor();

#ifdef QT_QUICK_LIB
    static void declareQML()
    {
        qmlRegisterUncreatableType<MessageAuthor>("AxelChat.MessageAuthor",
                                     1, 0, "MessageAuthor", "Type cannot be created in QML");
    }
#endif

private:
    friend class ChatMessage;

    bool _valid = false;

    QString _channelId;
    QString _name;
    QColor _nicknameColor;
    QMap<QString, QString> _twitchBadges; // <name, url>
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
    enum Flags {
        DonateSimple,
        DonateWithText,
        DonateWithImage,

        PlatformGeneric,

        YouTubeChatMembership,
        TwitchAction,
    };

    enum ForcedColorRoles {
        BodyBackgroundForcedColorRole
    };

    friend class ChatMessagesModel;
    ChatMessage() { }

    enum Type
    {
        Unknown,
        SoftwareNotification,
        TestMessage,
        YouTube,
        Twitch,
        GoodGame
    };
    Q_ENUM(Type)

    static QString convertServiceId(const ChatMessage::Type type)
    {
        switch (type)
        {
        case ChatMessage::SoftwareNotification: return "softwarenotification";
        case ChatMessage::TestMessage: return "testmessage";
        case ChatMessage::YouTube: return "youtube";
        case ChatMessage::Twitch: return "twitch";
        case ChatMessage::Unknown:
        default:
            return "unknown";
        }

        return "unknown";
    }

    static ChatMessage createFromYouTube(const QString& text,
                              const QString& id,
                              const QDateTime& publishedAt,
                              const QDateTime& receivedAt,
                              const MessageAuthor& author,
                              const QSet<Flags>& flags,
                              const QHash<ForcedColorRoles, QColor>& forcedColors);

    static ChatMessage createFromTwitch(const QString& text,
                              const QDateTime& receivedAt,
                              const MessageAuthor& author,
                              const QSet<Flags>& flags);

    static ChatMessage createFromGoodGame(const QString& text,
                              const QDateTime& timestamp,
                              const MessageAuthor& author);

    static ChatMessage createDeleterFromYouTube(const QString& text,
                                                const QString& id);

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
    inline bool isDeleterItem() const
    {
        return _isDeleterItem;
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
    inline bool markedAsDeleted() const
    {
        return _markedAsDeleted;
    }

    bool valid() const;

#ifdef QT_QUICK_LIB
    static void declareQML()
    {
        qmlRegisterUncreatableType<ChatMessage>("AxelChat.ChatMessage",
                                     1, 0, "ChatMessage", "Type cannot be created in QML");
    }
#endif

    void printMessageInfo(const QString& prefix, const int& row = -1) const;

    QString forcedColorRoleToQMLString(const ForcedColorRoles& role) const;

    static void trimCustom(QString& text);

private:
    bool _valid = false;
    bool _markedAsDeleted = false;

    uint64_t _idNum = 0;
    QString _id;
    QString _text;

    mutable bool _isBotCommand = false;
    bool _isDeleterItem = false;

    Type _type = Type::Unknown;

    QDateTime _publishedAt;
    QDateTime _receivedAt;

    MessageAuthor _author;

    QSet<Flags> _flags;
    QHash<ForcedColorRoles, QColor> _forcedColors;
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
        MessageMarkedAsDeleted,

        MessageIsDonateSimple,
        MessageIsDonateWithText,
        MessageIsDonateWithImage,

        MessageIsPlatformGeneric,

        MessageBodyBackgroundForcedColor,

        MessageIsYouTubeChatMembership,

        MessageIsTwitchAction,

        AuthorChannelId,
        AuthorPageUrl,
        AuthorName,
        AuthorNicknameColor,
        AuthorAvatarUrl,
        AuthorCustomBadgeUrl,
        AuthorTwitchBadgesUrls,
        AuthorIsVerified,
        AuthorIsChatOwner,
        AuthorChatSponsor,
        AuthorChatModerator
    };

    QHash<int, QByteArray> roleNames() const override {
        return _roleNames;
    }

    void append(ChatMessage&& message);
    bool contains(const QString& id);
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    static QVariant dataByRole(const ChatMessage& message, int role);
    QVariant dataByNumId(const uint64_t &idNum, int role);
    bool removeRows(int position, int rows, const QModelIndex &parent = QModelIndex()) override;
    void clear();
    uint64_t lastIdNum() const;
    QModelIndex createIndexByPtr(QVariant* data) const;
    int getRow(QVariant* data);
    void applyAvatar(const QString& channelId, const QUrl& url);

    std::vector<uint64_t> searchByMessageText(const QString& sample, Qt::CaseSensitivity caseSensitivity = Qt::CaseSensitivity::CaseInsensitive);//ToDo: need test

private:
    static const QHash<int, QByteArray> _roleNames;
    QList<QVariant*> _data;//*data
    QHash<QString, QVariant*> _dataById;//message_id, *data
    QHash<QVariant*, QString> _idByData;//*data, message_id
    QHash<uint64_t, QVariant*> _dataByIdNum;//idNum, *data
    QHash<QVariant*, uint64_t> _idNumByData;//*data, idNum
    QHash<QString, QSet<uint64_t>> _needUpdateAvatarMessages;//channel_id, QSet<idNum>

    const int _maxSize  = 10000;

    uint64_t _lastIdNum = 0;
    uint64_t _removedRows = 0;
};

