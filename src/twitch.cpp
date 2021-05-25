#include "twitch.hpp"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFile>
#include <QDesktopServices>
#include <QAbstractSocket>

namespace
{

static const QString ClientID = "cx5rgsivc62io2kk79yf6eivhhwiui";
static const QString RedirectUri = "https://localhost";
static const QString IRCHost = "tmi.twitch.tv";

}

Twitch::Twitch(QSettings* settings, const QString& settingsGroupPath, QObject *parent)
  : AbstractChatService(parent)
  , _settings(settings)
  , _settingsGroupPath(settingsGroupPath)
{
    QObject::connect(&_manager, &QNetworkAccessManager::finished, this, &Twitch::onReply);

    QObject::connect(&_socket, &QWebSocket::stateChanged, this, [=](QAbstractSocket::SocketState state){
        qDebug() << "QWebSocket::stateChanged:" << state;
    });

    QObject::connect(&_socket, &QWebSocket::textMessageReceived, this, &Twitch::onIRCMessage);

    QObject::connect(&_socket, &QWebSocket::connected, this, [=](){
        qDebug() << "WebSocket connected";

        _socket.sendTextMessage(QString("PASS oauth:") + _oauthToken);
        _socket.sendTextMessage(QString("NICK ") + _nick);
        _socket.sendTextMessage(QString("JOIN #") + _channelToConnect);
        _socket.sendTextMessage(QString("PING :") + IRCHost);

        _isConnected = true;

        emit connected(_channelToConnect);
        emit connectedChanged();
    });

    QObject::connect(&_socket, &QWebSocket::disconnected, this, [=](){
        qDebug() << "WebSocket disconnected";
        _isConnected = false;

        emit disconnected(_channelToConnect);
        emit connectedChanged();
    });

    QObject::connect(&_socket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), this, [=](QAbstractSocket::SocketError error_){
        qDebug() << "WebSocket error:" << error_;
    });

    reInitSocket();
}

bool Twitch::isConnected() const
{
    return _isConnected;
}

void Twitch::setOAuthToken(QString token)
{
    _oauthToken = token;
}

void Twitch::setNickOrLink(QString nickOrLink)
{
    _nick = nickOrLink;
}

void Twitch::setChannelToConnect(QString channelNickOrLink)
{
    _channelToConnect = channelNickOrLink;
}

void Twitch::onReply(QNetworkReply *reply)
{
    if (!reply)
    {
        return;
    }

    for (const auto& raw : reply->rawHeaderPairs())
    {
        qDebug () << raw.first << "=" << raw.second;
    }

    const QByteArray ba = reply->readAll();

    qDebug() << "result =" << ba;

}

void Twitch::requestAuthorization()
{
    //https://id.twitch.tv/oauth2/authorize?response_type=token&client_id=uo6dggojyb8d6soh92zknwmi5ej1q2&redirect_uri=http://localhost&scope=viewing_activity_read&state=c3ab8aa609ea11e793ae92361f002671

    QNetworkRequest request(QUrl("https://id.twitch.tv/oauth2/authorize?client_id=" + ClientID
                                 + "&redirect_uri=" + RedirectUri
                                 + "&response_type=token+id_token"
                                 + "&scope=openid+channel:read:polls"));

    QNetworkReply* reply = _manager.get(request);
    if (!reply)
    {
        qDebug() << Q_FUNC_INFO << ": !reply";
        return;
    }

    QObject::connect(reply, &QNetworkReply::finished, this, [=]() {
        QObject* sender_ = sender();
        if (!sender_)
        {
            qDebug() << Q_FUNC_INFO << ": !sender_";
            return;
        }

        QNetworkReply* reply = dynamic_cast<QNetworkReply*>(sender_);
        if (!reply)
        {
            qDebug() << Q_FUNC_INFO << ": !reply";
            return;
        }

        QByteArray ba = reply->readAll();

        /*QFile file("twitch.html");
        if (file.open(QFile::OpenModeFlag::WriteOnly | QFile::OpenModeFlag::Text))
        {
            file.write(ba);
            file.close();
            qDebug() << "Saved to" << file.fileName();
        }*/

        ba = ba.trimmed();

        if (ba.startsWith("<a href=\"") && ba.endsWith("\">Found</a>."))
        {
            ba = ba.remove(0, 9);
            ba = ba.remove(ba.length() - 12, ba.length());
            //qDebug() << ba;
            QDesktopServices::openUrl(QUrl(ba));
        }
    });
}

void Twitch::reInitSocket()
{
    _socket.close();
    _socket.open(QUrl("ws://irc-ws.chat.twitch.tv:80"));//ToDo: use SSL? wss://irc-ws.chat.twitch.tv:443
}

void Twitch::onIRCMessage(const QString &rawData)
{
    QList<ChatMessage> messages;
    QList<MessageAuthor> authors;

    const QVector<QStringRef> rawMessages = rawData.splitRef("\r\n");
    for (QStringRef raw : rawMessages)
    {
        QString rawMessage = raw.trimmed().toString();

        if (rawMessage.isEmpty())
        {
            continue;
        }

        qDebug(rawMessage.toUtf8());

        if (rawMessage.toUpper().startsWith("PING"))
        {
            _socket.sendTextMessage(QString("PONG :") + IRCHost);
        }

        if (rawMessage.startsWith(":" + IRCHost))
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

        const MessageAuthor author = MessageAuthor::createFromTwitch(authorName, authorName);
        authors.append(author);

        const ChatMessage message = ChatMessage::createFromTwitch(messageText, QDateTime::currentDateTime(), author);
        messages.append(message);

        //qDebug() << authorName << ":" << messageText;

        //:ryknowe1!ryknowe1@ryknowe1.tmi.twitch.tv PRIVMSG #arcus :haven't seen it recently
    }

    if (!messages.isEmpty())
    {
        emit readyRead(messages, authors);
    }
}
