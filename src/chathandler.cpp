#include "chathandler.hpp"
#include <QCoreApplication>
#ifndef AXELCHAT_LIBRARY
#include <QDesktopServices>
#endif
#include <QDebug>

namespace
{

static const QString SettingsGroupPath = "chat_handler";

static const QString SettingsEnabledSoundNewMessage             = SettingsGroupPath + "/enabledSoundNewMessage";
static const QString SettingsEnabledClearMessagesOnLinkChange   = SettingsGroupPath + "/enabledClearMessagesOnLinkChange";
static const QString SettingsProxyEnabled                       = SettingsGroupPath + "/proxyEnabled";
static const QString SettingsProxyAddress                       = SettingsGroupPath + "/proxyServerAddress";
static const QString SettingsProxyPort                          = SettingsGroupPath + "/proxyServerPort";

}

ChatHandler::ChatHandler(QSettings& settings_, QObject *parent)
    : QObject(parent)
    , settings(settings_)
{
    setEnabledSoundNewMessage(settings.value(SettingsEnabledSoundNewMessage, _enabledSoundNewMessage).toBool());

    setEnabledClearMessagesOnLinkChange(settings.value(SettingsEnabledClearMessagesOnLinkChange, _enabledClearMessagesOnLinkChange).toBool());

    setProxyEnabled(settings.value(SettingsProxyEnabled, _enabledProxy).toBool());
    setProxyServerAddress(settings.value(SettingsProxyAddress, _proxy.hostName()).toString());
    setProxyServerPort(settings.value(SettingsProxyPort, _proxy.port()).toInt());

#ifndef AXELCHAT_LIBRARY
    //Bot
    _bot = new ChatBot(settings, SettingsGroupPath + "/chat_bot");

    //Output to file
    _outputToFile = new OutputToFile(settings, SettingsGroupPath + "/output_to_file");
#endif

    //YouTube
    _youTube = new YouTube(proxy(), settings, SettingsGroupPath + "/youtube");

    connect(_youTube, &YouTube::readyRead, this, &ChatHandler::onReadyRead);

    connect(_youTube, SIGNAL(connected(QString)),
                     this, SLOT(onConnected(QString)));

    connect(_youTube, SIGNAL(disconnected(QString)),
            this, SLOT(onDisconnected(QString)));

    connect(_youTube, SIGNAL(stateChanged()),
            this, SLOT(onStateChanged()));

    connect(_youTube, &AbstractChatService::needSendNotification,
            this, [&](const QString& text){
        sendNotification(QString("YouTube: ") + text);
    });

    //Twitch
    _twitch = new Twitch(proxy(), settings, SettingsGroupPath + "/twitch");

    connect(_twitch, SIGNAL(readyRead(QList<ChatMessage>&)),
                     this, SLOT(onReadyRead(QList<ChatMessage>&)));

    connect(_twitch, SIGNAL(avatarDiscovered(const QString&, const QUrl&)),
                     this, SLOT(onAvatarDiscovered(const QString&, const QUrl&)));


    connect(_twitch, SIGNAL(connected(QString)),
                     this, SLOT(onConnected(QString)));

    connect(_twitch, SIGNAL(disconnected(QString)),
            this, SLOT(onDisconnected(QString)));

    connect(_twitch, SIGNAL(stateChanged()),
            this, SLOT(onStateChanged()));

    connect(_twitch, &AbstractChatService::needSendNotification,
            this, [&](const QString& text){
        sendNotification(QString("Twitch: ") + text);
    });

    //GoodGame
    _goodGame = new GoodGame(proxy(), settings, SettingsGroupPath + "/goodgame");

    connect(_goodGame, SIGNAL(readyRead(QList<ChatMessage>&)),
                     this, SLOT(onReadyRead(QList<ChatMessage>&)));

    connect(_goodGame, SIGNAL(connected(QString)),
                     this, SLOT(onConnected(QString)));

    connect(_goodGame, SIGNAL(disconnected(QString)),
            this, SLOT(onDisconnected(QString)));

    connect(_goodGame, SIGNAL(stateChanged()),
            this, SLOT(onStateChanged()));

    connect(_goodGame, &AbstractChatService::needSendNotification,
            this, [&](const QString& text){
        sendNotification(QString("GoodGame: ") + text);
    });

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

    if (_twitch)
    {
        delete _twitch;
        _twitch = nullptr;
    }

    if (_goodGame)
    {
        delete _goodGame;
        _goodGame = nullptr;
    }

#ifndef AXELCHAT_LIBRARY
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
#endif
}

MessageAuthor ChatHandler::authorByChannelId(const QString &channelId) const
{
    return _authors.value(channelId);
}


//ToDo: использование ссылок в слотах и сигналах может плохо кончиться! Особенно, если соеденены разные потоки
void ChatHandler::onReadyRead(QList<ChatMessage>& messages)
{
    QList<ChatMessage> messagesValidToAdd;

    for (int i = 0; i < messages.count(); ++i)
    {
        ChatMessage&& message = std::move(messages[i]);

        if (_messagesModel.contains(message.id()) && !message.isDeleterItem())
        {
            continue;
        }

        /*qDebug(QString("%1: %2")
               .arg(message.authorName).arg(message.text).toUtf8());*/

        const MessageAuthor author = message.author();

        const QString channelId = author.channelId();

        if (_authors.contains(channelId))
        {
            _authors[channelId]._messagesSentCurrent++;
        }
        else
        {
            _authors[channelId] = author;
            _authors[channelId]._messagesSentCurrent = 1;
        }

#ifndef AXELCHAT_LIBRARY
        if (_bot && channelId != MessageAuthor::softwareAuthor().channelId())
        {
            _bot->processMessage(message);
        }
#endif

        messagesValidToAdd.append(std::move(message));
    }

    if (messagesValidToAdd.isEmpty())
    {
        return;
    }

    if (_outputToFile)
    {
        _outputToFile->writeMessages(messagesValidToAdd);
    }

    for (int i = 0; i < messagesValidToAdd.count(); ++i)
    {
        ChatMessage&& message = std::move(messagesValidToAdd[i]);
        _messagesModel.append(std::move(message));
    }

    emit messagesDataChanged();

    if (_enabledSoundNewMessage && !messages.empty())
    {
        playNewMessageSound();
    }
}

void ChatHandler::sendTestMessage(const QString &text)
{
    QList<ChatMessage> messages = {ChatMessage::createTestMessage(text)};
    onReadyRead(messages);
}

void ChatHandler::playNewMessageSound()
{
#ifdef QT_MULTIMEDIA_LIB
    if (_soundDefaultNewMessage)
    {
        _soundDefaultNewMessage->play();
    }
    else
    {
        qDebug() << "sound not exists";
    }
#else
    qWarning() << Q_FUNC_INFO << ": module multimedia not included";
#endif
}

void ChatHandler::onAvatarDiscovered(const QString &channelId, const QUrl &url)
{
    _messagesModel.applyAvatar(channelId, url);
}

void ChatHandler::clearMessages()
{
    _messagesModel.clear();
}

void ChatHandler::onStateChanged()
{
    if (_outputToFile)
    {
        if (qobject_cast<YouTube*>(sender()) && _youTube)
        {
            _outputToFile->setYouTubeInfo(_youTube->getInfo());
        }
        else if (qobject_cast<Twitch*>(sender()) && _twitch)
        {
            _outputToFile->setTwitchInfo(_twitch->getInfo());
        }
    }

    emit viewersTotalCountChanged();
}

#ifndef AXELCHAT_LIBRARY
void ChatHandler::openProgramFolder()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(QString("file:///") + QCoreApplication::applicationDirPath()));
}
#endif

void ChatHandler::onConnected(QString name)
{
    QString text;

    YouTube* youTube = dynamic_cast<YouTube*>(sender());
    if (youTube)
    {
        text = tr("YouTube connected");
    }

    Twitch* twitch = dynamic_cast<Twitch*>(sender());
    if (twitch)
    {
        text = tr("Twitch connected");
    }

    GoodGame* goodGame = dynamic_cast<GoodGame*>(sender());
    if (goodGame)
    {
        text = tr("GoodGame connected");
    }

    if (!name.isEmpty())
    {
        text += ": " + name;
    }

    sendNotification(text);

    emit connectedCountChanged();
}

void ChatHandler::onDisconnected(QString name)
{
    QString text;

    YouTube* youTube = dynamic_cast<YouTube*>(sender());
    if (youTube)
    {
        text = tr("YouTube disconnected");

    }

    Twitch* twitch = dynamic_cast<Twitch*>(sender());
    if (twitch)
    {
        text = tr("Twitch disconnected");
    }

    if (!name.isEmpty())
    {
        text += ": " + name;
    }

    sendNotification(text);

    if (_enabledClearMessagesOnLinkChange)
    {
        clearMessages();
    }

    emit connectedCountChanged();
}

void ChatHandler::sendNotification(const QString &text)
{
    QList<ChatMessage> messages = {ChatMessage::createSoftwareNotification(text)};
    onReadyRead(messages);
}

void ChatHandler::updateProxy()
{
    QNetworkProxy proxy(QNetworkProxy::NoProxy);
    if (_enabledProxy)
    {
        proxy = _proxy;
    }

    if (_youTube)
    {
        _youTube->setProxy(proxy);
    }

    if (_twitch)
    {
        _twitch->setProxy(proxy);
    }

    if (_goodGame)
    {
        _goodGame->setProxy(proxy);
    }

    emit proxyChanged();
}

#ifndef AXELCHAT_LIBRARY
ChatBot *ChatHandler::bot() const
{
    return _bot;
}

OutputToFile *ChatHandler::outputToFile() const
{
    return _outputToFile;
}
#endif

ChatMessagesModel* ChatHandler::messagesModel()
{
    return &_messagesModel;
}

#ifdef QT_QUICK_LIB
void ChatHandler::declareQml()
{
    AbstractChatService::declareQML();

    qmlRegisterUncreatableType<ChatHandler> ("AxelChat.ChatHandler",
                                             1, 0, "ChatHandler", "Type cannot be created in QML");

    qmlRegisterUncreatableType<YouTube> ("AxelChat.YouTube",
                                                    1, 0, "YouTube", "Type cannot be created in QML");

    qmlRegisterUncreatableType<Twitch> ("AxelChat.Twitch",
                                                    1, 0, "Twitch", "Type cannot be created in QML");

    qmlRegisterUncreatableType<OutputToFile> ("AxelChat.OutputToFile",
                                              1, 0, "OutputToFile", "Type cannot be created in QML");

    ChatBot::declareQml();
    MessageAuthor::declareQML();
    ChatMessage::declareQML();
}
#endif

void ChatHandler::setEnabledSoundNewMessage(bool enabled)
{
    if (_enabledSoundNewMessage != enabled)
    {
        _enabledSoundNewMessage = enabled;

        settings.setValue(SettingsEnabledSoundNewMessage, enabled);

        emit enabledSoundNewMessageChanged();
    }
}

void ChatHandler::setEnabledClearMessagesOnLinkChange(bool enabled)
{
    if (_enabledClearMessagesOnLinkChange != enabled)
    {
        _enabledClearMessagesOnLinkChange = enabled;

        settings.setValue(SettingsEnabledClearMessagesOnLinkChange, enabled);

        emit enabledClearMessagesOnLinkChangeChanged();
    }
}

int ChatHandler::connectedCount() const
{
    return (_youTube->connectionStateType()  == AbstractChatService::ConnectionStateType::Connected) +
           (_twitch->connectionStateType()   == AbstractChatService::ConnectionStateType::Connected) +
            (_goodGame->connectionStateType() == AbstractChatService::ConnectionStateType::Connected);
}

int ChatHandler::viewersTotalCount() const
{
    int result = 0;

    bool found = false;

    if (_youTube && _youTube->connectionStateType() == AbstractChatService::ConnectionStateType::Connected)
    {
        if (_youTube->viewersCount() < 0)
        {
            return -1;
        }

        result += _youTube->viewersCount();
        found = true;
    }

    if (_twitch && _twitch->connectionStateType() == AbstractChatService::ConnectionStateType::Connected)
    {
        if (_twitch->viewersCount() < 0)
        {
            return -1;
        }

        result += _twitch->viewersCount();
        found = true;
    }

    if (_goodGame && _goodGame->connectionStateType() == AbstractChatService::ConnectionStateType::Connected)
    {
        if (_goodGame->viewersCount() < 0)
        {
            return -1;
        }

        result += _goodGame->viewersCount();
        found = true;
    }

    if (!found)
    {
        return -1;
    }

    return result;
}

int ChatHandler::authorMessagesSentCurrent(const QString &channelId) const
{
    return _authors.value(channelId)._messagesSentCurrent;
}

QUrl ChatHandler::authorSizedAvatarUrl(const QString &channelId, int height) const
{
    return YouTube::createResizedAvatarUrl(_authors.value(channelId).avatarUrl(), height);
}

void ChatHandler::setProxyEnabled(bool enabled)
{
    if (_enabledProxy != enabled)
    {
        _enabledProxy = enabled;

        settings.setValue(SettingsProxyEnabled, enabled);

        updateProxy();
    }
}

void ChatHandler::setProxyServerAddress(QString address)
{
    address = address.trimmed();

    if (_proxy.hostName() != address)
    {
        _proxy.setHostName(address);

        settings.setValue(SettingsProxyAddress, address);

        updateProxy();
    }
}

void ChatHandler::setProxyServerPort(int port)
{
    if (_proxy.port() != port)
    {
        _proxy.setPort(port);

        settings.setValue(SettingsProxyPort, port);

        updateProxy();
    }
}

QNetworkProxy ChatHandler::proxy() const
{
    if (_enabledProxy)
    {
        return _proxy;
    }

    return QNetworkProxy(QNetworkProxy::NoProxy);
}

YouTube* ChatHandler::youTube() const
{
    return _youTube;
}

Twitch* ChatHandler::twitch() const
{
    return _twitch;
}

GoodGame *ChatHandler::goodGame() const
{
    return _goodGame;
}
