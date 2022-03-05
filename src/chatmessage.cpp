#include "chatmessage.hpp"
#include <QCoreApplication>
#include <QUuid>
#include <QTranslator>

namespace
{

static const QUrl YouTubeDefaultAvatarUrl   = QUrl("qrc:/resources/images/youtube-rounded.svg");
static const QUrl TwitchDefaultAvatarUrl    = QUrl("qrc:/resources/images/twitch-round.svg");

}

MessageAuthor MessageAuthor::createFromYouTube(
        const QString &name,
        const QString &channelId,
        const QUrl &avatarUrl,
        const QUrl badgeUrl,
        const bool isVerified,
        const bool isChatOwner,
        const bool isChatSponsor,
        const bool isChatModerator)
{
    MessageAuthor author;

    author._valid = true;
    author._name = name;
    author._channelId = channelId;
    author._pageUrl = QUrl(QString("https://www.youtube.com/channel/%1").arg(channelId));
    author._avatarUrl = avatarUrl;
    author._customBadgeUrl = badgeUrl;
    author._isVerified = isVerified;
    author._isChatOwner = isChatOwner;
    author._isChatSponsor = isChatSponsor;
    author._isChatModerator = isChatModerator;

    return author;
}

MessageAuthor MessageAuthor::createFromTwitch(const QString &name, const QString &channelId, const QUrl& avatarUrl, const QColor& nicknameColor, const QMap<QString, QString>& badges)
{
    MessageAuthor author;

    author._valid = true;
    author._name = name;
    author._nicknameColor = nicknameColor;
    author._twitchBadges = badges;
    author._channelId = channelId;
    author._pageUrl = QUrl(QString("https://www.twitch.tv/%1").arg(channelId));
    if (avatarUrl.isValid())
    {
        author._avatarUrl = avatarUrl;
    }

    author._isVerified = badges.contains("partner/1");
    author._isChatOwner = badges.contains("broadcaster/1");
    author._isChatModerator = badges.contains("moderator/1");

    /*const QList<QString> badgesNames = badges.keys();
    for (const QString& badgeName : badgesNames)
    {
        if (badgeName.startsWith("subscriber/"))
        {
            author._isChatSponsor = true;
            break;
        }
    }*/

    return author;
}

MessageAuthor MessageAuthor::createFromGoodGame(const QString &name, const QString &userId, const int userGroup)
{
    MessageAuthor author;

    author._valid = true;
    author._name = name;
    //author._nicknameColor = nicknameColor;
    //author._twitchBadgesUrls = badgesUrls;
    author._channelId = userId;
    author._pageUrl = QUrl(QString("https://goodgame.ru/user/%1").arg(userId));
    /*if (avatarUrl.isValid())
    {
        author._avatarUrl = avatarUrl;
    }
    else
    {
        //author._avatarUrl = TwitchDefaultAvatarUrl;
    }*/

    return author;
}

MessageAuthor MessageAuthor::_softwareAuthor;
const MessageAuthor& MessageAuthor::softwareAuthor()
{
    if (!_softwareAuthor._valid)
    {
        _softwareAuthor._channelId = "____" + QCoreApplication::applicationName() + "____";
        _softwareAuthor._name = QCoreApplication::applicationName();
        //_softwareAuthor._avatarUrl = QUrl("qrc:/resources/images/axelchat-rounded.svg");

        //_softwareAuthor._customBadgeUrl = QUrl("qrc:/resources/images/axelchat.svg");
    }

    return _softwareAuthor;
}

MessageAuthor MessageAuthor::_testMessageAuthor;
const MessageAuthor& MessageAuthor::testMessageAuthor()
{
    if (!_testMessageAuthor._valid)
    {
        _testMessageAuthor._channelId = "____TEST_MESSAGE____";
        _testMessageAuthor._name = QTranslator::tr("Test Message");
        //_testMessageAuthor._avatarUrl = QUrl("qrc:/resources/images/flask.svg");

        //_testMessageAuthor._customBadgeUrl = QUrl("qrc:/resources/images/axelchat.svg");
    }

    return _testMessageAuthor;
}



bool MessageAuthor::valid() const
{
    return _valid;
}

ChatMessage ChatMessage::createFromYouTube(const QString& text,
                                       const QString& id,
                                       const QDateTime& publishedAt,
                                       const QDateTime& receivedAt,
                                       const MessageAuthor& author,
                                       const QSet<Flags>& flags,
                                       const QHash<ForcedColorRoles, QColor>& forcedColors)
{
    ChatMessage message = ChatMessage();

    message._valid = true;
    message._text        = text;
    message._id          = id;
    message._publishedAt = publishedAt;
    message._receivedAt  = receivedAt;
    message._type        = Type::YouTube;
    message._author      = author;
    message._flags       = flags;
    message._forcedColors = forcedColors;

    trimCustom(message._text);
    trimCustom(message._author._name);

    return message;
}

ChatMessage ChatMessage::createFromTwitch(const QString &text, const QDateTime &receivedAt, const MessageAuthor &author, const QSet<Flags>& flags)
{
    ChatMessage message = ChatMessage();

    message._valid = true;
    message._text        = text;
    message._id          = QUuid::createUuid().toString(QUuid::Id128);//ToDo: отказать вовсе того, чтобы id был обязателен
    message._publishedAt = receivedAt;//ToDo: возможно, это нужно исправить
    message._receivedAt  = receivedAt;
    message._type        = Type::Twitch;
    message._author      = author;
    message._flags       = flags;

    trimCustom(message._text);
    trimCustom(message._author._name);

    return message;
}

ChatMessage ChatMessage::createFromGoodGame(const QString &text, const QDateTime &timestamp, const MessageAuthor &author)
{
    ChatMessage message = ChatMessage();

    message._valid = true;
    message._text        = text;
    message._id          = QUuid::createUuid().toString(QUuid::Id128);//ToDo: отказать вовсе того, чтобы id был обязателен
    message._publishedAt = timestamp;//ToDo: возможно, это нужно исправить
    message._receivedAt  = timestamp;
    message._type        = Type::GoodGame;
    message._author      = author;

    trimCustom(message._text);
    trimCustom(message._author._name);

    return message;
}

ChatMessage ChatMessage::createDeleterFromYouTube(const QString& text,const QString& id)
{
    ChatMessage message = ChatMessage();

    message._valid         = true;
    message._text          = text;
    message._id            = id;
    message._isDeleterItem = true;

    return message;
}

ChatMessage ChatMessage::createSoftwareNotification(const QString &text)
{
    ChatMessage message = ChatMessage();

    message._valid       = true;
    message._text        = text;
    message._id          = QUuid::createUuid().toString(QUuid::Id128);//ToDo: отказать вовсе того, чтобы id был обязателен
    message._publishedAt = QDateTime::currentDateTime();
    message._receivedAt  = QDateTime::currentDateTime();
    message._type        = Type::SoftwareNotification;
    message._author      = MessageAuthor::softwareAuthor();

    return message;
}

ChatMessage ChatMessage::createTestMessage(const QString &text)
{
    ChatMessage message = ChatMessage();

    message._valid       = true;
    message._text        = text;
    message._id          = QUuid::createUuid().toString(QUuid::Id128);//ToDo: отказать вовсе того, чтобы id был обязателен
    message._publishedAt = QDateTime::currentDateTime();
    message._receivedAt  = QDateTime::currentDateTime();
    message._type        = Type::TestMessage;
    message._author      = MessageAuthor::testMessageAuthor();

    return message;
}

bool ChatMessage::valid() const
{
    return _valid;
}

QString boolToString(const bool& value)
{
    if (value)
    {
        return "true";
    }
    else
    {
        return "false";
    }
}

void ChatMessage::printMessageInfo(const QString &prefix, const int &row) const
{
    QString resultString = prefix;
    if (!resultString.isEmpty())
        resultString += "\n";

    resultString += "===========================";

    resultString += "\nAuthor Name: \"" + author().name() + "\"";
    resultString += "\nMessage Text: \"" + text() + "\"";
    resultString += "\nMessage Id: \"" + id() + "\"";
    resultString += QString("\nMessage Id Num: %1").arg(idNum());

    if (row != -1)
    {
        resultString += QString("\nMessage Row: %1").arg(row);
    }
    else
    {
        resultString += "\nMessage Row: failed to retrieve";
    }

    resultString += "\nMessage Is Valid: " + boolToString(valid());
    resultString += "\nMessage Is Bot Command: " + boolToString(isBotCommand());
    resultString += "\nMessage Is Marked as Deleted: " + boolToString(markedAsDeleted());
    resultString += "\nMessage Is Deleter: " + boolToString(isDeleterItem());
    //ToDo: message._type message._receivedAt message._publishedAt
    //ToDo: other author data

    resultString += "\n===========================";
    qDebug(resultString.toUtf8());
}

QString ChatMessage::forcedColorRoleToQMLString(const ForcedColorRoles &role) const
{
    if (_forcedColors.contains(role) && _forcedColors[role].isValid())
    {
        return _forcedColors[role].name(QColor::HexArgb);
    }

    return QString();
}

void ChatMessage::trimCustom(QString &text)
{
    static const QSet<QChar> trimChars = {
        ' ',
        '\n',
        '\r',
    };

    int left = 0;
    int right = 0;

    for (int i = 0; i < text.length(); ++i)
    {
        const QChar& c = text[i];
        if (trimChars.contains(c))
        {
            left++;
        }
        else
        {
            break;
        }
    }

    for (int i = text.length() - 1; i >= 0; --i)
    {
        const QChar& c = text[i];
        if (trimChars.contains(c))
        {
            right++;
        }
        else
        {
            break;
        }
    }

    text = text.mid(left, text.length() - left - right);
}

const QHash<int, QByteArray> ChatMessagesModel::_roleNames = QHash<int, QByteArray>{
    {MessageId ,              "messageId"},
    {MessageText ,            "messageText"},
    {MessageType ,            "messageType"},
    {MessagePublishedAt ,     "messagePublishedAt"},
    {MessageReceivedAt ,      "messageReceivedAt"},
    {MessageIsBotCommand ,    "messageIsBotCommand"},
    {MessageMarkedAsDeleted , "messageMarkedAsDeleted"},

    {MessageIsDonateSimple,   "messageIsDonateSimple"},
    {MessageIsDonateWithText, "messageIsDonateWithText"},
    {MessageIsDonateWithImage,"messageIsDonateWithImage"},

    {MessageIsPlatformGeneric,          "messageIsPlatformGeneric"},
    {MessageIsYouTubeChatMembership,    "messageIsYouTubeChatMembership"},
    {MessageIsTwitchAction,             "messageIsTwitchAction"},

    {MessageBodyBackgroundForcedColor, "messageBodyBackgroundForcedColor"},

    {AuthorChannelId ,        "authorChannelId"},
    {AuthorPageUrl ,          "authorPageUrl"},
    {AuthorName ,             "authorName"},
    {AuthorNicknameColor ,    "authorNicknameColor"},
    {AuthorAvatarUrl ,        "authorAvatarUrl"},
    {AuthorCustomBadgeUrl ,   "authorCustomBadgeUrl"},
    {AuthorTwitchBadgesUrls,  "authorTwitchBadgesUrls"},
    {AuthorIsVerified ,       "authorIsVerified"},
    {AuthorIsChatOwner ,      "authorIsChatOwner"},
    {AuthorChatSponsor ,      "authorChatSponsor"},
    {AuthorChatModerator ,    "authorChatModerator"}
};


ChatMessagesModel::ChatMessagesModel(QObject *parent) : QAbstractListModel(parent)
{

}

void ChatMessagesModel::append(ChatMessage&& message)
{
    //ToDo: добавить сортировку сообщений по времени

    if (!message.valid())
    {
        message.printMessageInfo(QString("%1: Ignore not valid message:")
                                  .arg(Q_FUNC_INFO));
        return;
    }


    if (message.id().isEmpty())
    {
        //message.printMessageInfo(QString("%1: Ignore message with empty id:").arg(Q_FUNC_INFO));
        return;
    }

    if (!message.isDeleterItem())
    {
        if (!_dataById.contains(message.id()))
        {
            //Normal message

            beginInsertRows(QModelIndex(), _data.count(), _data.count());

            message._idNum = _lastIdNum;
            _lastIdNum++;

            QVariant* messageData = new QVariant();

            messageData->setValue(message);

            _idByData.insert(messageData, message.id());
            _dataById.insert(message.id(), messageData);
            _dataByIdNum.insert(message._idNum, messageData);
            _idNumByData.insert(messageData, message._idNum);

            if (!message._author.avatarUrl().isValid())
            {
                const QString channelId = message.author().channelId();
                if (!_needUpdateAvatarMessages.contains(channelId))
                {
                    _needUpdateAvatarMessages.insert(channelId, QSet<uint64_t>());

                }

                _needUpdateAvatarMessages[channelId].insert(message._idNum);
            }

            _data.append(messageData);

            //printMessageInfo("New message:", rawMessage);

            endInsertRows();
        }
        else
        {
            qDebug(QString("%1: ignore message because this id already exists")
                   .arg(Q_FUNC_INFO).toUtf8());

            const QVariant* data = _dataById.value(message.id());
            if (data)
            {
                const ChatMessage& oldMessage = qvariant_cast<ChatMessage>(*data);

                message.printMessageInfo("Raw new message:");
                oldMessage.printMessageInfo("Old message:");
            }
            else
            {
                message.printMessageInfo("Raw new message:");
                qDebug("Old message: nullptr");
            }
        }
    }
    else
    {
        //Deleter

        //ToDo: Если пришёл делетер, а сообщение ещё нет, то когда это сообщение придёт не будет удалено

        QVariant* data = _dataById[message.id()];
        if (_dataById.contains(message.id()) && data)
        {
            const QModelIndex& index = createIndexByPtr(data);
            if (index.isValid())
            {
                if (!setData(index, true, ChatMessageRoles::MessageMarkedAsDeleted))
                {
                    qDebug(QString("%1: failed to set data with role ChatMessageRoles::MessageMarkedAsDeleted")
                           .arg(Q_FUNC_INFO).toUtf8());

                    message.printMessageInfo("Raw message:");
                }

                if (!setData(index, message.text(), ChatMessageRoles::MessageText))
                {
                    qDebug(QString("%1: failed to set data with role ChatMessageRoles::MessageText")
                           .arg(Q_FUNC_INFO).toUtf8());

                    message.printMessageInfo("Raw message:");
                }

                //qDebug(QString("Message \"%1\" marked as deleted").arg(rawMessage.id()).toUtf8());
            }
            else
            {
                qDebug(QString("%1: index not valid")
                       .arg(Q_FUNC_INFO).toUtf8());

                message.printMessageInfo("Raw message:");
            }
        }
    }

    //qDebug() << "Count:" << _data.count();

    //Remove old messages
    if (_data.count() >= _maxSize)
    {
        removeRows(0, _data.count() - _maxSize);
    }
}

bool ChatMessagesModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    Q_UNUSED(parent)

    beginRemoveRows(QModelIndex(), position, position + rows - 1);

    for (int row = 0; row < rows; ++row)
    {
        QVariant* messageData = _data[position];

        const ChatMessage& message = qvariant_cast<ChatMessage>(*messageData);

        const QString& id = message.id();
        const uint64_t& idNum = message.idNum();

        _dataById.remove(id);
        _idByData.remove(messageData);
        _dataByIdNum.remove(idNum);
        _idNumByData.remove(messageData);
        _data.removeAt(position);

        delete messageData;
        _removedRows++;
    }

    endRemoveRows();

    return true;
}

void ChatMessagesModel::clear()
{
    removeRows(0, rowCount(QModelIndex()));
}

uint64_t ChatMessagesModel::lastIdNum() const
{
    return _lastIdNum;
}

QModelIndex ChatMessagesModel::createIndexByPtr(QVariant *data) const
{
    if (_idNumByData.contains(data) && data != nullptr)
    {
        return createIndex(_idNumByData.value(data) - _removedRows, 0);
    }
    else
    {
        return QModelIndex();
    }
}

int ChatMessagesModel::getRow(QVariant *data)
{
    if (data)
    {
        const QModelIndex& index = createIndexByPtr(data);
        if (index.isValid())
        {
            return index.row();
        }
        else
        {
            return -1;
        }
    }
    else
    {
        return -1;
    }
}

void ChatMessagesModel::applyAvatar(const QString &channelId, const QUrl &url)
{
    for (const uint64_t& oldIdNum : qAsConst(_needUpdateAvatarMessages[channelId]))
    {
        if (_dataByIdNum.contains(oldIdNum))
        {
            QVariant* data = _dataByIdNum[oldIdNum];
            if (data)
            {
                const QModelIndex& index = createIndexByPtr(data);
                setData(index, url, ChatMessagesModel::AuthorAvatarUrl);
            }
        }
    }

    _needUpdateAvatarMessages.remove(channelId);
}

std::vector<uint64_t> ChatMessagesModel::searchByMessageText(const QString& sample, Qt::CaseSensitivity caseSensitivity)
{
    std::vector<uint64_t> result;
    result.reserve(std::min(_dataByIdNum.size(), 1000));

    for (auto it = _dataByIdNum.begin(); it != _dataByIdNum.end(); ++it)
    {
        QVariant*& data = it.value();
        if (!data)
        {
            continue;
        }

        if (((ChatMessage*&)(data))->_text.contains(sample, caseSensitivity))
        {
            result.push_back(it.key());
        }
    }

    return result;
}

bool ChatMessagesModel::contains(const QString &id)
{
    return _dataById.contains(id);
}

int ChatMessagesModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return _data.count();
}

QVariant ChatMessagesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() >= _data.size())
    {
        return QVariant();
    }

    const QVariant* data = _data.value(index.row());
    const ChatMessage& message = qvariant_cast<ChatMessage>(*data);
    return dataByRole(message, role);
}

bool ChatMessagesModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.row() >= _data.size())
    {
        return false;
    }

    ChatMessage message = qvariant_cast<ChatMessage>(*_data[index.row()]);

    switch (role) {
    case MessageId:
        return false;
    case MessageText:
        if (value.canConvert(QMetaType::QString))
        {
            message._text = value.toString();
        }
        else
        {
            return false;
        }
        break;
    case MessageType:
        return false;
    case MessagePublishedAt:
        return false;
    case MessageReceivedAt:
        return false;
    case MessageIsBotCommand:
        return false;
    case MessageMarkedAsDeleted:
        if (value.canConvert(QMetaType::Bool))
        {
            message._markedAsDeleted = value.toBool();
        }
        else
        {
            return false;
        }
        break;

    case AuthorChannelId:
        return false;
    case AuthorPageUrl:
        return false;
    case AuthorName:
        return false;
    case AuthorAvatarUrl:
        if (value.canConvert(QMetaType::QUrl))
        {
            message._author._avatarUrl = value.toUrl();
        }
        else
        {
            return false;
        }
        break;
    case AuthorCustomBadgeUrl:
        return false;
    case AuthorIsVerified:
        return false;
    case AuthorIsChatOwner:
        return false;
    case AuthorChatSponsor:
        return false;
    case AuthorChatModerator:
        return false;
    default:
        return false;
    }

    _data[index.row()]->setValue(message);

    emit dataChanged(index, index, {role});
    return true;
}

QVariant ChatMessagesModel::dataByRole(const ChatMessage &message, int role)
{
    switch (role) {
    case MessageId:
        return message.id();
    case MessageText:
        return message.text();
    case MessageType:
        return message.type();
    case MessagePublishedAt:
        return message.publishedAt();
    case MessageReceivedAt:
        return message.receivedAt();
    case MessageIsBotCommand:
        return message.isBotCommand();
    case MessageMarkedAsDeleted:
        return message.markedAsDeleted();

    case MessageIsDonateSimple:
        return message._flags.contains(ChatMessage::Flags::DonateSimple);
    case MessageIsDonateWithText:
        return message._flags.contains(ChatMessage::Flags::DonateWithText);
    case MessageIsDonateWithImage:
        return message._flags.contains(ChatMessage::Flags::DonateWithImage);

    case MessageIsPlatformGeneric:
        return message._flags.contains(ChatMessage::Flags::PlatformGeneric);

    case MessageIsYouTubeChatMembership:
        return message._flags.contains(ChatMessage::Flags::YouTubeChatMembership);

    case MessageIsTwitchAction:
        return message._flags.contains(ChatMessage::Flags::TwitchAction);

    case MessageBodyBackgroundForcedColor:
        return message.forcedColorRoleToQMLString(ChatMessage::ForcedColorRoles::BodyBackgroundForcedColorRole);

    case AuthorChannelId:
        return message.author().channelId();
    case AuthorPageUrl:
        return message.author().pageUrl();
    case AuthorName:
        return message.author().name();
    case AuthorNicknameColor:
        return message.author().nicknameColor();
    case AuthorAvatarUrl:
        return message.author().avatarUrl();
    case AuthorCustomBadgeUrl:
        return message.author().customBadgeUrl();
    case AuthorTwitchBadgesUrls:
        return message.author().twitchBadgesUrls();
    case AuthorIsVerified:
        return message.author().isVerified();
    case AuthorIsChatOwner:
        return message.author().isChatOwner();
    case AuthorChatSponsor:
        return message.author().isChatSponsor();
    case AuthorChatModerator:
        return message.author().isChatModerator();
    }
    return QVariant();
}

QVariant ChatMessagesModel::dataByNumId(const uint64_t &idNum, int role)
{
    if (_dataByIdNum.contains(idNum))
    {
        const QVariant* data = _dataByIdNum.value(idNum);
        const ChatMessage& message = qvariant_cast<ChatMessage>(*data);
        return dataByRole(message, role);
    }

    return QVariant();
}
