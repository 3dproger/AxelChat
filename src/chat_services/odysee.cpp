#include "odysee.h"
#include "models/message.h"
#include "models/author.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace
{

static const int ReconncectPeriod = 5000;
static const int PingSendTimeout = 10 * 1000;
static const int CheckPingSendTimeout = PingSendTimeout * 1.5;

static bool checkReply(QNetworkReply *reply, const char *tag, QByteArray& resultData)
{
    resultData.clear();

    if (!reply)
    {
        qWarning() << tag << ": !reply";
        return false;
    }

    resultData = reply->readAll();
    reply->deleteLater();
    if (resultData.isEmpty())
    {
        qWarning() << tag << ": data is empty";
        return false;
    }

    return true;
}

}

Odysee::Odysee(QSettings &settings, const QString &settingsGroupPathParent, QNetworkAccessManager &network_, cweqt::Manager&, QObject *parent)
    : ChatService(settings, settingsGroupPathParent, AxelChat::ServiceType::Odysee, false, parent)
    , network(network_)
{
    ui.findBySetting(stream)->setItemProperty("name", tr("Stream"));
    ui.findBySetting(stream)->setItemProperty("placeholderText", tr("Stream link..."));

    QObject::connect(&socket, &QWebSocket::stateChanged, this, [](QAbstractSocket::SocketState state){
        Q_UNUSED(state)
        //qDebug() << Q_FUNC_INFO << "webSocket state changed:" << state;
    });

    QObject::connect(&socket, &QWebSocket::textMessageReceived, this, &Odysee::onReceiveWebSocket);

    QObject::connect(&socket, &QWebSocket::connected, this, [this]()
    {
        //qDebug() << Q_FUNC_INFO << "webSocket connected";

        if (!state.connected)
        {
            state.connected = true;
            emit connectedChanged(true);
            emit stateChanged();

            sendPing();
        }

        checkPingTimer.setInterval(CheckPingSendTimeout);
        checkPingTimer.start();
    });

    QObject::connect(&socket, &QWebSocket::disconnected, this, [this]()
    {
        //qDebug() << Q_FUNC_INFO << "webSocket disconnected";

        if (state.connected)
        {
            state.connected = false;
            emit connectedChanged(false);
            emit stateChanged();
        }
    });

    QObject::connect(&socket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), this, [this](QAbstractSocket::SocketError error_){
        qDebug() << Q_FUNC_INFO << "webSocket error:" << error_ << ":" << socket.errorString();
    });

    QObject::connect(&timerReconnect, &QTimer::timeout, this, [this]()
    {
        if (!enabled.get())
        {
            return;
        }

        if (!state.connected)
        {
            reconnect();
        }
    });
    timerReconnect.start(ReconncectPeriod);

    reconnect();
}

ChatService::ConnectionStateType Odysee::getConnectionStateType() const
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

QString Odysee::getStateDescription() const
{
    switch (getConnectionStateType())
    {
    case ConnectionStateType::NotConnected:
        if (stream.get().isEmpty())
        {
            return tr("Channel not specified");
        }

        if (state.streamId.isEmpty())
        {
            return tr("The channel is not correct");
        }

        return tr("Not connected");

    case ConnectionStateType::Connecting:
        return tr("Connecting...");

    case ConnectionStateType::Connected:
        return tr("Successfully connected!");

    }

    return "<unknown_state>";
}

void Odysee::reconnectImpl()
{
    socket.close();

    state = State();
    info = Info();

    extractChannelAndVideo(stream.get(), info.channel, info.video);

    if (!info.channel.isEmpty() && !info.video.isEmpty())
    {
        state.streamId = info.channel + "/" + info.video;

        state.streamUrl = "https://odysee.com/@" + info.channel + "/" + info.video;
        state.chatUrl = "https://odysee.com/$/popout/@" + info.channel + "/" + info.video;
    }

    state.controlPanelUrl = "https://odysee.com/$/livestream";

    if (!enabled.get() || state.streamId.isEmpty())
    {
        return;
    }

    requestClaimId();
}

void Odysee::onReceiveWebSocket(const QString &rawData)
{
    if (!enabled.get())
    {
        return;
    }

    checkPingTimer.setInterval(CheckPingSendTimeout);
    checkPingTimer.start();

    const QJsonDocument doc = QJsonDocument::fromJson(rawData.toUtf8());
    const QJsonObject root = doc.object();

    qDebug() << "received:" << doc;

    const QJsonObject data = root.value("data").toObject();

    const QString type = root.value("type").toString();
    if (type == "delta")
    {
        parseComment(data);
    }
    else if (type == "removed")
    {
        parseRemoved(data);
    }
    else
    {
        qWarning() << Q_FUNC_INFO << "unknown type" << type << ", received =" << doc;
    }
}

void Odysee::requestClaimId()
{
    if (!enabled.get())
    {
        return;
    }

    const QJsonArray urls = { "lbry://@" + info.channel.replace(':', '#') + "/" + info.video.replace(':', '#') };
    const QJsonObject params = { { "urls", urls } };
    const QByteArray data = QJsonDocument(QJsonObject(
    {
        { "jsonrpc", "2.0" },
        { "method", "resolve" },
        { "params", params }
    })).toJson();

    QNetworkRequest request(QUrl("https://api.na-backend.odysee.com/api/v1/proxy"));
    request.setRawHeader("Content-Type", "application/json-rpc");

    QNetworkReply* reply = network.post(request, data);
    QObject::connect(reply, &QNetworkReply::finished, this, [this, reply]()
    {
        QByteArray data;
        if (!checkReply(reply, Q_FUNC_INFO, data))
        {
            return;
        }

        const QJsonObject root = QJsonDocument::fromJson(data).object();
        const QJsonObject result = root.value("result").toObject();
        const QStringList keys = result.keys();
        if (keys.isEmpty())
        {
            qWarning() << Q_FUNC_INFO << "keys of result is empty, root =" << root;
            return;
        }

        const QJsonObject videoInfo = result.value(keys.first()).toObject();
        info.claimId = videoInfo.value("claim_id").toString();

        if (info.claimId.isEmpty())
        {
            qWarning() << Q_FUNC_INFO << "failed to find claim id, root =" << root;
            return;
        }

        requestLive();
    });
}

void Odysee::requestLive()
{
    if (!enabled.get())
    {
        return;
    }

    QNetworkRequest request(QUrl("https://api.odysee.live/livestream/is_live"));
    request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");

    const QByteArray data = ("channel_claim_id=" + info.claimId).toUtf8();

    QNetworkReply* reply = network.post(request, data);
    QObject::connect(reply, &QNetworkReply::finished, this, [this, reply]()
    {
        QByteArray data;
        if (!checkReply(reply, Q_FUNC_INFO, data))
        {
            return;
        }

        const QJsonObject root = QJsonDocument::fromJson(data).object();
        const QJsonObject dataJson = root.value("data").toObject();

        state.viewersCount = dataJson.value("ViewerCount").toInt(-1);

        if (state.viewersCount < 0)
        {
            qWarning() << Q_FUNC_INFO << "failed to get viewers count, root =" << root;
        }

        emit stateChanged();

        if (socket.state() != QAbstractSocket::SocketState::ConnectedState)
        {
            socket.open(
                "wss://sockety.odysee.tv/ws/commentron?id=" +
                info.claimId +
                "&category=@" +
                info.video +
                "&sub_category=commenter");
        }
    });
}

void Odysee::sendPing()
{
    qWarning() << Q_FUNC_INFO << "not implemented";
}

void Odysee::extractChannelAndVideo(const QString &rawLink, QString &channel, QString &video)
{
    channel = QString();
    video = QString();

    QString link = AxelChat::simplifyUrl(rawLink.trimmed());

    link = AxelChat::removeFromStart(link, "odysee.com", Qt::CaseInsensitive);
    link = AxelChat::removeFromStart(link, "/", Qt::CaseInsensitive);
    link = AxelChat::removeFromStart(link, "@", Qt::CaseInsensitive);

    link = QUrl::fromPercentEncoding(link.toUtf8());

    const QStringList part = link.split('/', Qt::SplitBehaviorFlags::SkipEmptyParts);
    if (part.count() >= 2)
    {
        channel = part[0];
        video = part[1];
    }
}

void Odysee::parseComment(const QJsonObject &data)
{
    const QJsonObject comment = data.value("comment").toObject();

    const int64_t timestamp = comment.value("timestamp").toVariant().toLongLong();

    const QString rawText = comment.value("comment").toString();

    const auto author = Author::Builder(
        getServiceType(),
        generateAuthorId(comment.value("channel_id").toString()),
        comment.value("channel_name").toString())
        .build();

    const auto message = Message::Builder(
        author,
        generateMessageId(comment.value("comment_id").toString()))
        .addText(rawText)
        .setReceivedTime(QDateTime::currentDateTime())
        .setPublishedTime(QDateTime::fromSecsSinceEpoch(timestamp))
        .build();

    emit readyRead({ message}, { author });
}

void Odysee::parseRemoved(const QJsonObject &data)
{
    const QJsonObject comment = data.value("comment").toObject();

    const QString messageId = generateMessageId(comment.value("comment_id").toString());

    const auto pair = Message::Builder::createDeleter(getServiceType(), messageId);

    emit readyRead({ pair.second }, { pair.first });
}
