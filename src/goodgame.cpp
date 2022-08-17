#include "goodgame.h"
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QNetworkAccessManager>

GoodGame::GoodGame(QSettings& settings_, const QString& settingsGroupPath, QNetworkAccessManager& network_, QObject *parent)
    : AbstractChatService(parent)
    , settings(settings_)
    , SettingsGroupPath(settingsGroupPath)
    , network(network_)
{
    QObject::connect(&_socket, &QWebSocket::stateChanged, this, [=](QAbstractSocket::SocketState state){
        //qDebug() << "GoodGame WebSocket state changed:" << state;
    });

    QObject::connect(&_socket, &QWebSocket::textMessageReceived, this, &GoodGame::onWebSocketReceived);

    QObject::connect(&_socket, &QWebSocket::connected, this, [=]() {
        if (_info.connected)
        {
            _info.connected = false;
            emit stateChanged();
        }
    });

    QObject::connect(&_socket, &QWebSocket::disconnected, this, [=](){
        qDebug() << "GoodGame disconnected";

        if (_info.connected)
        {
            _info.connected = false;
            emit disconnected(_lastConnectedChannelName);
            emit stateChanged();
        }
    });

    QObject::connect(&_socket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), this, [=](QAbstractSocket::SocketError error_){
        qDebug() << "GoodGame WebSocket error:" << error_;
    });

    reconnect();

    QObject::connect(&_timerReconnect, &QTimer::timeout, this, &GoodGame::timeoutReconnect);
    _timerReconnect.start(2000);
}

GoodGame::~GoodGame()
{
    _socket.close();
}

AbstractChatService::ConnectionStateType GoodGame::connectionStateType() const
{
    return AbstractChatService::ConnectionStateType::NotConnected;
}

QString GoodGame::stateDescription() const
{
    return "unknown";
}

QString GoodGame::detailedInformation() const
{
    return "<empty>";
}

int GoodGame::viewersCount() const
{
    //ToDo:
    return 0;
}

void GoodGame::timeoutReconnect()
{
    if (!_info.connected /*&& !_info.oauthToken.isEmpty()*/)
    {
        reconnect();
    }
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

void GoodGame::requestGetChannelHistory()
{
    QJsonDocument document;

    QJsonObject root;
    root.insert("type", "get_channel_history");

    QJsonObject data;
    data.insert("channel_id", "5");

    root.insert("data", data);
    document.setObject(root);
    sendToWebSocket(document);
}

void GoodGame::reconnect()
{
    return;
    _socket.close();

    //https://goodgame.ru/chat/26624

    _socket.setProxy(network.proxy());

    _socket.open(QUrl("wss://chat.goodgame.ru/chat/websocket"));
}

QString GoodGame::getNameLocalized() const
{
    return tr("GoodGame");
}

void GoodGame::onWebSocketReceived(const QString &rawData)
{
    const QJsonDocument document = QJsonDocument::fromJson(rawData.toUtf8());
    const QJsonObject root = document.object();
    const QJsonObject data = root.value("data").toObject();
    const QString type = root.value("type").toString();
    const QString channelId = data.value("channel_id").toString();

    if (type == "channel_history")
    {
        QList<ChatMessage> messages;

        const QJsonArray jsonMessages = data.value("messages").toArray();
        for (const QJsonValue& value : qAsConst(jsonMessages))
        {
            const QJsonObject jsonMessage = value.toObject();

            const QString userId = jsonMessage.value("user_id").toString();
            const QString userName = jsonMessage.value("user_name").toString();
            const int userGroup = jsonMessage.value("user_group").toInt();
            const QString messageId = jsonMessage.value("message_id").toString();
            const qint64 timestamp = qint64(jsonMessage.value("timestamp").toDouble());
            const QString text = jsonMessage.value("text").toString();

            const MessageAuthor author = MessageAuthor::createFromGoodGame(userName, userId, userGroup);
            const ChatMessage message = ChatMessage::createFromGoodGame(text, QDateTime::fromMSecsSinceEpoch(timestamp), author);

            messages.append(message);
        }

        if (!messages.isEmpty())
        {
            emit readyRead(messages);
        }
    }
    else if (type == "welcome")
    {
        _info.protocolVersion = data.value("protocolVersion").toDouble();
        if (_info.protocolVersion != 1.1)
        {
            qWarning() << "GoodGame: unsupported protocol version" << _info.protocolVersion;
        }
    }
    else if (type == "error")
    {
        qWarning() << "GoodGame: client received error, channel id =" << channelId << ", error num =" << data.value("error_num").toInt() << ", error text =" << data.value("errorMsg").toString();
    }
    else if (type == "success_auth")
    {
        _info.channelId = channelId;
        _info.connected = true;
        _lastConnectedChannelName = _info.channelId;
        emit connected(_info.channelId);
        emit stateChanged();

        requestGetChannelHistory();

        qDebug() << "GoodGame connected" << _info.channelId;
    }
    else
    {
        qDebug() << "GoodGame: unknown message type" << type << ", data = \n" << rawData;
    }


    if (!_info.connected)
    {
        requestAuth();
    }
}

void GoodGame::sendToWebSocket(const QJsonDocument &data)
{
    _socket.sendTextMessage(QString::fromUtf8(data.toJson()));
}
