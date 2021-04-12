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
    author._type = AuthorType::YouTubeAuthorType;

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
        _softwareAuthor._type = AuthorType::SoftwareAuthorType;

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
        _softwareAuthor._type = AuthorType::TestAuthorType;

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
                                       const MessageAuthor& author)
{
    ChatMessage message = ChatMessage();

    message._valid = true;
    message._text        = text;
    message._id          = id;
    message._publishedAt = publishedAt;
    message._receivedAt  = receivedAt;
    message._author      = author;

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

const QHash<int, QByteArray> ChatMessagesModel::_roleNames = QHash<int, QByteArray>{
    {MessageId ,              "messageId"},
    {MessageText ,            "messageText"},
    {MessageType ,            "messageType"},
    {MessagePublishedAt ,     "messagePublishedAt"},
    {MessageReceivedAt ,      "messageReceivedAt"},
    {MessageIsBotCommand ,    "messageIsBotCommand"},
    {MessageMarkedAsDeleted , "messageMarkedAsDeleted"},

    {AuthorChannelId ,        "authorChannelId"},
    {AuthorPageUrl ,          "authorPageUrl"},
    {AuthorName ,             "authorName"},
    {AuthorAvatarUrl ,        "authorAvatarUrl"},
    {AuthorCustomBadgeUrl ,   "authorCustomBadgeUrl"},
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
        return false;
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
        return message.authorType();
    case MessagePublishedAt:
        return message.publishedAt();
    case MessageReceivedAt:
        return message.receivedAt();
    case MessageIsBotCommand:
        return message.isBotCommand();
    case MessageMarkedAsDeleted:
        return message.markedAsDeleted();

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
