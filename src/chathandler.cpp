#include "chathandler.hpp"
#include <QDebug>

ChatHandler::ChatHandler(QSettings* settings, const QString& settingsGroup, QObject *parent)
    : QObject(parent)
{
    _settingsGroupPath = settingsGroup;
    _settings = settings;

    //Bot
    _bot = new ChatBot(settings, _settingsGroupPath + "/chat_bot");

    //Output to file
    _outputToFile = new OutputToFile(settings, _settingsGroupPath + "/output_to_file");

    connect(this, &ChatHandler::messagesReceived,
            _outputToFile, &OutputToFile::onMessagesReceived);

    //YouTube
    _youTube = new YouTube(_outputToFile, settings, _settingsGroupPath + "/youtube");

    connect(_youTube, SIGNAL(readyRead(const QList<ChatMessage>&, const QList<MessageAuthor>&)),
                     this, SLOT(onReadyRead(const QList<ChatMessage>&, const QList<MessageAuthor>&)));

    connect(_youTube, SIGNAL(connected(QString)),
                     this, SLOT(onConnectedYouTube(QString)));

    connect(_youTube, SIGNAL(disconnected(QString)),
            this, SLOT(onDisconnectedYouTube(QString)));

    if (_settings)
    {
        setEnabledSoundNewMessage(_settings->value(_settingsGroupPath + "/" + _settingsEnabledSoundNewMessage, _enabledSoundNewMessage).toBool());
    }

    /*MessageAuthor a = MessageAuthor::createFromYouTube(
                "123213123123123123123123123",
                "21324f4f",
                QUrl("qrc:/resources/images/axelchat-rounded.svg"),
                QUrl("qrc:/resources/images/axelchat-rounded.svg"),
                true,
                true,
                true,
                true);

    onReadyRead({ChatMessage::createYouTube(
            "123gbdfgbdgbdfgbdfgbdfgbdfgbdfgbdfgb",
            "sadasdsbsgbfdgbdgfbdfgbdadfw",
            QDateTime::currentDateTime(),
            QDateTime::currentDateTime(),
                 a)}, {a});*/
}

ChatHandler::~ChatHandler()
{
    if (_youTube)
    {
        delete _youTube;
        _youTube = nullptr;
    }

    if (_bot)
    {
        delete _bot;
        _bot = nullptr;
    }

    if (_outputToFile)
    {
        delete _outputToFile;
        _outputToFile = nullptr;
    }
}

MessageAuthor ChatHandler::authorByChannelId(const QString &channelId) const
{
    return _authors.value(channelId);
}


//ToDo: использование ссылок в слотах и сигналах может плохо кончиться! Особенно, если соеденены разные потоки
void ChatHandler::onReadyRead(const QList<ChatMessage> &messages, const QList<MessageAuthor> &authors)
{
    if (_enabledSoundNewMessage && !messages.empty())
    {
        _soundDefaultNewMessage->play();
    }

    for (const MessageAuthor& author : authors)
    {
        const QString& channelId = author.channelId();
        int messagesSentCurrent = 0;

        if (_authors.contains(channelId))
        {
            messagesSentCurrent = _authors[channelId]._messagesSentCurrent;
        }

        _authors[channelId] = author;
        _authors[channelId]._messagesSentCurrent = messagesSentCurrent;
    }


    for (ChatMessage message : messages)//ToDo: убрать копирование
    {
        if (!_messagesModel.contains(message.id()) || message.isDeleterItem())
        {
            /*qDebug(QString("%1: %2")
                   .arg(message.authorName).arg(message.text).toUtf8());*/

            const QString& channelId = message.author().channelId();
            if (_authors.contains(channelId))
            {
                _authors[channelId]._messagesSentCurrent++;
            }

            if (_bot && message.author().channelId() != MessageAuthor::softwareAuthor().channelId())
            {
                _bot->processMessage(message);
            }

            _messagesModel.append(std::move(message));

            emit messagesReceived(message, message.author());
        }
        else
        {
            qDebug(QString("%1: ignore message because this id already exists")
                   .arg(Q_FUNC_INFO).toUtf8());

            message.printMessageInfo("Raw new message:");
        }
    }
}

void ChatHandler::sendTestMessage(const QString &text)
{
    const ChatMessage& message = ChatMessage::createTestMessage(text);
    onReadyRead({message}, {message.author()});
}

void ChatHandler::onConnectedYouTube(QString broadcastId)
{
    chatNotification(tr("YouTube connected: %1").arg(broadcastId));
    _connectedSome = true;
    emit connectedSomeChanged();
}

void ChatHandler::onDisconnectedYouTube(QString broadcastId)
{
    chatNotification(tr("YouTube disconnected: %1").arg(broadcastId));
    _connectedSome = false;
    emit connectedSomeChanged();
}

void ChatHandler::chatNotification(const QString &text)
{
    const ChatMessage& message = ChatMessage::createSoftwareNotification(text);
    onReadyRead({message}, {message.author()});
}

ChatBot *ChatHandler::bot() const
{
    return _bot;
}

ChatMessagesModel* ChatHandler::messagesModel()
{
    return &_messagesModel;
}

void ChatHandler::declareQml()
{
    qmlRegisterUncreatableType<ChatHandler> ("AxelChat.ChatHandler",
                                             1, 0, "ChatHandler", "Type cannot be created in QML");

    qmlRegisterUncreatableType<YouTube> ("AxelChat.YouTube",
                                                    1, 0, "YouTube", "Type cannot be created in QML");

    qmlRegisterUncreatableType<OutputToFile> ("AxelChat.OutputToFile",
                                              1, 0, "OutputToFile", "Type cannot be created in QML");

    ChatBot::declareQml();
    MessageAuthor::declareQML();
    ChatMessage::declareQML();
}

bool ChatHandler::isConnectedSome()
{
    return _connectedSome;
}

void ChatHandler::setEnabledSoundNewMessage(bool enabled)
{
    if (_enabledSoundNewMessage != enabled)
    {
        _enabledSoundNewMessage = enabled;

        if (_settings)
        {
            _settings->setValue(_settingsGroupPath + "/" + _settingsEnabledSoundNewMessage, enabled);
        }

        emit enabledSoundNewMessageChanged();
    }
}

int ChatHandler::authorMessagesSentCurrent(const QString &channelId) const
{
    return _authors.value(channelId)._messagesSentCurrent;
}

QUrl ChatHandler::authorSizedAvatarUrl(const QString &channelId, int height) const
{
    return YouTube::createResizedAvatarUrl(_authors.value(channelId).avatarUrl(), height);
}

YouTube *ChatHandler::youTube() const
{
    return _youTube;
}

OutputToFile *ChatHandler::outputToFile() const
{
    return _outputToFile;
}
