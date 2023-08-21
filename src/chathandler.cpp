#include "chathandler.h"
#include "chat_services/youtubehtml.h"
#include "chat_services/youtubebrowser.h"
#include "chat_services/twitch.h"
#include "chat_services/trovo.h"
#include "chat_services/rumble.h"
#include "chat_services/goodgame.h"
#include "chat_services/vkplaylive.h"
#include "chat_services/telegram.h"
#include "chat_services/discord.h"
#include "chat_services/vkvideo.h"
#include "chat_services/wasd.h"
#include "chat_services/kick.h"
#include "chat_services/donationalerts.h"
#include "chat_services/donatepay.h"
#include "models/author.h"
#include "models/message.h"
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
static const QString SettingsEnabledShowAuthorNameChanged       = SettingsGroupPath + "/enabledShowAuthorNameChanged";
static const QString SettingsProxyEnabled                       = SettingsGroupPath + "/proxyEnabled";
static const QString SettingsProxyAddress                       = SettingsGroupPath + "/proxyServerAddress";
static const QString SettingsProxyPort                          = SettingsGroupPath + "/proxyServerPort";

}

ChatHandler::ChatHandler(QSettings& settings_, QNetworkAccessManager& network_, cweqt::Manager& web_, QObject *parent)
    : QObject(parent)
    , settings(settings_)
    , network(network_)
    , web(web_)
    , emotesProcessor(settings_, SettingsGroupPath, network_)
    , outputToFile(settings, SettingsGroupPath + "/output_to_file", network, messagesModel, services)
    , bot(settings, SettingsGroupPath + "/chat_bot")
    , authorQMLProvider(*this, messagesModel, outputToFile)
    , webSocket(*this)
    , tcpServer(services)
{
    connect(&outputToFile, &OutputToFile::authorNameChanged, this, &ChatHandler::onAuthorNameChanged);

    setEnabledSoundNewMessage(settings.value(SettingsEnabledSoundNewMessage, _enabledSoundNewMessage).toBool());

    setEnabledClearMessagesOnLinkChange(settings.value(SettingsEnabledClearMessagesOnLinkChange, _enabledClearMessagesOnLinkChange).toBool());

    setEnabledShowAuthorNameChanged(settings.value(SettingsEnabledShowAuthorNameChanged, _enableShowAuthorNameChanged).toBool());

    setProxyEnabled(settings.value(SettingsProxyEnabled, _enabledProxy).toBool());
    setProxyServerAddress(settings.value(SettingsProxyAddress, _proxy.hostName()).toString());
    setProxyServerPort(settings.value(SettingsProxyPort, _proxy.port()).toInt());
    
    addService<YouTubeHtml>();
    //addService<YouTubeBrowser>();
    addService<Twitch>();
    addService<Trovo>();
    addService<Rumble>();
    addService<Kick>();
    addService<GoodGame>();
    addService<VkPlayLive>();
    addService<VkVideo>();
    addService<Wasd>();
    addService<Telegram>();
    addService<Discord>();
    addService<DonationAlerts>();
    //addService<DonatePay>();

    QTimer::singleShot(2000, [this]()
    {
        Q_UNUSED(this)
        //addTestMessages();
    });
}

ChatHandler::~ChatHandler()
{
    const int count = services.count();
    for (int i = 0; i < count; i++)
    {
        removeService(0);
    }
}

void ChatHandler::onReadyRead(const QList<std::shared_ptr<Message>>& messages, const QList<std::shared_ptr<Author>>& authors)
{
    ChatService* service = static_cast<ChatService*>(sender());

    if (messages.isEmpty() && authors.isEmpty())
    {
        return;
    }

    if (messages.count() != authors.count())
    {
        qWarning() << "The number of messages is not equal to the number of authors";
        return;
    }

    const AxelChat::ServiceType serviceType = service ? service->getServiceType() : AxelChat::ServiceType::Unknown;

    QList<std::shared_ptr<Message>> messagesValidToAdd;
    QList<std::shared_ptr<Author>> updatedAuthors;

    for (int i = 0; i < messages.count(); ++i)
    {
        std::shared_ptr<Message> message = messages[i];
        if (!message)
        {
            qWarning() << Q_FUNC_INFO << "message is null";
            continue;
        }

        if (messagesModel.contains(message->getId()) && !message->isHasFlag(Message::Flag::DeleterItem))
        {
            continue;
        }

        std::shared_ptr<Author> author = authors[i];
        if (!author)
        {
            qWarning() << Q_FUNC_INFO << "author is null";
            continue;
        }

        messagesModel.addAuthor(author);

        if ((service && service->isEnabledThirdPartyEmotes()) || !service)
        {
            emotesProcessor.processMessage(message);
        }

        messagesValidToAdd.append(message);
    }

    outputToFile.writeAuthors(updatedAuthors);

    if (messagesValidToAdd.isEmpty())
    {
        return;
    }

    outputToFile.writeMessages(messagesValidToAdd, serviceType);
    webSocket.sendMessages(messagesValidToAdd);

    for (int i = 0; i < messagesValidToAdd.count(); ++i)
    {
        const std::shared_ptr<Message>& message = messagesValidToAdd[i];

#ifndef AXELCHAT_LIBRARY
        if (!message->isHasFlag(Message::Flag::IgnoreBotCommand))
        {
            bot.processMessage(message);
        }
#endif
        
        messagesModel.addMessage(message);
    }

    emit messagesDataChanged();

    if (_enabledSoundNewMessage && !messages.empty())
    {
        playNewMessageSound();
    }
}

void ChatHandler::sendTestMessage(const QString &text)
{
    std::shared_ptr<Author> author = Author::getSoftwareAuthor();

    std::shared_ptr<Message> message = std::make_shared<Message>(
        QList<std::shared_ptr<Message::Content>>{std::make_shared<Message::Text>(text)},
        author);

    message->setCustomAuthorName(tr("Test Message"));
    message->setCustomAuthorAvatarUrl(QUrl("qrc:/resources/images/flask2.svg"));

    onReadyRead({message}, {author});
}

void ChatHandler::sendSoftwareMessage(const QString &text)
{
    std::shared_ptr<Author> author = Author::getSoftwareAuthor();
\
    std::shared_ptr<Message> message = std::make_shared<Message>(
        QList<std::shared_ptr<Message::Content>>{std::make_shared<Message::Text>(text)},
        author,
        QDateTime::currentDateTime(),
        QDateTime::currentDateTime(),
        QString());

    onReadyRead({message}, {author});
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

void ChatHandler::onAuthorDataUpdated(const QString& authorId, const QMap<Author::Role, QVariant>& values)
{
    AxelChat::ServiceType serviceType = AxelChat::ServiceType::Unknown;
    ChatService* service = qobject_cast<ChatService*>(sender());
    if (service)
    {
        serviceType = service->getServiceType();
    }

    const QList<Author::Role> roles = values.keys();
    for (const Author::Role role : roles)
    {
        if (role == Author::Role::AvatarUrl)
        {
            outputToFile.downloadAvatar(authorId, serviceType, values[role].toUrl());
        }
    }

    messagesModel.setAuthorValues(serviceType, authorId, values);
    webSocket.sendAuthorValues(authorId, values);
}

void ChatHandler::clearMessages()
{
    messagesModel.clear();
}

void ChatHandler::onStateChanged()
{
    ChatService* service = dynamic_cast<ChatService*>(sender());
    if (service)
    {
        outputToFile.writeServiceState(service);
    }

    outputToFile.writeApplicationState(true, getViewersTotalCount());
    webSocket.sendState();

    emit viewersTotalCountChanged();
}

#ifndef AXELCHAT_LIBRARY
void ChatHandler::openProgramFolder()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(QString("file:///") + QCoreApplication::applicationDirPath()));
}
#endif

void ChatHandler::onConnectedChanged(const bool connected)
{
    ChatService* service = qobject_cast<ChatService*>(sender());
    if (!service)
    {
        qCritical() << Q_FUNC_INFO << "!service";
        return;
    }

    if (!connected && _enabledClearMessagesOnLinkChange)
    {
        clearMessages();
    }

    emit connectedCountChanged();
}

void ChatHandler::onAuthorNameChanged(const Author& author, const QString &prevName, const QString &newName)
{
    if (_enableShowAuthorNameChanged)
    {
        sendSoftwareMessage(tr("%1: \"%2\" changed name to \"%3\"")
                            .arg(ChatService::getName(author.getServiceType()), prevName, newName));
    }
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

    for (const std::shared_ptr<ChatService>& service : qAsConst(services))
    {
        if (!service)
        {
            qWarning() << Q_FUNC_INFO << "service is null";
            continue;
        }

        service->reconnect();
    }

    emit proxyChanged();
}

void ChatHandler::removeService(const int index)
{
    if (index >= services.count() || index < 0)
    {
        qWarning() << "index not valid";
        return;
    }

    std::shared_ptr<ChatService> service = services.at(index);
    if (service)
    {
        QObject::disconnect(service.get(), nullptr, this, nullptr);
        QObject::disconnect(this, nullptr, service.get(), nullptr);
    }

    services.removeAt(index);
}

template<typename ChatServiceInheritedClass>
void ChatHandler::addService()
{
    static_assert(std::is_base_of<ChatService, ChatServiceInheritedClass>::value, "ChatServiceInheritedClass must derive from ChatService");

    std::shared_ptr<ChatServiceInheritedClass> service = std::make_shared<ChatServiceInheritedClass>(settings, SettingsGroupPath, network, web, this);

    QObject::connect(service.get(), &ChatService::stateChanged, this, &ChatHandler::onStateChanged);
    QObject::connect(service.get(), &ChatService::readyRead, this, &ChatHandler::onReadyRead);
    QObject::connect(service.get(), &ChatService::connectedChanged, this, &ChatHandler::onConnectedChanged);
    QObject::connect(service.get(), &ChatService::authorDataUpdated, this, &ChatHandler::onAuthorDataUpdated);

    services.append(service);

    if (service->getServiceType() == AxelChat::ServiceType::Twitch)
    {
        if (std::shared_ptr<Twitch> twitch = std::dynamic_pointer_cast<Twitch>(service); twitch)
        {
            emotesProcessor.connectTwitch(twitch);
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Failed to cast to Twitch";
        }
    }
}

void ChatHandler::addTestMessages()
{
    QList<std::shared_ptr<Message>> messages;
    QList<std::shared_ptr<Author>> authors;

    {
        auto author = Author::Builder(
                          AxelChat::ServiceType::YouTube,
                          QUuid::createUuid().toString(),
                          "Mario")
                          .setAvatar("https://static.wikia.nocookie.net/mario/images/e/e3/MPS_Mario.png/revision/latest/scale-to-width-down/350?cb=20220814154953")
                          .build();

        messages.append(
            Message::Builder(author)
            .addText("Hello it's me Mario!")
            .build());

        authors.append(author);
    }

    {
        auto author = Author::Builder(
                          AxelChat::ServiceType::Twitch,
                          QUuid::createUuid().toString(),
                          "Big Smoke")
                          .setAvatar("https://static.wikia.nocookie.net/gtawiki/images/b/bf/BigSmoke-GTASAde.png/revision/latest/scale-to-width-down/350?cb=20211113214309")
                          .build();

        messages.append(
            Message::Builder(author)
                .addText("I’ll have two number 9s, a number 9 large, a number 6 with extra dip, a number 7, two number 45s, one with cheese, and a large soda")
                .build());

        authors.append(author);
    }

    {
        auto author = Author::Builder(
                          AxelChat::ServiceType::Trovo,
                          QUuid::createUuid().toString(),
                          "Luigi")
                          .setAvatar("https://static.wikia.nocookie.net/mario/images/7/72/MPSS_Luigi.png/revision/latest/scale-to-width-down/254?cb=20220705200355")
                          .setCustomNicknameColor(QColor("#FF00C7"))
                          .setCustomNicknameBackgroundColor(QColor("#FFFFFF"))
                          .build();

        messages.append(
            Message::Builder(author)
                .addText("Mamma mia!")
                .setForcedColor(Message::ColorRole::BodyBackgroundColorRole, QColor("#A8D9FF"))
                .setDestination({ "Underground" })
                .build());

        authors.append(author);
    }

    {
        auto author = Author::Builder(
                          AxelChat::ServiceType::GoodGame,
                          QUuid::createUuid().toString(),
                          "CJ")
                          .setAvatar("https://static.wikia.nocookie.net/gtawiki/images/2/29/CarlJohnson-GTASAde-Infobox.png/revision/latest/scale-to-width-down/350?cb=20211113054252")
                          .addLeftBadge("https://static.wikia.nocookie.net/gtawiki/images/2/29/CarlJohnson-GTASAde-Infobox.png/revision/latest/scale-to-width-down/350?cb=20211113054252")
                          .addLeftBadge("https://static.wikia.nocookie.net/gtawiki/images/2/29/CarlJohnson-GTASAde-Infobox.png/revision/latest/scale-to-width-down/350?cb=20211113054252")
                          .addRightBadge("https://static.wikia.nocookie.net/gtawiki/images/2/29/CarlJohnson-GTASAde-Infobox.png/revision/latest/scale-to-width-down/350?cb=20211113054252")
                          .addRightBadge("https://static.wikia.nocookie.net/gtawiki/images/2/29/CarlJohnson-GTASAde-Infobox.png/revision/latest/scale-to-width-down/350?cb=20211113054252")
                          .build();

        messages.append(
            Message::Builder(author)
                .addText("Here We Go Again")
                .build());

        authors.append(author);
    }

    {
        auto author = Author::Builder(
                          AxelChat::ServiceType::VkPlayLive,
                          QUuid::createUuid().toString(),
                          "Kenneth Rosenberg")
                          .setAvatar("https://static.wikia.nocookie.net/p__/images/b/b2/Ken_rosenberg.jpg/revision/latest?cb=20130915190559&path-prefix=protagonist")
                          .build();

        messages.append(
            Message::Builder(author)
                .addText("Hey, just like old times, huh, Tommy?")
                .build());

        authors.append(author);
    }

    {
        auto author = Author::Builder(
                          AxelChat::ServiceType::Telegram,
                          QUuid::createUuid().toString(),
                          "G-Man")
                          .setAvatar("https://static.wikia.nocookie.net/half-life/images/4/41/G-Man_Alyx_Trailer.jpg/revision/latest/scale-to-width-down/350?cb=20191122020607&path-prefix=en")
                          .build();

        messages.append(
            Message::Builder(author)
                .addText("Rise and shine, Mister Freeman")
                .setDestination({ "Xen", "Stasis" })
                .build());

        authors.append(author);
    }

    {
        auto author = Author::Builder(
                          AxelChat::ServiceType::Discord,
                          QUuid::createUuid().toString(),
                          "Gordon Freeman")
                          .setAvatar("https://static.wikia.nocookie.net/half-life/images/1/1f/GordonALYX.png/revision/latest/scale-to-width-down/350?cb=20220520125500&path-prefix=en")
                          .build();

        messages.append(
            Message::Builder(author)
                .addText("May I say a few words?")
                .build());

        authors.append(author);
    }

    onReadyRead(messages, authors);
}

#ifndef AXELCHAT_LIBRARY
ChatBot& ChatHandler::getBot()
{
    return bot;
}

OutputToFile& ChatHandler::getOutputToFile()
{
    return outputToFile;
}
#endif

MessagesModel& ChatHandler::getMessagesModel()
{
    return messagesModel;
}

void ChatHandler::setEnabledSoundNewMessage(bool enabled)
{
    if (_enabledSoundNewMessage != enabled)
    {
        _enabledSoundNewMessage = enabled;

        settings.setValue(SettingsEnabledSoundNewMessage, enabled);

        emit enabledSoundNewMessageChanged();
    }
}

void ChatHandler::setEnabledShowAuthorNameChanged(bool enabled)
{
    if (_enableShowAuthorNameChanged != enabled)
    {
        _enableShowAuthorNameChanged = enabled;

        settings.setValue(SettingsEnabledShowAuthorNameChanged, enabled);

        emit enabledShowAuthorNameChangedChanged();
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

    for (const std::shared_ptr<ChatService>& service : qAsConst(services))
    {
        if (!service)
        {
            qWarning() << Q_FUNC_INFO << "service is null";
            continue;
        }

        if (service->getConnectionStateType()  == ChatService::ConnectionStateType::Connected)
        {
            result++;
        }
    }

    return result;
}

int ChatHandler::getViewersTotalCount() const
{
    int result = 0;

    for (const std::shared_ptr<ChatService>& service : qAsConst(services))
    {
        if (!service)
        {
            qWarning() << Q_FUNC_INFO << "service is null";
            continue;
        }

        const int count = service->getViewersCount();
        if (count >= 0)
        {
            result += count;
        }
    }

    return result;
}

bool ChatHandler::isKnownViewesServicesMoreOne() const
{
    int count = 0;

    for (const std::shared_ptr<ChatService>& service : qAsConst(services))
    {
        if (!service)
        {
            continue;
        }

        if (service->getState().viewersCount >= 0)
        {
            count++;

            if (count >= 2)
            {
                return true;
            }
        }
    }

    return false;
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

int ChatHandler::getServicesCount() const
{
    return services.count();
}

ChatService *ChatHandler::getServiceAtIndex(int index) const
{
    if (index >= services.count() || index < 0)
    {
        return nullptr;
    }

    return services.at(index).get();
}

ChatService *ChatHandler::getServiceByType(int type) const
{
    for (const std::shared_ptr<ChatService>& service : services)
    {
        if (!service)
        {
            qWarning() << Q_FUNC_INFO << "service is null";
            continue;
        }

        if (service->getServiceType() == (AxelChat::ServiceType)type)
        {
            return service.get();
        }
    }

    return nullptr;
}

QUrl ChatHandler::getServiceIconUrl(int serviceType) const
{
    return ChatService::getIconUrl((AxelChat::ServiceType)serviceType);
}

QUrl ChatHandler::getServiceName(int serviceType) const
{
    return ChatService::getName((AxelChat::ServiceType)serviceType);
}
