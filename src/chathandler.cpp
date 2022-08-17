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

ChatHandler::ChatHandler(QSettings& settings_, QNetworkAccessManager& network_, QObject *parent)
    : QObject(parent)
    , settings(settings_)
    , network(network_)
{
    setEnabledSoundNewMessage(settings.value(SettingsEnabledSoundNewMessage, _enabledSoundNewMessage).toBool());

    setEnabledClearMessagesOnLinkChange(settings.value(SettingsEnabledClearMessagesOnLinkChange, _enabledClearMessagesOnLinkChange).toBool());

    setProxyEnabled(settings.value(SettingsProxyEnabled, _enabledProxy).toBool());
    setProxyServerAddress(settings.value(SettingsProxyAddress, _proxy.hostName()).toString());
    setProxyServerPort(settings.value(SettingsProxyPort, _proxy.port()).toInt());

#ifndef AXELCHAT_LIBRARY
    //Bot
    _bot = new ChatBot(settings, SettingsGroupPath + "/chat_bot", this);

    //Output to file
    _outputToFile = new OutputToFile(settings, SettingsGroupPath + "/output_to_file", this);
#endif

    youtube = new YouTube(settings, SettingsGroupPath + "/youtube", network, this);
    addService(*youtube);

    twitch = new Twitch(settings, SettingsGroupPath + "/twitch", network, this);
    connect(twitch, &Twitch::avatarDiscovered, this, &ChatHandler::onAvatarDiscovered);
    addService(*twitch);

    goodGame = new GoodGame(settings, SettingsGroupPath + "/goodgame", network, this);
    addService(*goodGame);
}

MessageAuthor ChatHandler::getAuthorByChannelId(const QString &channelId) const
{
    return authors.value(channelId);
}

//ToDo: использование ссылок в слотах и сигналах может плохо кончиться! Особенно, если соеденены разные потоки
void ChatHandler::onReadyRead(QList<ChatMessage>& messages)
{
    QList<ChatMessage> messagesValidToAdd;

    for (int i = 0; i < messages.count(); ++i)
    {
        ChatMessage&& message = std::move(messages[i]);

        if (messagesModel.contains(message.id()) && !message.isDeleterItem())
        {
            continue;
        }

        /*qDebug(QString("%1: %2")
               .arg(message.authorName).arg(message.text).toUtf8());*/

        const MessageAuthor author = message.author();

        const QString channelId = author.channelId();

        if (authors.contains(channelId))
        {
            authors[channelId]._messagesSentCurrent++;
        }
        else
        {
            authors[channelId] = author;
            authors[channelId]._messagesSentCurrent = 1;
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
        messagesModel.append(std::move(message));
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
    messagesModel.applyAvatar(channelId, url);
}

void ChatHandler::clearMessages()
{
    messagesModel.clear();
}

void ChatHandler::onStateChanged()
{
    if (_outputToFile)
    {
        if (qobject_cast<YouTube*>(sender()) && youtube)
        {
            _outputToFile->setYouTubeInfo(youtube->getInfo());
        }
        else if (qobject_cast<Twitch*>(sender()) && twitch)
        {
            _outputToFile->setTwitchInfo(twitch->getInfo());
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
    AbstractChatService* service = qobject_cast<AbstractChatService*>(sender());
    if (!service)
    {
        return;
    }

    sendNotification(tr("%1 connected: %2").arg(service->getNameLocalized()).arg(name));

    emit connectedCountChanged();
}

void ChatHandler::onDisconnected(QString name)
{
    AbstractChatService* service = qobject_cast<AbstractChatService*>(sender());
    if (!service)
    {
        return;
    }

    sendNotification(tr("%1 disconnected: %2").arg(service->getNameLocalized()).arg(name));

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
    if (_enabledProxy)
    {
        network.setProxy(_proxy);
    }
    else
    {
        network.setProxy(QNetworkProxy(QNetworkProxy::NoProxy));
    }

    for (AbstractChatService* chat : qAsConst(services))
    {
        chat->reconnect();
    }

    emit proxyChanged();
}

void ChatHandler::addService(AbstractChatService& service)
{
    services.append(&service);

    connect(&service, &AbstractChatService::readyRead, this, &ChatHandler::onReadyRead);
    connect(&service, &AbstractChatService::connected, this, &ChatHandler::onConnected);
    connect(&service, &AbstractChatService::disconnected, this, &ChatHandler::onDisconnected);
    connect(&service, &AbstractChatService::stateChanged, this, &ChatHandler::onStateChanged);
    connect(&service, &AbstractChatService::needSendNotification, this, [this](const QString& text)
    {
        AbstractChatService* service = qobject_cast<AbstractChatService*>(sender());
        if (!service)
        {
            return;
        }

        sendNotification(service->getNameLocalized() + ": " + text);
    });
}

#ifndef AXELCHAT_LIBRARY
ChatBot& ChatHandler::getBot() const
{
    return *_bot;
}

OutputToFile& ChatHandler::getOutputToFile() const
{
    return *_outputToFile;
}
#endif

ChatMessagesModel& ChatHandler::getMessagesModel()
{
    return messagesModel;
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
    int result = 0;

    for (AbstractChatService* service : services)
    {
        if (service->connectionStateType()  == AbstractChatService::ConnectionStateType::Connected)
        {
            result++;
        }
    }

    return result;
}

int ChatHandler::viewersTotalCount() const
{
    int result = 0;

    for (AbstractChatService* service : services)
    {
        if (service->connectionStateType()  == AbstractChatService::ConnectionStateType::Connected)
        {
            const int count = service->viewersCount();
            if (count < 0)
            {
                return -1;
            }
            else
            {
                result += count;
            }
        }
    }

    return result;
}

int ChatHandler::authorMessagesSentCurrent(const QString &channelId) const
{
    return authors.value(channelId)._messagesSentCurrent;
}

QUrl ChatHandler::authorSizedAvatarUrl(const QString &channelId, int height) const
{
    return YouTube::createResizedAvatarUrl(authors.value(channelId).avatarUrl(), height);
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

YouTube& ChatHandler::getYoutube()
{
    return *youtube;
}

Twitch& ChatHandler::getTwitch() const
{
    return *twitch;
}

GoodGame& ChatHandler::getGoodGame() const
{
    return *goodGame;
}
