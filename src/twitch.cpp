#include "twitch.hpp"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDesktopServices>
#include <QAbstractSocket>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QColor>

namespace
{

static const QByteArray AcceptLanguageNetworkHeaderName = ""; // "en-US;q=0.5,en;q=0.3";

static const QString ApplicationClientID = "cx5rgsivc62io2kk79yf6eivhhwiui";
static const QString RedirectUri = "https://twitchapps.com/tmi/";//"https://localhost";
static const QString TwitchIRCHost = "tmi.twitch.tv";

static const QString SettingsKeyOAuthToken = "oauth_token";
static const QString SettingsKeyUserSpecifiedChannel = "user_specified_channel";

static const QString FolderLogs = "logs_twitch";

static const int ReconncectPeriod = 2 * 1000;
static const int PingPeriod = 60 * 1000;
static const int PongTimeout = 5 * 1000;
static const int UpdateStreamInfoPeriod = 10 * 1000;

static bool needIgnoreMessage(const QString& text)
{
    if (text.isEmpty())
    {
        return true;
    }

    return false;
}

static bool checkReply(QNetworkReply *reply, const char *tag, QByteArray& resultData)
{
    resultData.clear();

    if (!reply)
    {
        qWarning() << tag << ": !reply";
        return false;
    }

    resultData = reply->readAll();
    if (resultData.isEmpty())
    {
        qWarning() << tag << ": data is empty";
        return false;
    }

    const QJsonObject root = QJsonDocument::fromJson(resultData).object();
    if (root.contains("error"))
    {
        qWarning() << tag << "Error:" << resultData;
        return false;
    }

    return true;
}

}

Twitch::Twitch(const QNetworkProxy& proxy, QSettings& settings_, const QString& settingsGroupPath, QObject *parent)
  : AbstractChatService(proxy, parent)
  , settings(settings_)
  , SettingsGroupPath(settingsGroupPath)
{
    _socket.setProxy(proxy);

    QObject::connect(&_socket, &QWebSocket::stateChanged, this, [=](QAbstractSocket::SocketState state){
        Q_UNUSED(state)
        //qDebug() << "Twitch: WebSocket state changed:" << state;
    });

    QObject::connect(&_socket, &QWebSocket::textMessageReceived, this, &Twitch::onIRCMessage);

    QObject::connect(&_socket, &QWebSocket::connected, this, [=]() {
        qDebug() << "Twitch: connected" << _info.channelLogin;

        if (_info.connected)
        {
            _info.connected = false;
            emit stateChanged();
        }

        _info.viewers = -1;

        sendIRCMessage("CAP REQ :twitch.tv/tags");
        //sendIRCMessage("CAP REQ :twitch.tv/commands");
        sendIRCMessage(QString("PASS oauth:") + _info.oauthToken);
        sendIRCMessage(QString("NICK ") + _info.channelLogin);
        sendIRCMessage(QString("JOIN #") + _info.channelLogin);
        sendIRCMessage(QString("PING :") + TwitchIRCHost);

        requestUserInfo(_info.channelLogin);
        requestStreamInfo(_info.channelLogin);
    });

    QObject::connect(&_socket, &QWebSocket::disconnected, this, [=](){
        qDebug() << "Twitch: disconnected";

        if (_info.connected)
        {
            _info.connected = false;
            emit disconnected(_lastConnectedChannelName);
            emit stateChanged();
        }

        _info.viewers = -1;
    });

    QObject::connect(&_socket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), this, [=](QAbstractSocket::SocketError error_){
        qDebug() << "Twitch: WebSocket error:" << error_;
    });

    reInitSocket();

    QObject::connect(&_timerReconnect, &QTimer::timeout, this, [&](){
        if (!_info.connected && !_info.oauthToken.isEmpty())
        {
            reInitSocket();
        }
    });
    _timerReconnect.start(ReconncectPeriod);

    QObject::connect(&_timerCheckPong, &QTimer::timeout, this, [&]{
        if (_info.connected)
        {
            qWarning() << Q_FUNC_INFO << "Pong timeout! Reconnection...";

            sendNotification(tr("Ping timeout! Reconnection..."));

            _info.connected = false;

            emit disconnected(_lastConnectedChannelName);
            emit stateChanged();
            reInitSocket();
        }
    });

    QObject::connect(&_timerPing, &QTimer::timeout, this, [&]{
        if (_info.connected)
        {
            sendIRCMessage(QString("PING :") + TwitchIRCHost);
            _timerCheckPong.start(PongTimeout);
        }
    });
    _timerPing.start(PingPeriod);

    QObject::connect(&_timerUpdaetStreamInfo, &QTimer::timeout, this, [&]{
        if (_info.connected)
        {
            requestStreamInfo(_info.channelLogin);
        }
    });
    _timerUpdaetStreamInfo.start(UpdateStreamInfoPeriod);

    const QString token = QString::fromUtf8(QByteArray::fromBase64(settings.value(SettingsGroupPath + "/" + SettingsKeyOAuthToken).toByteArray()));
    setOAuthToken(token);

    setUserSpecifiedChannel(settings.value(SettingsGroupPath + "/" + SettingsKeyUserSpecifiedChannel).toString());

    QFile fileBadges(":/resources/twitch-global-badges-20210728.json");
    if (fileBadges.open(QIODevice::OpenModeFlag::ReadOnly | QIODevice::OpenModeFlag::Text))
    {
        parseBadgesJson(fileBadges.readAll());
        fileBadges.close();
    }
    else
    {
        qWarning() << Q_FUNC_INFO << "failed to open file" << fileBadges;
    }

    requestForGlobalBadges();
}

Twitch::~Twitch()
{
    _socket.close();

    _info.connected = false;
    emit disconnected(_lastConnectedChannelName);
    emit stateChanged();
}

AbstractChatService::ConnectionStateType Twitch::connectionStateType() const
{
    if (_info.connected)
    {
        return AbstractChatService::ConnectionStateType::Connected;
    }
    else if (!_info.oauthToken.isEmpty() && !_info.channelLogin.isEmpty())
    {
        return AbstractChatService::ConnectionStateType::Connecting;
    }

    return AbstractChatService::ConnectionStateType::NotConnected;
}

QString Twitch::stateDescription() const
{
    switch (connectionStateType()) {
    case ConnectionStateType::NotConnected:
        if (_info.channelLogin.isEmpty())
        {
            return tr("Channel not specified");
        }

        if (_info.oauthToken.isEmpty())
        {
            return tr("OAuth token not specified");
        }

        return tr("Not connected");

    case ConnectionStateType::Connecting:
        return tr("Connecting...");

    case ConnectionStateType::Connected:
        return tr("Successfully connected!");

    }

    return "<unknown_state>";
}

QString Twitch::detailedInformation() const
{
    return _info.detailedInformation;
}

int Twitch::viewersCount() const
{
    return _info.viewers;
}

QUrl Twitch::requesGetAOuthTokenUrl() const
{
    return QUrl("https://id.twitch.tv/oauth2/authorize?client_id=" + ApplicationClientID
                        + "&redirect_uri=" + RedirectUri
                        + "&response_type=token"
                        + "&scope=openid+chat:read");
}

QUrl Twitch::chatUrl() const
{
    if (_info.channelLogin.isEmpty())
    {
        return QUrl();
    }

    return _info.chatUrl;
}

QUrl Twitch::controlPanelUrl() const
{
    if (_info.channelLogin.isEmpty())
    {
        return QUrl();
    }

    return _info.controlPanelUrl;
}

QUrl Twitch::broadcastUrl() const
{
    if (_info.channelLogin.isEmpty())
    {
        return QUrl();
    }

    return _info.channelUrl;
}

bool Twitch::isChannelNameUserSpecified() const
{
    return _info.userSpecifiedChannel.trimmed() == _info.channelLogin.trimmed() && !_info.userSpecifiedChannel.isEmpty();
}

void Twitch::setProxy(const QNetworkProxy &proxy)
{
    _socket.setProxy(proxy);
    reInitSocket();
}

void Twitch::setUserSpecifiedChannel(QString userChannel)
{
    userChannel = userChannel.trimmed();

    if (_info.userSpecifiedChannel != userChannel)
    {
        _info.userSpecifiedChannel = userChannel;

        settings.setValue(SettingsGroupPath + "/" + SettingsKeyUserSpecifiedChannel, _info.userSpecifiedChannel);

        reInitSocket();

        emit stateChanged();
    }
}

void Twitch::setOAuthToken(QString token)
{
    if (token.startsWith("oauth:", Qt::CaseSensitivity::CaseInsensitive))
    {
        token = token.mid(6);
    }

    if (_info.oauthToken != token)
    {
        _info.oauthToken = token;

        settings.setValue(SettingsGroupPath + "/" + SettingsKeyOAuthToken, _info.oauthToken.toUtf8().toBase64());

        reInitSocket();

        emit stateChanged();
    }
}

void Twitch::sendIRCMessage(const QString &message)
{
    //qDebug() << "Twitch: send:" << message.toUtf8() << "\n";
    _socket.sendTextMessage(message);
}

void Twitch::reInitSocket()
{
    _info.userSpecifiedChannel = _info.userSpecifiedChannel.trimmed().toLower();

    QRegExp rx;

    // user channel
    _info.channelLogin.clear();
    const QString simpleUserSpecifiedUserChannel = AxelChat::simplifyUrl(_info.userSpecifiedChannel);
    rx = QRegExp("^twitch.tv/([^/]*)$", Qt::CaseInsensitive);
    if (rx.indexIn(simpleUserSpecifiedUserChannel) != -1)
    {
        _info.channelLogin = rx.cap(1);
    }

    if (_info.channelLogin.isEmpty())
    {
        rx = QRegExp("^[a-zA-Z0-9_]+$", Qt::CaseInsensitive);
        if (rx.indexIn(_info.userSpecifiedChannel) != -1)
        {
            _info.channelLogin = _info.userSpecifiedChannel;
        }
    }

    _info.chatUrl = QUrl(QString("https://www.twitch.tv/popout/%1/chat").arg(_info.channelLogin));

    _info.channelUrl = QUrl(QString("https://www.twitch.tv/%1").arg(_info.channelLogin));

    _info.controlPanelUrl = QUrl(QString("https://dashboard.twitch.tv/u/%1/stream-manager").arg(_info.channelLogin));

    _socket.close();

    if (_info.connected)
    {
        _info.connected = false;
        emit stateChanged();
    }

    if (!_info.channelLogin.isEmpty())
    {
        // ToDo: use SSL? wss://irc-ws.chat.twitch.tv:443
        _socket.open(QUrl("ws://irc-ws.chat.twitch.tv:80"));
    }
}

void Twitch::onIRCMessage(const QString &rawData)
{
    if (_timerCheckPong.isActive())
    {
        _timerCheckPong.stop();
    }

    QList<ChatMessage> messages;

    const QVector<QStringRef> rawMessages = rawData.splitRef("\r\n");
    for (const QStringRef& raw : rawMessages)
    {
        QSet<ChatMessage::Flags> flags;

        QString rawMessage = raw.trimmed().toString();
        if (rawMessage.isEmpty())
        {
            continue;
        }

        //qDebug() << "Twitch: received:" << rawMessage.toUtf8() << "\n";

        if (rawMessage.startsWith("PING", Qt::CaseSensitivity::CaseInsensitive))
        {
            sendIRCMessage(QString("PONG :") + TwitchIRCHost);
        }

        if (!_info.connected && rawMessage.startsWith(':') && rawMessage.count(':') == 1 && rawMessage.contains("JOIN #", Qt::CaseSensitivity::CaseInsensitive))
        {
            _info.connected = true;
            _lastConnectedChannelName = _info.channelLogin;
            emit connected(_info.channelLogin);
            emit stateChanged();
        }

        if (rawMessage.startsWith(":" + TwitchIRCHost))
        {
            // service messages should not be displayed
            continue;
        }

        // user message
        //@badge-info=;badges=broadcaster/1;client-nonce=24ce478a2773ed000a5553c13c1a3e05;color=#8A2BE2;display-name=Axe1_k;emotes=;flags=;id=3549a9a1-a4b2-4c25-876d-b514a89506b0;mod=0;room-id=215601682;subscriber=0;tmi-sent-ts=1627320332721;turbo=0;user-id=215601682;user-type= :axe1_k!axe1_k@axe1_k.tmi.twitch.tv PRIVMSG #axe1_k :2332

        if (!rawMessage.startsWith("@"))
        {
            continue;
        }

        static const QString Snippet1 = ".tmi.twitch.tv PRIVMSG #";
        const int posSnippet1 = rawMessage.indexOf(Snippet1);
        if (posSnippet1 == -1)
        {
            continue;
        }

        const QString snippet2 = rawMessage.mid(posSnippet1 + Snippet1.length()); // [owner-channel-id] :[message-text]
        QString rawMessageText = snippet2.mid(snippet2.indexOf(':') + 1);

        if (rawMessageText.startsWith(QString("\u0001ACTION")) && rawMessageText.endsWith("\u0001"))
        {
            rawMessageText = rawMessageText.mid(7);
            rawMessageText = rawMessageText.left(rawMessageText.length() - 1);
            rawMessageText = rawMessageText.trimmed();
            flags.insert(ChatMessage::Flags::TwitchAction);
        }

        if (needIgnoreMessage(rawMessageText))
        {
            //qDebug(QString("Twitch: ignore message \"%1\"").arg(messageText).toUtf8());
            continue;
        }

        const QString snippet3 = rawMessage.left(posSnippet1); // [@tags] :[channel-id]![channel-id]@[channel-id]

        const QString channelLogin = snippet3.mid(snippet3.lastIndexOf("@") + 1);

        QString displayName;
        QColor nicknameColor;
        QMap<QString, QString> badges;
        QVector<MessageEmoteInfo> emotesInfo;

        const QString tagsSnippet = snippet3.left(snippet3.lastIndexOf(":")).mid(1).trimmed();
        const QVector<QStringRef> rawTags = tagsSnippet.splitRef(";", Qt::SplitBehaviorFlags::SkipEmptyParts);
        for (const QStringRef& tag : rawTags)
        {
            // https://dev.twitch.tv/docs/irc/tags

            if (!tag.contains("="))
            {
                continue;
            }

            const QVector<QStringRef> tagArray = tag.split("=", Qt::SplitBehaviorFlags::SkipEmptyParts);
            if (tagArray.count() > 2)
            {
                qWarning() << Q_FUNC_INFO << "tag" << tag << "contains more than 1 equals";
            }

            if (tagArray.isEmpty())
            {
                continue;
            }

            const QStringRef tagName = tagArray.first().trimmed();
            QString tagValue;
            if (tagArray.count() >= 2)
            {
                tagValue = tagArray[1].trimmed().toString();

                //qDebug() << "Twitch:" << tagName << "=" << tagValue;
            }

            if (tagName == "color")
            {
                nicknameColor = QColor(tagValue);
            }
            else if (tagName == "display-name")
            {
                displayName = tagValue;
            }
            else if (tagName == "emotes")
            {
                if (tagValue.isEmpty())
                {
                    continue;
                }

                // http://static-cdn.jtvnw.net/emoticons/v1/:<emote ID>/:<size>
                // https://static-cdn.jtvnw.net/emoticons/v2/:<emote ID>/default/light/:<size>
                // "166266:20-28/64138:113-121" "244:8-16" "1902:36-40" "558563:5-11"
                const QVector<QStringRef> emotesPartInfo = tagValue.splitRef('/', Qt::SplitBehaviorFlags::SkipEmptyParts);
                for (const QStringRef& emotePartInfo : emotesPartInfo)
                {
                    const QStringRef emoteId = emotePartInfo.left(emotePartInfo.indexOf(':'));
                    if (emoteId.isEmpty())
                    {
                        continue;
                    }

                    MessageEmoteInfo emoteInfo;
                    emoteInfo.id = emoteId.toString();

                    const QStringRef emoteIndexesInfoPart = emotePartInfo.mid(emotePartInfo.indexOf(':') + 1);
                    const QVector<QStringRef> emoteIndexesPairsPart = emoteIndexesInfoPart.split(",", Qt::SplitBehaviorFlags::SkipEmptyParts);
                    for (const QStringRef& emoteIndexesPair : emoteIndexesPairsPart)
                    {
                        const QVector<QStringRef> emoteIndexes = emoteIndexesPair.split('-', Qt::SplitBehaviorFlags::SkipEmptyParts);
                        if (emoteIndexes.size() != 2)
                        {
                            qDebug() << Q_FUNC_INFO << "emoteIndexes.size() != 2, emotesPartInfo =" << emotesPartInfo;
                            continue;
                        }

                        bool ok = false;
                        const int firstIndex = emoteIndexes[0].toInt(&ok);
                        if (!ok)
                        {
                            qDebug() << Q_FUNC_INFO << "!ok for firstIndex, emoteIndexes[0] =" << emoteIndexes[0] << ", emotesPartInfo =" << emotesPartInfo;
                            continue;
                        }

                        ok = false;
                        const int lastIndex = emoteIndexes[1].toInt(&ok);
                        if (!ok)
                        {
                            qDebug() << Q_FUNC_INFO << "!ok for firstIndex, emoteIndexes[1] =" << emoteIndexes[1] << ", emotesPartInfo =" << emotesPartInfo;
                            continue;
                        }

                        emoteInfo.indexes.append(QPair<int, int>(firstIndex, lastIndex));
                    }

                    if (emoteInfo.isValid())
                    {
                        emotesInfo.append(emoteInfo);
                    }
                    else
                    {
                        qDebug() << Q_FUNC_INFO << "emoteInfo not valid, emotesPartInfo =" << emotesPartInfo;
                    }
                }
            }
            else if (tagName == "id") // id of message
            {
                //
            }
            else if (tagName == "user-id")
            {
                //ToDo
            }
            else if (tagName == "badge-info")
            {
                //ToDo
            }
            else if (tagName == "badges")
            {
                const QVector<QStringRef> badgesInfo = tagValue.splitRef(',', Qt::SplitBehaviorFlags::SkipEmptyParts);
                for (const QStringRef& badgeInfo : badgesInfo)
                {
                    const QString badgeInfoStr = badgeInfo.toString();
                    if (_badgesUrls.contains(badgeInfoStr))
                    {
                        badges.insert(badgeInfoStr, _badgesUrls[badgeInfoStr]);
                    }
                    else
                    {
                        qWarning() << Q_FUNC_INFO << "unknown badge" << badgeInfoStr;
                        badges.insert(badgeInfoStr, "qrc:/resources/images/unknown-badge.png");
                    }
                }
            }
            else if (tagName == "mod")
            {
                //ToDo
            }
            else if (tagName == "turbo")
            {
                //ToDo
            }
            else if (tagName == "subscriber")
            {

            }
        }

        QUrl avatar;
        if (avatarsUrls.contains(channelLogin))
        {
            avatar = avatarsUrls[channelLogin];
        }
        else
        {
            //requestForAvatarsByChannelPage(channelLogin);
            requestUserInfo(channelLogin);
        }

        if (displayName.isEmpty())
        {
            displayName = channelLogin;
        }

        const MessageAuthor author = MessageAuthor::createFromTwitch(displayName, channelLogin, avatar, nicknameColor, badges);

        QString messageText;
        if (emotesInfo.isEmpty())
        {
            messageText = rawMessageText;
        }
        else
        {
            for (int i = 0; i < rawMessageText.length(); ++i)
            {
                bool needIgnoreChar = false;

                for (const MessageEmoteInfo& emoteInfo : emotesInfo)
                {
                    for (const QPair<int, int> indexPair : emoteInfo.indexes)
                    {
                        if (i >= indexPair.first && i <= indexPair.second)
                        {
                            needIgnoreChar = true;
                        }

                        if (i == indexPair.first)
                        {
                            static const QString sizeUrlPart = "1.0";
                            const QString emoteUrl = QString("https://static-cdn.jtvnw.net/emoticons/v2/%1/default/light/%2").arg(emoteInfo.id, sizeUrlPart);

                            messageText += QString("<img align=\"top\" src=\"%1\">").arg(emoteUrl);
                        }
                    }
                }

                if (!needIgnoreChar)
                {
                    messageText += rawMessageText[i];
                }
            }
        }

        const ChatMessage message = ChatMessage::createFromTwitch(messageText, QDateTime::currentDateTime(), author, flags);
        messages.append(message);

        //qDebug() << "Twitch:" << authorName << ":" << messageText;
    }

    if (!messages.isEmpty())
    {
        emit readyRead(messages);
    }
}

void Twitch::requestForAvatarsByChannelPage(const QString &channelLogin)
{
    QNetworkRequest request(QString("https://www.twitch.tv/%1").arg(channelLogin));
    request.setRawHeader("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8");
    //request.setRawHeader("Accept-Encoding", "gzip, deflate, br");
    //request.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader, AxelChat::UserAgentNetworkHeaderName);
    request.setRawHeader("Accept-Language", AcceptLanguageNetworkHeaderName);
    QNetworkReply* reply = _manager.get(request);
    if (!reply)
    {
        qDebug() << Q_FUNC_INFO << ": !reply";
        return;
    }

    repliesForAvatar.insert(reply, channelLogin);

    QObject::connect(reply, &QNetworkReply::finished, this, &Twitch::onReplyAvatarsByChannelPage);
}

void Twitch::onReplyAvatarsByChannelPage()
{
    //ToDo: https://www.teamfortress.tv/3580/show-twitch-profile-pictures-in-all-streams-list
    //ToDo: 600x600, 300x300, 150x150, 70x70, 50x50, 28x28
    QNetworkReply* reply = dynamic_cast<QNetworkReply*>(sender());
    QByteArray data;
    if (!checkReply(reply, Q_FUNC_INFO, data))
    {
        return;
    }

    const QString channelLogin = repliesForAvatar[reply];
    repliesForAvatar.remove(reply);

    static const QList<QByteArray> AvatarUrlPrefixes = {
        "https://static-cdn.jtvnw.net/jtv_user_pictures/",
        "https://static-cdn.jtvnw.net/user-default-pictures-uv/",
    };

    int startPos = -1;
    for (const QByteArray& prefix : AvatarUrlPrefixes)
    {
        startPos = data.indexOf(prefix);
        if (startPos != -1)
        {
            break;
        }
    }

    if (startPos == -1)
    {
        qWarning() << Q_FUNC_INFO << "failed to find prefix" << AvatarUrlPrefixes << "of avatar link, request url:" << reply->url();
        AxelChat::saveDebugDataToFile(FolderLogs, "no_prefix_avatar_url.html", data);
        return;
    }

    static const QByteArray AvatarUrlPostfix = "\"";
    const int postfixPos = data.indexOf(AvatarUrlPostfix, startPos);
    if (postfixPos == -1 || postfixPos - startPos > 200)
    {
        qWarning() << Q_FUNC_INFO << "failed to find postfix of avatar link";
        AxelChat::saveDebugDataToFile(FolderLogs, "no_postfix_avatar_url.html", data);
        return;
    }

    const QByteArray rawUrl = data.mid(startPos, postfixPos - startPos);
    if (rawUrl.isEmpty())
    {
        qWarning() << Q_FUNC_INFO << "empty avatar url";
        AxelChat::saveDebugDataToFile(FolderLogs, "empty_avatar_url.html", data);
        return;
    }

    const QUrl url(QString::fromUtf8(rawUrl));
    if (!url.isValid())
    {
        qWarning() << Q_FUNC_INFO << "invalid avatar url";
        AxelChat::saveDebugDataToFile(FolderLogs, "invalid_avatar_url.html", data);
        return;
    }

    avatarsUrls.insert(channelLogin, url);

    emit avatarDiscovered(channelLogin, url);
}

void Twitch::requestForGlobalBadges()
{
    QNetworkRequest request(QString("https://badges.twitch.tv/v1/badges/global/display"));
    request.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader, AxelChat::UserAgentNetworkHeaderName);
    request.setRawHeader("Accept-Language", AcceptLanguageNetworkHeaderName);
    QNetworkReply* reply = _manager.get(request);
    if (!reply)
    {
        qDebug() << Q_FUNC_INFO << ": !reply";
        return;
    }

    QObject::connect(reply, &QNetworkReply::finished, this, &Twitch::onReplyBadges);
}

void Twitch::requestForChannelBadges(const QString &broadcasterId)
{
    QNetworkRequest request(QString("https://badges.twitch.tv/v1/badges/channels/%1/display").arg(broadcasterId));
    request.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader, AxelChat::UserAgentNetworkHeaderName);
    request.setRawHeader("Accept-Language", AcceptLanguageNetworkHeaderName);
    QNetworkReply* reply = _manager.get(request);
    if (!reply)
    {
        qDebug() << Q_FUNC_INFO << ": !reply";
        return;
    }

    QObject::connect(reply, &QNetworkReply::finished, this, &Twitch::onReplyBadges);
}

void Twitch::onReplyBadges()
{
    QNetworkReply* reply = dynamic_cast<QNetworkReply*>(sender());
    QByteArray data;
    if (!checkReply(reply, Q_FUNC_INFO, data))
    {
        return;
    }

    parseBadgesJson(data);
}

void Twitch::parseBadgesJson(const QByteArray &data)
{
    if (data.isEmpty())
    {
        return;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(data);
    const QJsonObject badgeSetsObj = doc.object().value("badge_sets").toObject();

    const QStringList badgesNames = badgeSetsObj.keys();
    for (const QString& badgeName : badgesNames)
    {
        const QJsonObject versionsObj = badgeSetsObj.value(badgeName).toObject().value("versions").toObject();
        const QStringList versions = versionsObj.keys();
        for (const QString& version : versions)
        {
            const QString url = versionsObj.value(version).toObject().value("image_url_1x").toString();
            if (!url.isEmpty())
            {
                _badgesUrls.insert(badgeName + "/" + version, url);
            }
        }
    }
}

void Twitch::requestUserInfo(const QString& login)
{
    QNetworkRequest request(QString("https://api.twitch.tv/helix/users?login=%1").arg(login));
    request.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader, AxelChat::UserAgentNetworkHeaderName);
    request.setRawHeader("Accept-Language", AcceptLanguageNetworkHeaderName);
    request.setRawHeader("Client-ID", ApplicationClientID.toUtf8());
    request.setRawHeader("Authorization", QByteArray("Bearer ") + _info.oauthToken.toUtf8());
    QNetworkReply* reply = _manager.get(request);
    if (!reply)
    {
        qDebug() << Q_FUNC_INFO << ": !reply";
        return;
    }

    QObject::connect(reply, &QNetworkReply::finished, this, &Twitch::onReplyUserInfo);
}

void Twitch::onReplyUserInfo()
{
    QNetworkReply* reply = dynamic_cast<QNetworkReply*>(sender());
    QByteArray data;
    if (!checkReply(reply, Q_FUNC_INFO, data))
    {
        return;
    }

    const QJsonArray dataArr = QJsonDocument::fromJson(data).object().value("data").toArray();
    for (const QJsonValue& v : qAsConst(dataArr))
    {
        const QJsonObject vObj = v.toObject();

        const QString broadcasterId = vObj.value("id").toString();
        const QString channelLogin = vObj.value("login").toString();
        const QUrl profileImageUrl = QUrl(vObj.value("profile_image_url").toString());

        if (channelLogin.isEmpty())
        {
            continue;
        }

        if (!profileImageUrl.isEmpty())
        {
            avatarsUrls.insert(channelLogin, profileImageUrl);
            emit avatarDiscovered(channelLogin, profileImageUrl);
        }

        if (channelLogin == _info.channelLogin)
        {
            _info.broadcasterId = broadcasterId;
            requestForChannelBadges(broadcasterId);
        }
    }
}

void Twitch::requestStreamInfo(const QString &login)
{
    QNetworkRequest request(QString("https://api.twitch.tv/helix/streams?user_login=%1").arg(login));
    request.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader, AxelChat::UserAgentNetworkHeaderName);
    request.setRawHeader("Accept-Language", AcceptLanguageNetworkHeaderName);
    request.setRawHeader("Client-ID", ApplicationClientID.toUtf8());
    request.setRawHeader("Authorization", QByteArray("Bearer ") + _info.oauthToken.toUtf8());
    QNetworkReply* reply = _manager.get(request);
    if (!reply)
    {
        qDebug() << Q_FUNC_INFO << ": !reply";
        return;
    }

    QObject::connect(reply, &QNetworkReply::finished, this, &Twitch::onReplyStreamInfo);
}

void Twitch::onReplyStreamInfo()
{
    QNetworkReply* reply = dynamic_cast<QNetworkReply*>(sender());
    QByteArray data;
    if (!checkReply(reply, Q_FUNC_INFO, data))
    {
        return;
    }

    bool found = false;

    const QJsonArray dataArr = QJsonDocument::fromJson(data).object().value("data").toArray();
    for (const QJsonValue& v : qAsConst(dataArr))
    {
        const QJsonObject vObj = v.toObject();

        const QString channelLogin = vObj.value("user_login").toString();
        const int viewers = vObj.value("viewer_count").toInt();

        if (channelLogin.isEmpty())
        {
            continue;
        }

        if (channelLogin == _info.channelLogin)
        {
            _info.viewers = viewers;
            found = true;
            emit stateChanged();
        }
    }

    if (!found)
    {
        _info.viewers = -1;
        emit stateChanged();
    }
}

