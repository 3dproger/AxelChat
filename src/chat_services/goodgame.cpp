#include "goodgame.h"
#include "models/messagesmodel.h"
#include "models/message.h"
#include "models/author.h"
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QNetworkAccessManager>
#include <QNetworkReply>

namespace
{

static const int RequestChatInterval = 2000;
static const int RequestChannelStatus = 10000;
static const int ReconncectPeriod = 3 * 1000;

static const QMap<QString, QColor> ColorTypes =
{
    {"gold",                QColor(238, 252, 8)},
    {"silver",              QColor(180, 180, 180)},
    {"bronze",              QColor(231, 130, 10)},
    {"king",                QColor(48,  213, 200)},
    {"premium",             QColor(189, 112, 215)},
    {"premium-personal",    QColor(49,  169, 58)},
    {"diamond",             QColor(135, 129, 189)},
    {"moderator",           QColor(236, 64,  88)},
    {"streamer",            QColor(232, 187, 0)},
    {"streamer-helper",     QColor(232, 187, 0)},
    {"undead",              QColor(171, 72,  115)},
    {"top-one",             QColor(59,  203, 255)},
    {"newguy",              QColor(255, 255, 255)},
};

static const QMap<QString, QString> IconTypesExtra =
{
    {"ggplus",                  ":/resources/images/goodgame/star-one.svg"},
};

}

GoodGame::GoodGame(QSettings& settings_, const QString& settingsGroupPath, QNetworkAccessManager& network_, QObject *parent)
    : ChatService(settings_, settingsGroupPath, AxelChat::ServiceType::GoodGame, parent)
    , settings(settings_)
    , network(network_)
{
    getParameter(stream)->setPlaceholder(tr("Link or channel name..."));

    QObject::connect(&socket, &QWebSocket::stateChanged, this, [](QAbstractSocket::SocketState state)
    {
        Q_UNUSED(state)
        //qDebug() << Q_FUNC_INFO << ": WebSocket state changed:" << state;
    });

    QObject::connect(&socket, &QWebSocket::textMessageReceived, this, &GoodGame::onWebSocketReceived);

    QObject::connect(&socket, &QWebSocket::connected, this, [this]()
    {
        if (state.connected)
        {
            state.connected = false;
            emit connectedChanged(false, lastConnectedChannelName);
        }

        requestChannelStatus();
        emit stateChanged();
    });

    QObject::connect(&socket, &QWebSocket::disconnected, this, [this]()
    {
        if (state.connected)
        {
            state.connected = false;
            emit stateChanged();
            emit connectedChanged(false, lastConnectedChannelName);
        }
    });

    QObject::connect(&socket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), this, [this](QAbstractSocket::SocketError error_)
    {
        qDebug() << Q_FUNC_INFO << ": WebSocket error:" << error_ << ":" << socket.errorString();
    });

    timerUpdateMessages.setInterval(RequestChatInterval);
    connect(&timerUpdateMessages, &QTimer::timeout, this, [this]()
    {
        if (!state.connected)
        {
            return;
        }

        requestChannelHistory();
    });
    timerUpdateMessages.start();

    timerUpdateChannelStatus.setInterval(RequestChannelStatus);
    connect(&timerUpdateChannelStatus, &QTimer::timeout, this, [this]()
    {
        if (!state.connected)
        {
            return;
        }

        requestChannelStatus();
    });
    timerUpdateChannelStatus.start();

    QObject::connect(&timerReconnect, &QTimer::timeout, this, [this]()
    {
        if (socket.state() == QAbstractSocket::SocketState::UnconnectedState)
        {
            reconnect();
        }
    });
    timerReconnect.start(ReconncectPeriod);

    reconnect();
}

ChatService::ConnectionStateType GoodGame::getConnectionStateType() const
{
    if (state.connected)
    {
        return ChatService::ConnectionStateType::Connected;
    }
    else if (enabled.get() && !state.streamId.isEmpty())
    {
        return ChatService::ConnectionStateType::Connecting;
    }

    return ChatService::ConnectionStateType::NotConnected;
}

QString GoodGame::getStateDescription() const
{
    switch (getConnectionStateType())
    {
    case ConnectionStateType::NotConnected:
        if (state.streamId.isEmpty())
        {
            return tr("Channel not specified");
        }

        return tr("Not connected");

    case ConnectionStateType::Connecting:
        return tr("Connecting...");

    case ConnectionStateType::Connected:
        return tr("Successfully connected!");

    }

    return "<unknown_state>";
}

void GoodGame::requestAuth()
{
    QJsonDocument document;

    QJsonObject root;
    root.insert("type", "auth");

    QJsonObject data;
    data.insert("user_id", "0");

    root.insert("data", data);
    document.setObject(root);
    sendToWebSocket(document);
}

void GoodGame::requestChannelHistory()
{
    QJsonDocument document;

    QJsonObject root;
    root.insert("type", "get_channel_history");

    QJsonObject data;
    data.insert("channel_id", QString("%1").arg(channelId));

    root.insert("data", data);
    document.setObject(root);
    sendToWebSocket(document);
}

void GoodGame::requestChannelStatus()
{
    const QString channelName = state.streamId.trimmed().toLower();

    QNetworkRequest request(QUrl("https://goodgame.ru/api/4/stream/" + channelName));
    QNetworkReply* reply = network.get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]()
    {
        const QJsonObject root = QJsonDocument::fromJson(reply->readAll()).object();
        state.viewersCount = root.value("viewers").toInt(-1);

        channelId = root.value("id").toInt(-1);

        emit stateChanged();
        requestChannelHistory();

        reply->deleteLater();
    });
}

void GoodGame::requestUserPage(const QString &authorName, const QString &authorId)
{
    QNetworkRequest request(QUrl("https://goodgame.ru/user/" + authorId));
    request.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader, AxelChat::UserAgentNetworkHeaderName);
    QNetworkReply* reply = network.get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, authorName, authorId]()
    {
        const QByteArray data = reply->readAll();
        reply->deleteLater();

        static const QByteArray urlPrefix = "https://goodgame.ru/files/avatars";

        const int startAvatarPos = data.indexOf(urlPrefix);
        if (startAvatarPos == -1)
        {
            qCritical() << Q_FUNC_INFO << ": not found avatar url, author name" << authorName;
            return;
        }

        QString url;
        for (int i = startAvatarPos + urlPrefix.length(); i < qMin(startAvatarPos + 1024, data.length()); ++i)
        {
            const char c = data[i];
            if (c == '"')
            {
                break;
            }

            url += c;
        }

        if (!url.isEmpty())
        {
            url = urlPrefix + url;

            emit authorDataUpdated(authorId, { {Author::Role::AvatarUrl, QUrl(url)} });
        }
    });
}

void GoodGame::requestSmiles()
{
    QNetworkRequest request(QUrl("https://goodgame.ru/api/4/smiles"));
    QNetworkReply* reply = network.get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]()
    {
        const QJsonArray array = QJsonDocument::fromJson(reply->readAll()).array();
        reply->deleteLater();

        for (const QJsonValue& v : array)
        {
            const QJsonObject smile = v.toObject();
            const QString smileName = smile.value("key").toString().trimmed().toLower();
            const QUrl smileUrl = QUrl(smile.value("images").toObject().value("big").toString());

            if (smileName.isEmpty() || smileUrl.isEmpty())
            {
                qWarning() << Q_FUNC_INFO << "smile name or url is empty";
                continue;
            }

            smiles.insert(smileName, smileUrl);
        }
    });
}

QString GoodGame::getStreamId(const QString &stream)
{
    QString streamId = stream.trimmed().toLower();
    if (streamId.contains("goodgame.ru"))
    {
        streamId = AxelChat::simplifyUrl(streamId);

        bool ok = false;
        streamId = AxelChat::removeFromStart(streamId, "goodgame.ru/channel/", Qt::CaseSensitivity::CaseInsensitive, &ok);

        if (!ok)
        {
            return QString();
        }

        if (streamId.contains('#'))
        {
            streamId = streamId.left(streamId.indexOf('#'));
        }

        streamId.remove('/');
    }

    return streamId;
}

void GoodGame::reconnectImpl()
{
    if (smiles.isEmpty())
    {
        requestSmiles();
        return;
    }

    socket.close();

    state = State();

    state.streamId = getStreamId(stream.get());

    if (state.streamId.isEmpty())
    {
        emit stateChanged();
        return;
    }

    state.streamUrl = "https://goodgame.ru/channel/" + state.streamId;
    state.chatUrl = "https://goodgame.ru/chat/" + state.streamId;

    if (enabled.get())
    {
        socket.setProxy(network.proxy());
        socket.open(QUrl("wss://chat-1.goodgame.ru/chat2/"));
    }
}

void GoodGame::onWebSocketReceived(const QString &rawData)
{
    //qDebug(rawData.toUtf8());

    if (!enabled.get())
    {
        return;
    }

    const QJsonDocument document = QJsonDocument::fromJson(rawData.toUtf8());
    const QJsonObject root = document.object();
    const QJsonObject data = root.value("data").toObject();
    const QString type = root.value("type").toString();
    const QString channelId_ = data.value("channel_id").toString();

    if (type == "channel_history")
    {
        if (!state.connected)
        {
            state.connected = true;
            lastConnectedChannelName = state.streamId;
            emit connectedChanged(true, state.streamId);
            emit stateChanged();
        }

        QList<Message> messages;
        QList<Author> authors;

        const QJsonArray jsonMessages = data.value("messages").toArray();
        for (const QJsonValue& value : qAsConst(jsonMessages))
        {
            const QJsonObject jsonMessage = value.toObject();

            const QString authorId = QString("%1").arg(jsonMessage.value("user_id").toVariant().toLongLong());
            const QString authorName = jsonMessage.value("user_name").toString();
            const QString messageId = QString("%1").arg(jsonMessage.value("message_id").toVariant().toLongLong());
            const QDateTime publishedAt = QDateTime::fromSecsSinceEpoch(jsonMessage.value("timestamp").toVariant().toLongLong());
            const QString rawText = jsonMessage.value("text").toString();
            const QString colorType = jsonMessage.value("color").toString();
            const QString iconType = jsonMessage.value("icon").toString();
            const int mobile = jsonMessage.value("mobile").toInt();

            QColor nicknameColor;

            if (colorType == "" || colorType == "simple")
            {
                // TODO: may be different: QColor(255, 255, 255), QColor(115, 173, 255)
                nicknameColor = QColor();
            }
            else if (ColorTypes.contains(colorType))
            {
                nicknameColor = ColorTypes.value(colorType);
            }
            else
            {
                qWarning() << Q_FUNC_INFO << ": unknown color type" << colorType << ", author name =" << authorName;
            }

            QString badge;

            if (iconType.isEmpty() || iconType == "none")
            {
                if (mobile == 2)
                {
                    badge = ":/resources/images/goodgame/ios.svg";
                }
                else if (mobile == 4)
                {
                    badge = ":/resources/images/goodgame/android.svg";
                }
                else if (mobile != 0)
                {
                    badge = ":/resources/images/goodgame/smartphone.svg";
                }
            }
            else
            {
                if (IconTypesExtra.contains(iconType))
                {
                    badge = IconTypesExtra.value(iconType);
                }
                else
                {
                    badge = ":/resources/images/goodgame/" + iconType + ".svg";
                }
            }

            QStringList leftBadges;

            if (!badge.isEmpty())
            {
                if (QFileInfo::exists(badge))
                {
                    leftBadges.append("qrc" + badge);
                }
                else
                {
                    qWarning() << Q_FUNC_INFO << ": not found badge" << badge << ", author name" << authorName;
                }
            }

            const Author author(getServiceType(),
                                authorName,
                                authorId,
                                QUrl(),
                                QUrl("https://goodgame.ru/user/" + authorId),
                                {}, // leftBadges, // TODO
                                {},
                                {},
                                nicknameColor);

            QList<Message::Content*> contents;

            QString text;
            QString rawSmileId;
            bool foundColon = false;
            for (const QChar& c : rawText)
            {
                if (c == ':')
                {
                    if (foundColon)
                    {
                        const QString smileId = rawSmileId.toLower();
                        if (smiles.contains(smileId.toLower()))
                        {
                            contents.append(new Message::Image(smiles.value(smileId)));
                            rawSmileId = QString();
                        }
                        else
                        {
                            //qWarning() << Q_FUNC_INFO << ": smile" << rawSmileId << "not found";
                            text += ":" + rawSmileId + ":";
                        }

                        foundColon = false;
                    }
                    else
                    {
                        if (!text.isEmpty())
                        {
                            contents.append(new Message::Text(text));
                            text = QString();
                        }

                        foundColon = true;
                    }
                }
                else
                {
                    if (foundColon)
                    {
                        if (SmilesValidSymbols.contains(c.toLower()))
                        {
                            rawSmileId += c;
                        }
                        else
                        {
                            text += ":" + rawSmileId + c;
                            rawSmileId.clear();
                            foundColon = false;
                        }
                    }
                    else
                    {
                        text += c;
                    }
                }
            }

            if (!rawSmileId.isEmpty())
            {
                text += ":" + rawSmileId;
            }

            if (!text.isEmpty())
            {
                contents.append(new Message::Text(text));
            }

            const Message message(contents, author, publishedAt, QDateTime::currentDateTime(), messageId);

            messages.append(message);
            authors.append(author);

            if (!requestedInfoUsers.contains(authorName))
            {
                requestedInfoUsers.insert(authorName);
                requestUserPage(authorName, authorId);
            }
        }

        if (!messages.isEmpty())
        {
            emit readyRead(messages, authors);
        }
    }
    else if (type == "welcome")
    {
        const double protocolVersion = data.value("protocolVersion").toDouble();
        if (protocolVersion != 2)
        {
            qWarning() << Q_FUNC_INFO << ": unsupported protocol version" << protocolVersion;
        }
    }
    else if (type == "error")
    {
        qWarning() << Q_FUNC_INFO << ": client received error, channel id =" << channelId_ << ", error num =" << data.value("error_num").toInt() << ", error text =" << data.value("errorMsg").toString();
    }
    else if (type == "success_auth")
    {
        channelId = -1;
        requestChannelStatus();
    }
    else
    {
        qWarning() << Q_FUNC_INFO << ": unknown message type" << type << ", data = \n" << rawData;
    }

    if (!state.connected)
    {
        requestAuth();
    }
}

void GoodGame::sendToWebSocket(const QJsonDocument &data)
{
    socket.sendTextMessage(QString::fromUtf8(data.toJson()));
}
