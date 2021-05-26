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

    QObject::connect(&_socket, &QWebSocket::connected, this, [=](){
        qDebug() << "Twitch WebSocket connected" << _info.channelName;

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
            emit connectedChanged();
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

void Twitch::extractOAuthTokenFromUrl(const QUrl &url)
{
    //https://localhost/#access_token=8esldyawj3599z5bhqy7o742xhbfvx&id_token=eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCIsImtpZCI6IjEifQ.eyJhdWQiOiJjeDVyZ3NpdmM2MmlvMmtrNzl5ZjZlaXZoaHdpdWkiLCJleHAiOjE2MjE5OTM5NTcsImlhdCI6MTYyMTk5MzA1NywiaXNzIjoiaHR0cHM6Ly9pZC50d2l0Y2gudHYvb2F1dGgyIiwic3ViIjoiMjE1NjAxNjgyIiwiYXRfaGFzaCI6Il8taDUtLVpFUFJkcmFZc2pWZUZFV3ciLCJhenAiOiJjeDVyZ3NpdmM2MmlvMmtrNzl5ZjZlaXZoaHdpdWkiLCJwcmVmZXJyZWRfdXNlcm5hbWUiOiJBeGUxX2sifQ.QwYuUV0KoFeK7P5hq5bEWj4l7rk8iHXm8vBTKIxFh6DAGn93boszdo0syAdU983qB7p4rifaiuV9iTU_gGf4ywG1nbk0DWenIHBTxqSBpBubhc77CWRmFSDe94FVWtep5Me8QBjvwb2HJoVHugWZBJ8NLDxQUbZOzSNzq3rCtooZFRKc3xBenN5mukgsG1sTXFqJOcgKlYMgJBJwuga6d-skHEYYekyjuudEAWli9llIHvcI-6AGcoMg5upcxOMTkDGYGEXHXjZH0dRI6cNHpBN6F93Ffhut5LhEQZhKAaGI32sV8MY_mP4ag-faqoS6WgrHtg1PURI18h9WVNk45A&scope=openid+chat%253Aread&token_type=bearer
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
        emit connectedChanged();
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

        qDebug(rawMessage.toUtf8());

        if (rawMessage.startsWith("PING", Qt::CaseSensitivity::CaseInsensitive))
        {
            _socket.sendTextMessage(QString("PONG :") + TwitchIRCHost);
        }

        if (!_info.connected && rawMessage.startsWith(':') && rawMessage.count(':') == 1 && rawMessage.contains("JOIN #", Qt::CaseSensitivity::CaseInsensitive))
        {
            _info.connected = true;
            emit connected(_info.channelName);
            emit connectedChanged();
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
