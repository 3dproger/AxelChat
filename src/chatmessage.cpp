#include "chatmessage.hpp"
#include <QCoreApplication>
#include <QUuid>
#include <QTranslator>

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

MessageAuthor MessageAuthor::_softwareAuthor;
const MessageAuthor& MessageAuthor::softwareAuthor()
{
    if (!_softwareAuthor._valid)
    {
        _softwareAuthor._channelId = "____" + QCoreApplication::applicationName() + "____";
        _softwareAuthor._name = QCoreApplication::applicationName();
        _softwareAuthor._avatarUrl = QUrl("qrc:/resources/images/axelchat-rounded.svg");

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
        _testMessageAuthor._avatarUrl = QUrl("qrc:/resources/images/flask.svg");

        //_testMessageAuthor._customBadgeUrl = QUrl("qrc:/resources/images/axelchat.svg");
    }

    return _testMessageAuthor;
}



bool MessageAuthor::valid() const
{
    return _valid;
}

ChatMessage ChatMessage::createYouTube(const QString& text,
                                       const QString& id,
                                       const QDateTime& publishedAt,
                                       const QDateTime& receivedAt,
                                       const MessageAuthor& author)
{
    ChatMessage message = ChatMessage();

    message._valid = true;
    message._text        = text;
    message._id          = id;
    message._publishedAt = publishedAt;
    message._receivedAt  = receivedAt;
    message._type        = Type::YouTube;
    message._author      = author;

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

const QHash<int, QByteArray> ChatMessagesModel::_roleNames = QHash<int, QByteArray>{
    {MessageId ,             "messageId"},
    {MessageText ,           "messageText"},
    {MessageType ,           "messageType"},
    {MessagePublishedAt ,    "messagePublishedAt"},
    {MessageReceivedAt ,     "messageReceivedAt"},
    {MessageIsBotCommand ,   "messageIsBotCommand"},

    {AuthorChannelId ,       "authorChannelId"},
    {AuthorPageUrl ,         "authorPageUrl"},
    {AuthorName ,            "authorName"},
    {AuthorAvatarUrl ,       "authorAvatarUrl"},
    {AuthorCustomBadgeUrl ,  "authorCustomBadgeUrl"},
    {AuthorIsVerified ,      "authorIsVerified"},
    {AuthorIsChatOwner ,     "authorIsChatOwner"},
    {AuthorChatSponsor ,     "authorChatSponsor"},
    {AuthorChatModerator ,   "authorChatModerator"}
};


ChatMessagesModel::ChatMessagesModel(QObject *parent) : QAbstractListModel(parent)
{

}

void ChatMessagesModel::append(const QList<ChatMessage>& messages)
{
    const int preSize = _data.size();

    beginInsertRows(QModelIndex(), preSize, preSize + messages.count() - 1);

    for (ChatMessage message : messages)
    {
        QVariant* messageData = new QVariant();
        message._idNum = _lastIdNum;
        _lastIdNum++;

        messageData->setValue(message);

        _idByData.insert(messageData, message.id());
        _dataById.insert(message.id(), messageData);
        _dataByIdNum.insert(message._idNum, messageData);
        _data.append(messageData);  
    }

    endInsertRows();

    if (_data.count() >= _maxSize)
    {
        removeRows(0, _data.count() - _maxSize);
    }

    //qDebug() << "Count:" << _data.count();
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
        _data.removeAt(position);

        delete messageData;
    }

    endRemoveRows();

    return true;
}

uint64_t ChatMessagesModel::lastIdNum() const
{
    return _lastIdNum;
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

    case AuthorChannelId:
        return message.author().channelId();
    case AuthorPageUrl:
        return message.author().pageUrl();
    case AuthorName:
        return message.author().name();
    case AuthorAvatarUrl:
        return message.author().avatarUrl();
    case AuthorCustomBadgeUrl:
        return message.author().customBadgeUrl();
    case AuthorIsVerified:
        return message.author().isVerified();
    case AuthorIsChatOwner:
        return message.author().isChatOwner();
    case AuthorChatSponsor:
        return message.author().isChatSponsor();
    case AuthorChatModerator:
        return message.author().isChatModerator();
    default:
        return QVariant();
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
