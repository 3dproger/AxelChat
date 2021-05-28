#include "youtube.hpp"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>
#include <QFile>
#include <QDir>
#include <QNetworkRequest>
#include <QNetworkReply>

namespace
{

static const int RequestChatInterval = 2000;
static const QString FolderLogs = "logs_youtube";
static const QByteArray UserAgent = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.77 Safari/537.36";
static const QByteArray AcceptLanguage = "en-US";

void saveFile(const QString& fileName, const QByteArray& data)
{
    QDir().mkpath(FolderLogs);

    QFile file(fileName);
    if (file.open(QFile::OpenModeFlag::WriteOnly | QFile::OpenModeFlag::Text))
    {
        file.write(data);
        file.close();
        qDebug() << "Saved to" << file.fileName();
    }
    else
    {
        qDebug() << "Failed to save" << file.fileName();
    }
}

}

YouTube::YouTube(const QNetworkProxy& proxy, OutputToFile* outputToFile, QSettings* settings, const QString& settingsGroupPath, QObject *parent)
    : AbstractChatService(proxy, parent), _outputToFile(outputToFile), _settings(settings), _settingsGroupPath(settingsGroupPath)
{
    _manager.setProxy(proxy);

    if (_settings)
    {
        setLink(_settings->value(_settingsGroupPath + "/" + _settingsKeyUserSpecifiedLink).toString());
    }

    QObject::connect(&_manager, &QNetworkAccessManager::finished, this, &YouTube::onReply);

    QObject::connect(&_timerRequestChat, &QTimer::timeout, this, &YouTube::onTimeoutRequestChat);

    _timerRequestChat.start(RequestChatInterval);
}

YouTube::~YouTube()
{
    _info.broadcastConnected = false;
    emit disconnected(_info.broadcastId);
    emit stateChanged();

    if (_outputToFile)
    {
        _outputToFile->setYouTubeInfo(_info);
    }
}

QString YouTube::extractBroadcastId(const QString &link) const
{
    const QString simpleUrl = simplifyUrl(link);

    const QUrlQuery& urlQuery = QUrlQuery(QUrl(link).query());

    const QString& vParameter = urlQuery.queryItemValue("v");

    QString broadcastId;
    QRegExp rx;

    //youtu.be/rSjMyeISW7w
    if (broadcastId.isEmpty())
    {
        rx = QRegExp("^youtu.be/([^/]*)$", Qt::CaseInsensitive);
        if (rx.indexIn(simpleUrl) != -1)
        {
            broadcastId = rx.cap(1);
        }
    }

    //studio.youtube.com/video/rSjMyeISW7w/livestreaming
    if (broadcastId.isEmpty())
    {
        rx = QRegExp("^(studio\\.)?youtube.com/video/([^/]*)/livestreaming$", Qt::CaseInsensitive);
        if (rx.indexIn(simpleUrl) != -1)
        {
            broadcastId = rx.cap(2);
        }
    }

    //youtube.com/video/rSjMyeISW7w
    //studio.youtube.com/video/rSjMyeISW7w
    if (broadcastId.isEmpty())
    {
        rx = QRegExp("^(studio\\.)?youtube.com/video/([^/]*)$", Qt::CaseInsensitive);
        if (rx.indexIn(simpleUrl) != -1)
        {
            broadcastId = rx.cap(2);
        }
    }

    //youtube.com/watch/rSjMyeISW7w
    //studio.youtube.com/watch/rSjMyeISW7w
    if (broadcastId.isEmpty())
    {
        rx = QRegExp("^(studio\\.)?youtube.com/watch/([^/]*)$", Qt::CaseInsensitive);
        if (rx.indexIn(simpleUrl) != -1)
        {
            broadcastId = rx.cap(2);
        }
    }

    //youtube.com/live_chat?is_popout=1&v=rSjMyeISW7w
    //studio.youtube.com/live_chat?v=rSjMyeISW7w&is_popout=1
    if (broadcastId.isEmpty())
    {
        rx = QRegExp("^(studio\\.)?youtube.com/live_chat$", Qt::CaseInsensitive);
        if (rx.indexIn(simpleUrl) != -1 && !vParameter.isEmpty())
        {
            broadcastId = vParameter;
        }
    }

    //https://www.youtube.com/watch?v=rSjMyeISW7w&feature=youtu.be
    //https://www.youtube.com/watch?v=rSjMyeISW7w
    if (broadcastId.isEmpty())
    {
        rx = QRegExp("^(studio\\.)?youtube.com/watch$", Qt::CaseInsensitive);
        if (rx.indexIn(simpleUrl) != -1 && !vParameter.isEmpty())
        {
            broadcastId = vParameter;
        }
    }

    //Broadcast id only
    //rSjMyeISW7w
    if (broadcastId.isEmpty())
    {
        rx = QRegExp("^[a-zA-Z0-9_\\-]+$", Qt::CaseInsensitive);
        if (rx.indexIn(link) != -1)
        {
            broadcastId = link;
        }
    }

    return broadcastId;
}

void YouTube::printData(const QString &tag, const QByteArray& data)
{
    qDebug() << "==============================================================================================================================";
    qDebug(tag.toUtf8());
    qDebug() << "==================================DATA========================================================================================";
    qDebug() << data;
    qDebug() << "==============================================================================================================================";
}

QUrl YouTube::chatUrl() const
{
    return _info.broadcastChatUrl;
}

QUrl YouTube::controlPanelUrl() const
{
    return _info.controlPanelUrl;
}

QUrl YouTube::createResizedAvatarUrl(const QUrl &sourceAvatarUrl, int imageHeight)
{
    //qDebug("Source URL: " + sourceAvatarUrl.toString().toUtf8());

    //example: https://yt3.ggpht.com/ytc/AAUvwngFVeI2l6JADC9wxZbdGK1fu382MwOtp6bYWA=s3200-c-k-c0x00ffffff-no-rj

    QString source = sourceAvatarUrl.toString().trimmed();
    source.replace('\\', '/');
    if (source.back() == '/')
    {
        source = source.left(source.length() - 1);
    }

    const QVector<QStringRef>& parts = source.splitRef('/', Qt::KeepEmptyParts);
    if (parts.count() < 2)
    {
        qDebug() << Q_FUNC_INFO << ": Failed to convert: parts.count() < 2";
        return sourceAvatarUrl;
    }

    //qDebug("Before URL: " + sourceAvatarUrl.toString().toUtf8());

    QString targetPart = parts[parts.count() - 1].toString();
    QRegExp rx(".*s(\\d+).*", Qt::CaseInsensitive);
    rx.setMinimal(false);
    if (rx.lastIndexIn(targetPart) != -1)
    {
        //qDebug("Regexp matched: '" + rx.cap(1).toUtf8() + "'");
        //qDebug(QString("Regexp matched position: %1").arg(rx.pos()).toUtf8());

        targetPart.remove(rx.pos() + 1, rx.cap(1).length());
        targetPart.insert(rx.pos() + 1, QString("%1").arg(imageHeight));

        QString newUrlStr;
        for (int i = 0; i < parts.count(); ++i)
        {
            if (i != parts.count() - 1)
            {
                newUrlStr += parts[i].toString();
            }
            else
            {
                newUrlStr += targetPart;
            }

            if (i != parts.count() - 1)
            {
                newUrlStr += "/";
            }
        }

        //qDebug("Result URL: " + newUrlStr.toUtf8());
        return newUrlStr;
    }

    qDebug() << Q_FUNC_INFO << ": Failed to convert";
    return sourceAvatarUrl;
}

void YouTube::setProxy(const QNetworkProxy &proxy)
{
    _manager.setProxy(proxy);
    reconnect();
}

QUrl YouTube::broadcastLongUrl() const
{
    return _info.broadcastLongUrl;
}

QString YouTube::userSpecifiedLink() const
{
    return _info.userSpecified;
}

QUrl YouTube::broadcastUrl() const
{
    return _info.broadcastShortUrl;
}

QString YouTube::broadcastId() const
{
    return _info.broadcastId;
}

bool YouTube::isBroadcastIdUserSpecified() const
{
    return _info.userSpecified.trimmed() == _info.broadcastId.trimmed() && !_info.userSpecified.isEmpty();
}

void YouTube::reconnect()
{
    const QString link = _info.userSpecified;
    setLink("");
    setLink(link);
}

AbstractChatService::ConnectionStateType YouTube::connectionStateType() const
{
    if (_info.broadcastConnected)
    {
        return AbstractChatService::ConnectionStateType::Connected;
    }
    else if (!_info.broadcastId.isEmpty())
    {
        return AbstractChatService::ConnectionStateType::Connecting;
    }

    return AbstractChatService::ConnectionStateType::NotConnected;
}

QString YouTube::stateDescription() const
{
    switch (connectionStateType()) {
    case ConnectionStateType::NotConnected:
        if (_info.userSpecified.isEmpty())
        {
            return tr("Broadcast not specified");
        }

        if (_info.broadcastId.isEmpty())
        {
            return tr("The broadcast is not correct");
        }

        return tr("Not connected");

    case ConnectionStateType::Connecting:
        return tr("Connecting...");

    case ConnectionStateType::Connected:
        return tr("Successfully connected!");

    }

    return "<unknown_state>";
}

int YouTube::messagesReceived() const
{
    return _messagesReceived;
}

void YouTube::setLink(QString link)
{
    link = link.trimmed();

    if (_info.userSpecified != link)
    {
        bool preConnected = _info.broadcastConnected;
        QString preBroadcastId = _info.broadcastId;

        _info = YouTubeInfo();

        _info.broadcastConnected = false;

        _info.userSpecified = link;
        _info.broadcastId = extractBroadcastId(link);

        if (!_info.broadcastId.isEmpty())
        {
            _info.broadcastChatUrl = QUrl(QString("https://www.youtube.com/live_chat?v=%1")
                    .arg(_info.broadcastId));

            _info.broadcastShortUrl = QUrl(QString("https://youtu.be/%1")
                                 .arg(_info.broadcastId));

            _info.broadcastLongUrl = QUrl(QString("https://www.youtube.com/watch?v=%1")
                                 .arg(_info.broadcastId));

            _info.controlPanelUrl = QUrl(QString("https://studio.youtube.com/video/%1/livestreaming")
                                                         .arg(_info.broadcastId));
        }

        if (_settings)
        {
            _settings->setValue(_settingsGroupPath + "/" + _settingsKeyUserSpecifiedLink, _info.userSpecified);
        }

        if (preConnected && !preBroadcastId.isEmpty())
        {
            emit disconnected(preBroadcastId);
        }

        if (_outputToFile)
        {
            _outputToFile->setYouTubeInfo(_info);
        }

        /*qDebug() << "User Specified Link" << _youtubeInfo.userSpecified;
        qDebug() << "Broadcast ID:" << _youtubeInfo.broadcastId;
        qDebug() << "Broadcast URL:" << _youtubeInfo.broadcastURL.toString();
        qDebug() << "Chat URL:" << _youtubeInfo.broadcastChatURL.toString();*/

        //ToDo update url
    }

    emit stateChanged();

    onTimeoutRequestChat();
}

void YouTube::onTimeoutRequestChat()
{
    if (_info.broadcastChatUrl.isEmpty())
    {
        return;
    }

    QNetworkRequest request(_info.broadcastChatUrl);
    request.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader, UserAgent);
    request.setRawHeader("accept-language", AcceptLanguage);
    QNetworkReply* reply = _manager.get(request);
    if (!reply)
    {
        qDebug() << Q_FUNC_INFO << ": !reply";
        return;
    }
}

void YouTube::onReply(QNetworkReply *reply)
{
    if (!reply)
    {
        qDebug() << "!reply";
        return;
    }

    const QByteArray rawData = reply->readAll();
    if (rawData.isEmpty())
    {
        qDebug() << Q_FUNC_INFO << ":rawData is empty";
        return;
    }

    const QString startData = QString::fromUtf8(rawData.left(100));

    if (startData.contains("<title>Oops</title>", Qt::CaseSensitivity::CaseInsensitive))
    {
        //ToDo: show message "bad url"
        return;
    }

    const int start = rawData.indexOf("\"actions\":[");
    if (start == -1)
    {
        qDebug() << Q_FUNC_INFO << ": not found actions";
        saveFile(FolderLogs + "/not_found_actions_from_html_youtube.html", rawData);
        return;
    }

    QByteArray data = rawData.mid(start + 10);
    const int pos = data.lastIndexOf(",\"actionPanel\"");
    if (pos != -1)
    {
        data = data.remove(pos, data.length());
    }

    const QJsonDocument jsonDocument = QJsonDocument::fromJson(data);
    if (jsonDocument.isArray())
    {
        const QJsonArray actionsArray = jsonDocument.array();
        //qDebug() << "array size = " << actionsArray.size();
        parseActionsArray(actionsArray, data);
    }
    else
    {
        printData(Q_FUNC_INFO + QString(": document is not array"), data);

        saveFile(FolderLogs + "/failed_to_parse_from_html_youtube.html", rawData);
        saveFile(FolderLogs + "/failed_to_parse_from_html_youtube.json", data);
    }
}

void YouTube::parseActionsArray(const QJsonArray& array, const QByteArray& data)
{
    if (!_info.broadcastConnected && !_info.broadcastId.isEmpty())
    {
        qDebug() << "YouTube connected" << _info.broadcastId;
        _info.broadcastConnected = true;
        if (_outputToFile)
        {
            _outputToFile->setYouTubeInfo(_info);
        }

        emit connected(_info.broadcastId);
        emit stateChanged();
    }

    QList<ChatMessage> messages;
    QList<MessageAuthor> authors;

    foreach (const QJsonValue& actionJson, array)
    {
        bool valid = false;
        bool isDeleter = false;

        QString messageText;
        QString messageId;
        const QDateTime& receivedAt = QDateTime::currentDateTime();
        QDateTime publishedAt;

        QString authorName;
        QString authorChannelId;
        QUrl authorBadgeUrl;
        QUrl authorAvatarUrl;
        bool authorIsVerified      = false;
        bool authorIsChatOwner     = false;
        bool authorIsChatSponsor   = false;
        bool authorIsChatModerator = false;

        const QJsonObject& actionObject = actionJson.toObject();

        if (actionObject.contains("addChatItemAction"))
        {
            //Message

            const QJsonObject& addChatItemAction = actionObject.value("addChatItemAction").toObject();

            const QJsonObject& liveChatTextMessageRenderer = addChatItemAction
                    .value("item").toObject()
                    .value("liveChatTextMessageRenderer").toObject();

            if (!liveChatTextMessageRenderer.isEmpty())
            {
                messageId = liveChatTextMessageRenderer
                        .value("id").toString();

                publishedAt = QDateTime::fromMSecsSinceEpoch(
                            liveChatTextMessageRenderer.value("timestampUsec").toString().toLongLong() / 1000,
                            Qt::TimeSpec::UTC).toLocalTime();

                authorName = liveChatTextMessageRenderer
                        .value("authorName").toObject()
                        .value("simpleText").toString();

                authorChannelId = liveChatTextMessageRenderer
                        .value("authorExternalChannelId").toString();

                const QJsonArray& thumbnails = liveChatTextMessageRenderer
                        .value("authorPhoto").toObject()
                        .value("thumbnails").toArray();

                int preHeight = -1;

                for (const QJsonValue& element : thumbnails)
                {
                    const QJsonObject& thumbnail = element.toObject();

                    int height = thumbnail.value("height").toInt();
                    if (height == 32)
                    {
                        //Preferably 32x32
                        authorAvatarUrl  = thumbnail.value("url").toString();
                        break;
                    }
                    else if (height > preHeight || authorAvatarUrl.isEmpty())
                    {
                        //Or getting max size avatar
                        authorAvatarUrl  = thumbnail.value("url").toString();
                        preHeight = height;
                    }
                }

                if (liveChatTextMessageRenderer.contains("authorBadges"))
                {
                    const QJsonArray& authorBadges = liveChatTextMessageRenderer.value("authorBadges").toArray();

                    foreach (const QJsonValue& badge, authorBadges)
                    {
                        const QJsonObject& liveChatAuthorBadgeRenderer = badge.toObject().value("liveChatAuthorBadgeRenderer").toObject();

                        if (liveChatAuthorBadgeRenderer.contains("icon"))
                        {
                            const QString& iconType = liveChatAuthorBadgeRenderer.value("icon").toObject()
                                    .value("iconType").toString();

                            bool foundIconType = false;

                            if (iconType.toLower() == "owner")
                            {
                                authorIsChatOwner = true;
                                foundIconType = true;
                            }

                            if (iconType.toLower() == "moderator")
                            {
                                authorIsChatModerator = true;
                                foundIconType = true;
                            }

                            if (iconType.toLower() == "verified")
                            {
                                authorIsVerified = true;
                                foundIconType = true;
                            }

                            if (!foundIconType && !iconType.isEmpty())
                            {
                                qDebug() << "Unknown iconType" << iconType;
                            }
                        }
                        else if (liveChatAuthorBadgeRenderer.contains("customThumbnail"))
                        {
                            authorIsChatSponsor = true;//ToDo: is not a fact

                            const QJsonArray& thumbnails = liveChatAuthorBadgeRenderer.value("customThumbnail").toObject()
                                    .value("thumbnails").toArray();

                            foreach (const QJsonValue& thumbnailJson, thumbnails)
                            {
                                if (authorBadgeUrl.isEmpty())
                                {
                                    authorBadgeUrl = QUrl(thumbnailJson.toObject().value("url").toString());
                                }
                                else
                                {
                                    break;
                                }
                            }
                        }
                        else
                        {
                            printData(Q_FUNC_INFO + QString(": Unknown json structure of object \"%1\"").arg("liveChatAuthorBadgeRenderer"), data);
                        }
                    }
                }

                const QJsonArray& runs = liveChatTextMessageRenderer
                        .value("message").toObject()
                        .value("runs").toArray();
                foreach (const QJsonValue& run, runs)
                {

                    messageText += run.toObject().value("text").toString();
                }

                valid = true;
            }
        }
        else if (actionObject.contains("markChatItemAsDeletedAction"))
        {
            //Deleted message by unknown

            const QJsonObject& markChatItemAsDeletedAction = actionObject.value("markChatItemAsDeletedAction").toObject();
            const QJsonObject& deletedStateMessage = markChatItemAsDeletedAction.value("deletedStateMessage").toObject();

            const QJsonArray& runs = deletedStateMessage
                    .value("runs").toArray();
            for (const QJsonValue& run : runs)
            {
                messageText += run.toObject().value("text").toString();
            }

            messageId = markChatItemAsDeletedAction.value("targetItemId").toString();

            isDeleter = true;
            valid = true;
        }
        else if (actionObject.contains("markChatItemsByAuthorAsDeletedAction"))
        {
            //ToDo: нужно протестировать. Возможно, это событие удаления всех сообщений
            //Deleted message by author

            const QJsonObject& markChatItemsByAuthorAsDeletedAction = actionObject.value("markChatItemsByAuthorAsDeletedAction").toObject();
            const QJsonObject& deletedStateMessage = markChatItemsByAuthorAsDeletedAction.value("deletedStateMessage").toObject();

            const QJsonArray& runs = deletedStateMessage
                    .value("runs").toArray();
            for (const QJsonValue& run : runs)
            {
                messageText += run.toObject().value("text").toString();
            }

            messageId = markChatItemsByAuthorAsDeletedAction.value("targetItemId").toString();

            isDeleter = true;
            valid = true;
        }
        else if (actionObject.contains("replaceChatItemAction"))
        {
            //qDebug() << Q_FUNC_INFO << QString(": object \"replaceChatItemAction\" not supported yet");
        }
        else if (actionObject.contains("addLiveChatTickerItemAction"))
        {
            //qDebug() << Q_FUNC_INFO << QString(": object \"addLiveChatTickerItemAction\" not supported yet");
        }
        else if (actionObject.contains("addBannerToLiveChatCommand"))
        {
            //qDebug() << Q_FUNC_INFO << QString(": object \"addBannerToLiveChatCommand\" not supported yet");
        }
        else if (actionObject.contains("addToPlaylistCommand") || actionObject.contains("clickTrackingParams"))
        {
            // it probably doesn't need to be maintained
        }
        else
        {
            QJsonDocument doc(actionObject);
            printData(Q_FUNC_INFO + QString(": unknown json structure of array \"%1\"").arg("actions"), doc.toJson());
        }

        if (valid)
        {
            if (isDeleter)
            {
                const ChatMessage& message = ChatMessage::createDeleterFromYouTube(messageText, messageId);
                messages.append(message);
            }
            else
            {
                const MessageAuthor& author = MessageAuthor::createFromYouTube(
                            authorName,
                            authorChannelId,
                            authorAvatarUrl,
                            authorBadgeUrl,
                            authorIsVerified,
                            authorIsChatOwner,
                            authorIsChatSponsor,
                            authorIsChatModerator);

                const ChatMessage& message = ChatMessage::createFromYouTube(
                            messageText,
                            messageId,
                            publishedAt,
                            receivedAt,
                            author);

                messages.append(message);
                authors.append(author);

                _messagesReceived++;
            }
        }
    }

    emit readyRead(messages, authors);
    emit stateChanged();
}
