#include "twitch.hpp"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFile>
#include <QDesktopServices>
#include <QAbstractSocket>

namespace
{

static const QString ApplicationClientID = "cx5rgsivc62io2kk79yf6eivhhwiui";
static const QString RedirectUri = "https://twitchapps.com/tmi/";//"https://localhost";
static const QString TwitchIRCHost = "tmi.twitch.tv";

static const QString SettingsKeyOAuthToken = "oauth_token";
static const QString SettingsKeyUserSpecifiedChannel = "user_specified_channel";

}

Twitch::Twitch(QSettings* settings, const QString& settingsGroupPath, QObject *parent)
  : AbstractChatService(parent)
  , _settings(settings)
  , _settingsGroupPath(settingsGroupPath)
{
    QObject::connect(&_socket, &QWebSocket::stateChanged, this, [=](QAbstractSocket::SocketState state){
        //qDebug() << "Twitch WebSocket state changed:" << state;
    });

    QObject::connect(&_socket, &QWebSocket::textMessageReceived, this, &Twitch::onIRCMessage);

    QObject::connect(&_socket, &QWebSocket::connected, this, [=]() {
        qDebug() << "Twitch WebSocket connected" << _info.channelName;

        if (_info.connected)
        {
            _info.connected = false;
            emit stateChanged();
        }

        _socket.sendTextMessage(QString("PASS oauth:") + _info.oauthToken);
        _socket.sendTextMessage(QString("NICK ") + _info.channelName);
        _socket.sendTextMessage(QString("JOIN #") + _info.channelName);
        _socket.sendTextMessage(QString("PING :") + TwitchIRCHost);
    });

    QObject::connect(&_socket, &QWebSocket::disconnected, this, [=](){
        qDebug() << "Twitch WebSocket disconnected";

        if (_info.connected)
        {
            _info.connected = false;
            emit disconnected(_info.channelName);
            emit stateChanged();
        }
    });

    QObject::connect(&_socket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), this, [=](QAbstractSocket::SocketError error_){
        //qDebug() << "Twitch WebSocket error:" << error_;
    });

    reInitSocket();

    QObject::connect(&_timerReconnect, &QTimer::timeout, this, &Twitch::timeoutReconnect);
    _timerReconnect.start(2000);

    if (_settings)
    {
        const QString token = QString::fromUtf8(QByteArray::fromBase64(_settings->value(_settingsGroupPath + "/" + SettingsKeyOAuthToken).toByteArray()));
        setOAuthToken(token);

        setUserSpecifiedChannel(_settings->value(_settingsGroupPath + "/" + SettingsKeyUserSpecifiedChannel).toString());
    }
}

Twitch::~Twitch()
{
    _socket.close();
}

AbstractChatService::ConnectionStateType Twitch::connectionStateType() const
{
    if (_info.connected)
    {
        return AbstractChatService::ConnectionStateType::Connected;
    }
    else if (!_info.oauthToken.isEmpty() && !_info.channelName.isEmpty())
    {
        return AbstractChatService::ConnectionStateType::Connecting;
    }

    return AbstractChatService::ConnectionStateType::NotConnected;
}

QString Twitch::stateDescription() const
{
    switch (connectionStateType()) {
    case ConnectionStateType::NotConnected:
        if (_info.channelName.isEmpty())
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

QUrl Twitch::requesGetAOuthTokenUrl() const
{
    return QUrl("https://id.twitch.tv/oauth2/authorize?client_id=" + ApplicationClientID
                        + "&redirect_uri=" + RedirectUri
                        + "&response_type=token"
                        + "&scope=openid+chat:read");
}

QUrl Twitch::chatUrl() const
{
    if (_info.channelName.isEmpty())
    {
        return QUrl();
    }

    return QUrl(QString("https://www.twitch.tv/popout/%1/chat").arg(_info.channelName));
}

QUrl Twitch::controlPanelUrl() const
{
    if (_info.channelName.isEmpty())
    {
        return QUrl();
    }

    return QUrl(QString("https://dashboard.twitch.tv/u/%1/stream-manager").arg(_info.channelName));
}

QUrl Twitch::broadcastUrl() const
{
    if (_info.channelName.isEmpty())
    {
        return QUrl();
    }

    return QUrl(QString("https://www.twitch.tv/%1").arg(_info.channelName));
}

bool Twitch::isChannelNameUserSpecified() const
{
    return _info.userSpecifiedChannel.trimmed() == _info.channelName.trimmed() && !_info.userSpecifiedChannel.isEmpty();
}

void Twitch::setUserSpecifiedChannel(QString userChannel)
{
    userChannel = userChannel.trimmed();

    if (_info.userSpecifiedChannel != userChannel)
    {
        _info.userSpecifiedChannel = userChannel;

        if (_settings)
        {
            _settings->setValue(_settingsGroupPath + "/" + SettingsKeyUserSpecifiedChannel, _info.userSpecifiedChannel);
        }

        reInitSocket();

        emit linkChanged();
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

        if (_settings)
        {
            _settings->setValue(_settingsGroupPath + "/" + SettingsKeyOAuthToken, _info.oauthToken.toUtf8().toBase64());
        }

        reInitSocket();

        emit linkChanged();
    }
}

void Twitch::reInitSocket()
{
    _info.userSpecifiedChannel = _info.userSpecifiedChannel.trimmed();

    QRegExp rx;

    // user channel
    _info.channelName.clear();
    const QString simpleUserSpecifiedUserChannel = simplifyUrl(_info.userSpecifiedChannel);
    rx = QRegExp("^twitch.tv/([^/]*)$", Qt::CaseInsensitive);
    if (rx.indexIn(simpleUserSpecifiedUserChannel) != -1)
    {
        _info.channelName = rx.cap(1);
    }

    if (_info.channelName.isEmpty())
    {
        rx = QRegExp("^[a-zA-Z0-9_]+$", Qt::CaseInsensitive);
        if (rx.indexIn(_info.userSpecifiedChannel) != -1)
        {
            _info.channelName = _info.userSpecifiedChannel;
        }
    }

    _socket.close();

    if (_info.connected)
    {
        _info.connected = false;
        emit stateChanged();
    }

    if (!_info.channelName.isEmpty())
    {
        // ToDo: use SSL? wss://irc-ws.chat.twitch.tv:443
        _socket.open(QUrl("ws://irc-ws.chat.twitch.tv:80"));
    }
}

void Twitch::onIRCMessage(const QString &rawData)
{
    QList<ChatMessage> messages;
    QList<MessageAuthor> authors;

    const QVector<QStringRef> rawMessages = rawData.splitRef("\r\n");
    for (const QStringRef& raw : rawMessages)
    {
        QString rawMessage = raw.trimmed().toString();

        if (rawMessage.isEmpty())
        {
            continue;
        }

        //qDebug(rawMessage.toUtf8());

        if (rawMessage.startsWith("PING", Qt::CaseSensitivity::CaseInsensitive))
        {
            _socket.sendTextMessage(QString("PONG :") + TwitchIRCHost);
        }

        if (!_info.connected && rawMessage.startsWith(':') && rawMessage.count(':') == 1 && rawMessage.contains("JOIN #", Qt::CaseSensitivity::CaseInsensitive))
        {
            _info.connected = true;
            emit connected(_info.channelName);
            emit stateChanged();
        }

        if (rawMessage.startsWith(":" + TwitchIRCHost))
        {
            // service messages should not be displayed
            continue;
        }

        rawMessage = rawMessage.trimmed();
        if (!rawMessage.startsWith(":"))
        {
            continue;
        }

        rawMessage = rawMessage.mid(1);

        if (!rawMessage.contains("!"))
        {
            continue;
        }

        if (!rawMessage.contains(':'))
        {
            continue;
        }

        const QString authorName = rawMessage.left(rawMessage.indexOf('!'));
        const QString messageText = rawMessage.mid(rawMessage.indexOf(':') + 1);

        if (authorName.isEmpty() || messageText.isEmpty())
        {
            continue;
        }

        //:ryknowe1!ryknowe1@ryknowe1.tmi.twitch.tv PRIVMSG #arcus :haven't seen it recently

        const MessageAuthor author = MessageAuthor::createFromTwitch(authorName, authorName);
        authors.append(author);

        const ChatMessage message = ChatMessage::createFromTwitch(messageText, QDateTime::currentDateTime(), author);
        messages.append(message);

        //qDebug() << authorName << ":" << messageText;
    }

    if (!messages.isEmpty())
    {
        emit readyRead(messages, authors);
    }
}

void Twitch::timeoutReconnect()
{
    if (!_info.connected && !_info.oauthToken.isEmpty())
    {
        reInitSocket();
    }
}
