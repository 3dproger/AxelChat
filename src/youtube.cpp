#include "youtube.hpp"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>
#include <QFile>

YouTube::YouTube(OutputToFile* outputToFile, QSettings* settings, CefRefPtr<QtCefApp> cefApp, const QString& settingsGroupPath, QObject *parent)
    : QObject(parent), _outputToFile(outputToFile), _settings(settings), _settingsGroupPath(settingsGroupPath), _cefApp(cefApp)
{
    if (!_cefApp)
    {
        qWarning() << Q_FUNC_INFO << ": cefApp == nullptr";
    }

    if (_settings)
    {
        setLink(_settings->value(_settingsGroupPath + "/" + _settingsKeyUserSpecifiedLink).toString());
    }
}

YouTube::~YouTube()
{
    _youtubeInfo.broadcastConnected = false;
    emit disconnected(_youtubeInfo.broadcastId);
    emit connectedChanged();

    if (_outputToFile)
    {
        _outputToFile->setYouTubeInfo(_youtubeInfo);
    }
}

QString YouTube::extractBroadcastId(const QString &link) const
{
    QString withoutHttpsWWW = link;

    // \ ->/
    withoutHttpsWWW = withoutHttpsWWW.replace('\\', '/');

    //https:// http://
    if (withoutHttpsWWW.startsWith("https://"))
    {
        withoutHttpsWWW = withoutHttpsWWW.mid(8);
    }
    else if (withoutHttpsWWW.startsWith("http://"))
    {
        withoutHttpsWWW = withoutHttpsWWW.mid(7);
    }

    //www.
    if (withoutHttpsWWW.startsWith("www."))
    {
        withoutHttpsWWW = withoutHttpsWWW.mid(4);
    }

    // remove last '/'
    if (withoutHttpsWWW.endsWith("/"))
    {
        withoutHttpsWWW = withoutHttpsWWW.left(withoutHttpsWWW.lastIndexOf('/'));
    }

    //?
    QString withoutQuery = withoutHttpsWWW;
    if (withoutHttpsWWW.contains('?'))
    {
        withoutQuery = withoutHttpsWWW.left(withoutHttpsWWW.indexOf('?'));
    }

    const QUrlQuery& urlQuery = QUrlQuery(QUrl(link).query());

    const QString& vParameter = urlQuery.queryItemValue("v");

    QString broadcastId;
    QRegExp rx;

    //youtu.be/rSjMyeISW7w
    if (broadcastId.isEmpty())
    {
        rx = QRegExp("^youtu.be/([^/]*)$", Qt::CaseInsensitive);
        if (rx.indexIn(withoutQuery) != -1)
        {
            broadcastId = rx.cap(1);
        }
    }

    //studio.youtube.com/video/rSjMyeISW7w/livestreaming
    if (broadcastId.isEmpty())
    {
        rx = QRegExp("^(studio\\.)?youtube.com/video/([^/]*)/livestreaming$", Qt::CaseInsensitive);
        if (rx.indexIn(withoutQuery) != -1)
        {
            broadcastId = rx.cap(2);
        }
    }

    //youtube.com/video/rSjMyeISW7w
    //studio.youtube.com/video/rSjMyeISW7w
    if (broadcastId.isEmpty())
    {
        rx = QRegExp("^(studio\\.)?youtube.com/video/([^/]*)$", Qt::CaseInsensitive);
        if (rx.indexIn(withoutQuery) != -1)
        {
            broadcastId = rx.cap(2);
        }
    }

    //youtube.com/watch/rSjMyeISW7w
    //studio.youtube.com/watch/rSjMyeISW7w
    if (broadcastId.isEmpty())
    {
        rx = QRegExp("^(studio\\.)?youtube.com/watch/([^/]*)$", Qt::CaseInsensitive);
        if (rx.indexIn(withoutQuery) != -1)
        {
            broadcastId = rx.cap(2);
        }
    }

    //youtube.com/live_chat?is_popout=1&v=rSjMyeISW7w
    //studio.youtube.com/live_chat?v=rSjMyeISW7w&is_popout=1
    if (broadcastId.isEmpty())
    {
        rx = QRegExp("^(studio\\.)?youtube.com/live_chat$", Qt::CaseInsensitive);
        if (rx.indexIn(withoutQuery) != -1 && !vParameter.isEmpty())
        {
            broadcastId = vParameter;
        }
    }

    //https://www.youtube.com/watch?v=rSjMyeISW7w&feature=youtu.be
    //https://www.youtube.com/watch?v=rSjMyeISW7w
    if (broadcastId.isEmpty())
    {
        rx = QRegExp("^(studio\\.)?youtube.com/watch$", Qt::CaseInsensitive);
        if (rx.indexIn(withoutQuery) != -1 && !vParameter.isEmpty())
        {
            broadcastId = vParameter;
        }
    }

    //Broadcast id only
    //rSjMyeISW7w
    if (broadcastId.isEmpty())
    {
        rx = QRegExp("^[a-zA-Z0-9_\\-]+$", Qt::CaseInsensitive);
        if (rx.indexIn(withoutHttpsWWW) != -1)
        {
            broadcastId = withoutHttpsWWW;
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
    return _youtubeInfo.broadcastChatUrl;
}

QUrl YouTube::controlPanelUrl() const
{
    return _youtubeInfo.controlPanelUrl;
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

QUrl YouTube::broadcastLongUrl() const
{
    return _youtubeInfo.broadcastLongUrl;
}

QString YouTube::userSpecifiedLink() const
{
    return _youtubeInfo.userSpecified;
}

QUrl YouTube::broadcastShortUrl() const
{
    return _youtubeInfo.broadcastShortUrl;
}

QString YouTube::broadcastId() const
{
    return _youtubeInfo.broadcastId;
}

bool YouTube::isConnected() const
{
    return _youtubeInfo.broadcastConnected;
}

bool YouTube::isBroadcastIdUserSpecified() const
{
    return _youtubeInfo.userSpecified.trimmed() == _youtubeInfo.broadcastId.trimmed() && !_youtubeInfo.userSpecified.isEmpty();
}

void YouTube::reconnect()
{
    const QString link = _youtubeInfo.userSpecified;
    setLink("");
    setLink(link);
}

int YouTube::messagesReceived() const
{
    return _messagesReceived;
}

void YouTube::setLink(QString link)
{
    link = link.trimmed();

    if (_youtubeInfo.userSpecified != link)
    {
        bool preConnected = _youtubeInfo.broadcastConnected;
        QString preBroadcastId = _youtubeInfo.broadcastId;

        _youtubeInfo = YouTubeInfo();

        if (_youtubeInfo.broadcastConnected)
        {
            _youtubeInfo.broadcastConnected = false;
            emit connectedChanged();
        }

        _youtubeInfo.userSpecified = link;
        _youtubeInfo.broadcastId = extractBroadcastId(link);

        if (!_youtubeInfo.broadcastId.isEmpty())
        {
            _youtubeInfo.broadcastChatUrl = QUrl(QString("https://www.youtube.com/watch?v=%1")
                                .arg(_youtubeInfo.broadcastId));

            /*_youtubeInfo.broadcastChatUrl = QUrl(QString("https://www.youtube.com/live_chat?v=%1")
                    .arg(_youtubeInfo.broadcastId));*/

            _youtubeInfo.broadcastShortUrl = QUrl(QString("https://youtu.be/%1")
                                 .arg(_youtubeInfo.broadcastId));

            _youtubeInfo.broadcastLongUrl = QUrl(QString("https://www.youtube.com/watch?v=%1")
                                 .arg(_youtubeInfo.broadcastId));

            _youtubeInfo.controlPanelUrl = QUrl(QString("https://studio.youtube.com/video/%1/livestreaming")
                                                         .arg(_youtubeInfo.broadcastId));
        }

        if (_settings)
        {
            _settings->setValue(_settingsGroupPath + "/" + _settingsKeyUserSpecifiedLink, _youtubeInfo.userSpecified);
        }

        if (preConnected && !preBroadcastId.isEmpty())
        {
            emit disconnected(preBroadcastId);
        }

        if (_outputToFile)
        {
            _outputToFile->setYouTubeInfo(_youtubeInfo);
        }

        /*qDebug() << "User Specified Link" << _youtubeInfo.userSpecified;
        qDebug() << "Broadcast ID:" << _youtubeInfo.broadcastId;
        qDebug() << "Broadcast URL:" << _youtubeInfo.broadcastURL.toString();
        qDebug() << "Chat URL:" << _youtubeInfo.broadcastChatURL.toString();*/

        if (_cefApp)
        {
            _cefApp->setUrl(_youtubeInfo.broadcastChatUrl.toString());
        }

        //_webPage.load(QUrl(_youtubeInfo.broadcastChatUrl));

        emit linkChanged();
    }
}

void YouTube::onDataReceived(std::shared_ptr<QByteArray> data)
{
    if (!data)
    {
        return;
    }

    if (data->toUpper().startsWith("<!DOCTYPE HTML>"))
    {
        parseHTML(data);
        return;
    }

    const QJsonDocument jsonDocument = QJsonDocument::fromJson(*data);
    const QJsonObject liveChatContinuation = jsonDocument.object().value("continuationContents").toObject().value("liveChatContinuation").toObject();
    const QJsonValue actions = liveChatContinuation.value("actions");
    if (actions.isArray())
    {
        parseActionsArray(actions.toArray(), *data);
        return;
    }

    if (data->startsWith("{\n  \"responseContext\":"))
    {
        return;
    }

    QByteArray snap = data->left(128);
    if (data->size() > snap.size())
    {
        snap += "...";
    }

    printData(Q_FUNC_INFO + QString(": unknown data type"), snap);
}

void YouTube::parseActionsArray(const QJsonArray& array, const QByteArray& data)
{
    if (!_youtubeInfo.broadcastConnected && !_youtubeInfo.broadcastId.isEmpty())
    {
        qDebug(QString("YouTube connected: %1")
               .arg(_youtubeInfo.broadcastId).toUtf8());
        _youtubeInfo.broadcastConnected = true;
        if (_outputToFile)
        {
            _outputToFile->setYouTubeInfo(_youtubeInfo);
        }

        emit connected(_youtubeInfo.broadcastId);
        emit connectedChanged();
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
            qDebug() << Q_FUNC_INFO << QString(": object \"replaceChatItemAction\" not supported yet");
        }
        else if (actionObject.contains("addLiveChatTickerItemAction"))
        {
            qDebug() << Q_FUNC_INFO << QString(": object \"addLiveChatTickerItemAction\" not supported yet");
        }
        else if (actionObject.contains("addBannerToLiveChatCommand"))
        {
            qDebug() << Q_FUNC_INFO << QString(": object \"addBannerToLiveChatCommand\" not supported yet");
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

void YouTube::parseHTML(std::shared_ptr<QByteArray> rawData)
{
    if (!rawData)
    {
        return;
    }

    /*QFile file("debug.html");
    if (file.open(QFile::OpenModeFlag::WriteOnly | QFile::OpenModeFlag::Text))
    {
        file.write(*rawData);
        file.close();
    }*/

    const int start = rawData->indexOf("\"actions\":[");
    if (start == -1)
    {
        qDebug() << Q_FUNC_INFO << ": not found actions";
        return;
    }

    QByteArray& data = rawData->remove(0, start + 10);

    //ToDo: учитывать символы [ и ] в кавычках
    int brackets = 0;
    bool insideQuotes = false;
    for (int i = 0; i < data.length(); ++i)
    {
        if (!insideQuotes)
        {
            if (data[i] == '[')
            {
                brackets++;
            }
            else if  (data[i] == ']')
            {
                brackets--;
            }
            else if (data[i] == '\"')
            {
                insideQuotes = true;
            }

            if (brackets == 0)
            {
                data = data.remove(i + 1, data.length());
                break;
            }
        }
        else
        {
            const bool prevIsBackslash = i > 0 && data[i - 1] == '\\';
            if (data[i] == '\"' && !prevIsBackslash)
            {
                insideQuotes = false;
            }
        }
    }

    const QJsonDocument jsonDocument = QJsonDocument::fromJson(data);
    if (jsonDocument.isArray())
    {
        const QJsonArray actionsArray = jsonDocument.array();
        parseActionsArray(actionsArray, data);
    }
    else
    {
        printData(Q_FUNC_INFO + QString(": document is not array"), data);
    }
}
