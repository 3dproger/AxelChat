#include "youtube.hpp"
#include "types.hpp"
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
static const int RequestStreamInterval = 20000;

static const QString FolderLogs = "logs_youtube";

static const QString  SettingsKeyUserSpecifiedLink = "user_specified_link";
static const QString  SettingsKeyIgnoreMessagesBeforeConnect = "ignore_nessages_before_connect";

static const QByteArray AcceptLanguageNetworkHeaderName = ""; // "en-US;q=0.5,en;q=0.3";

static const int MaxBadChatReplies = 10;
static const int MaxBadLivePageReplies = 3;

QByteArray extractDigitsOnly(const QByteArray& data)
{
    QByteArray result;
    for (const char& c : data)
    {
        if (c >= 48 && c <= 57)
        {
            result += c;
        }
    }

    return result;
}

}

YouTube::YouTube(const QNetworkProxy& proxy, QSettings* settings, const QString& settingsGroupPath, QObject *parent)
    : AbstractChatService(proxy, parent)
    , _settings(settings)
    , _settingsGroupPath(settingsGroupPath)
{
    _manager.setProxy(proxy);

    if (_settings)
    {
        setLink(_settings->value(_settingsGroupPath + "/" + SettingsKeyUserSpecifiedLink).toString());
    }

    QObject::connect(&_timerRequestChat, &QTimer::timeout, this, &YouTube::onTimeoutRequestChat);
    _timerRequestChat.start(RequestChatInterval);

    QObject::connect(&_timerRequestStreamPage,&QTimer::timeout, this, &YouTube::onTimeoutRequestStreamPage);
    _timerRequestStreamPage.start(RequestStreamInterval);
}

YouTube::~YouTube()
{
    _info.broadcastConnected = false;
    emit disconnected(_info.broadcastId);
    emit stateChanged();
}

QString YouTube::extractBroadcastId(const QString &link) const
{
    const QString simpleUrl = AxelChat::simplifyUrl(link);

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

    if (source.startsWith("//"))
    {
        source = QString("https:") + source;
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

void YouTube::setNeedRemoveBeforeAtAsCurrent()
{
    _needRemoveBeforeAt = QDateTime::currentDateTime();
}

bool YouTube::isNeedIgnoreMessagesBeforeConnect() const
{
    if (_settings)
    {
        return _settings->value(_settingsGroupPath + "/" + SettingsKeyIgnoreMessagesBeforeConnect, false).toBool();
    }

    return false;
}

void YouTube::setNeedIgnoreMessagesBeforeConnect(bool enable)
{
    if (_settings)
    {
        _settings->setValue(_settingsGroupPath + "/" + SettingsKeyIgnoreMessagesBeforeConnect, enable);
    }
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

QString YouTube::detailedInformation() const
{
    return _info.detailedInformation;
}

int YouTube::viewersCount() const
{
    return _info.viewers;
}

int YouTube::messagesReceived() const
{
    return _messagesReceived;
}

void YouTube::setLink(QString link)
{
    link = link.trimmed();

    const QString simplified = AxelChat::simplifyUrl(link);

    if (_info.userSpecified != link)
    {
        const bool preConnected = _info.broadcastConnected;
        const QString preBroadcastId = _info.broadcastId;

        _info = AxelChat::YouTubeInfo();

        if (!link.isEmpty() && isNeedIgnoreMessagesBeforeConnect())
        {
            setNeedRemoveBeforeAtAsCurrent();
        }

        _badChatReplies = 0;
        _badLivePageReplies = 0;

        _info.broadcastConnected = false;

        _info.userSpecified = link;

        static const QRegExp rx = QRegExp("^youtube.com/channel/(.*)/?$", Qt::CaseInsensitive);
        if (rx.indexIn(simplified) != -1)
        {
            const QString channelId = rx.cap(1);

            _info.channelId = channelId;

            QNetworkRequest request(QString("https://youtube.com/channel/%1/live").arg(channelId));
            request.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader, AxelChat::UserAgentNetworkHeaderName);
            request.setRawHeader("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8");
            request.setRawHeader("Accept-Language", AcceptLanguageNetworkHeaderName);
            QNetworkReply* reply = _manager.get(request);
            if (!reply)
            {
                qDebug() << Q_FUNC_INFO << ": !reply";
                return;
            }

            QObject::connect(reply, &QNetworkReply::finished, this, &YouTube::onReplyChannelLivePage);
        }
        else
        {
            _info.broadcastId = extractBroadcastId(link);
        }

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
            _settings->setValue(_settingsGroupPath + "/" + SettingsKeyUserSpecifiedLink, _info.userSpecified);
        }

        if (preConnected && !preBroadcastId.isEmpty())
        {
            emit disconnected(preBroadcastId);
        }

        _info.detailedInformation += tr("Broadcast Short URL: %1").arg(_info.broadcastShortUrl.toString()) + "\n";
        _info.detailedInformation += tr("Broadcast Long URL: %1").arg(_info.broadcastLongUrl.toString()) + "\n";
        _info.detailedInformation += tr("Chat URL: %1").arg(_info.broadcastChatUrl.toString()) + "\n";
        _info.detailedInformation += tr("Control Panel URL: %1").arg(_info.broadcastChatUrl.toString()) + "\n";

        emit detailedInformationChanged();
    }

    emit stateChanged();

    onTimeoutRequestChat();
    onTimeoutRequestStreamPage();
}

void YouTube::onTimeoutRequestChat()
{
    if (_info.broadcastChatUrl.isEmpty())
    {
        return;
    }

    QNetworkRequest request(_info.broadcastChatUrl);
    request.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader, AxelChat::UserAgentNetworkHeaderName);
    request.setRawHeader("accept-language", AcceptLanguageNetworkHeaderName);
    QNetworkReply* reply = _manager.get(request);
    if (!reply)
    {
        qDebug() << Q_FUNC_INFO << ": !reply";
        return;
    }

    QObject::connect(reply, &QNetworkReply::finished, this, &YouTube::onReplyChatPage);
}

void YouTube::onReplyChatPage()
{
    QNetworkReply *reply = dynamic_cast<QNetworkReply*>(sender());
    if (!reply)
    {
        qDebug() << "!reply";
        return;
    }

    //_traffic += reply->size();
    //qDebug() << _traffic / 1024.0 / 1024.0 << "MB";
    //emit stateChanged();

    const QByteArray rawData = reply->readAll();
    if (rawData.isEmpty())
    {
        processBadChatReply();
        qDebug() << Q_FUNC_INFO << ":rawData is empty";
        return;
    }

    const QString startData = QString::fromUtf8(rawData.left(100));

    if (startData.contains("<title>Oops</title>", Qt::CaseSensitivity::CaseInsensitive))
    {
        //ToDo: show message "bad url"
        processBadChatReply();
        return;
    }

    const int start = rawData.indexOf("\"actions\":[");
    if (start == -1)
    {
        qDebug() << Q_FUNC_INFO << ": not found actions";
        AxelChat::saveDebugDataToFile(FolderLogs, "not_found_actions_from_html_youtube.html", rawData);
        processBadChatReply();
        return;
    }

    QByteArray data = rawData.mid(start + 10);
    const int pos = data.lastIndexOf(",\"actionPanel\"");
    if (pos != -1)
    {
        data = data.remove(pos, data.length());
    }

    //saveDataToFile(FolderLogs, "last_youtube.json", data);

    const QJsonDocument jsonDocument = QJsonDocument::fromJson(data);
    if (jsonDocument.isArray())
    {
        const QJsonArray actionsArray = jsonDocument.array();
        //qDebug() << "array size = " << actionsArray.size();
        parseActionsArray(actionsArray, data);
        _badChatReplies = 0;
    }
    else
    {
        printData(Q_FUNC_INFO + QString(": document is not array"), data);

        AxelChat::saveDebugDataToFile(FolderLogs, "failed_to_parse_from_html_youtube.html", rawData);
        AxelChat::saveDebugDataToFile(FolderLogs, "failed_to_parse_from_html_youtube.json", data);
        processBadChatReply();
    }
}

void YouTube::onReplyChannelLivePage()
{
    QNetworkReply *reply = dynamic_cast<QNetworkReply*>(sender());
    if (!reply)
    {
        qDebug() << "!reply";
        return;
    }

    //_traffic += reply->size();
    emit stateChanged();

    for (const QNetworkReply::RawHeaderPair& pair : reply->rawHeaderPairs())
    {
        qDebug() << pair.first << "=" << pair.second;
    }

    const QUrl possibleRedirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    if (possibleRedirectUrl.isValid())
    {
        qDebug() << possibleRedirectUrl;
    }
    else
    {
        qWarning() << Q_FUNC_INFO << "invalid redirect url";
    }

    const QByteArray rawData = reply->readAll();
    if (rawData.isEmpty())
    {
        qDebug() << Q_FUNC_INFO << ":rawData is empty";
        return;
    }

    qDebug() << rawData;
}

void YouTube::onTimeoutRequestStreamPage()
{
    //return;//ToDo:
    if (_info.broadcastLongUrl.isEmpty())
    {
        return;
    }

    QNetworkRequest request(_info.broadcastLongUrl);
    request.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader, AxelChat::UserAgentNetworkHeaderName);
    request.setRawHeader("accept-language", AcceptLanguageNetworkHeaderName);

    QNetworkReply* reply = _manager.get(request);
    if (!reply)
    {
        qDebug() << Q_FUNC_INFO << ": !reply";
        return;
    }

    QObject::connect(reply, &QNetworkReply::finished, this, &YouTube::onReplyStreamPage);
}

void YouTube::onReplyStreamPage()
{
    QNetworkReply *reply = dynamic_cast<QNetworkReply*>(sender());
    if (!reply)
    {
        qDebug() << "!reply";
        return;
    }

    //_traffic += reply->size();
    //emit stateChanged();

    const QByteArray rawData = reply->readAll();
    if (rawData.isEmpty())
    {
        qDebug() << Q_FUNC_INFO << ":rawData is empty";
        processBadLivePageReply();
        return;
    }

    if (parseViews(rawData))
    {
        _badLivePageReplies = 0;
    }
    else
    {
        processBadLivePageReply();
    }
}

void YouTube::parseActionsArray(const QJsonArray& array, const QByteArray& data)
{
    //AxelChat::saveDebugDataToFile(FolderLogs, "debug.json", data);

    if (!_info.broadcastConnected && !_info.broadcastId.isEmpty())
    {
        qDebug() << "YouTube connected" << _info.broadcastId;
        _info.broadcastConnected = true;

        emit connected(_info.broadcastId);
        emit stateChanged();
    }

    QList<ChatMessage> messages;

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
        QSet<ChatMessage::Flags> flags;
        QHash<ChatMessage::ForcedColorRoles, QColor> forcedColors;

        const QJsonObject& actionObject = actionJson.toObject();

        if (actionObject.contains("addChatItemAction"))
        {
            //Message

            const QJsonObject& addChatItemAction = actionObject.value("addChatItemAction").toObject();
            const QJsonObject& item = addChatItemAction.value("item").toObject();

            QJsonObject itemRenderer;

            if (item.contains("liveChatTextMessageRenderer"))
            {
                itemRenderer = item.value("liveChatTextMessageRenderer").toObject();

                valid = true;
            }
            else if (item.contains("liveChatPaidMessageRenderer"))
            {
                itemRenderer = item.value("liveChatPaidMessageRenderer").toObject();

                flags.insert(ChatMessage::Flags::DonateWithText);

                valid = true;
            }
            else if (item.contains("liveChatPaidStickerRenderer"))
            {
                //ToDo:
                itemRenderer = item.value("liveChatPaidStickerRenderer").toObject();

                flags.insert(ChatMessage::Flags::DonateWithImage);

                valid = true;
            }
            else if (item.contains("liveChatMembershipItemRenderer"))
            {
                itemRenderer = item.value("liveChatMembershipItemRenderer").toObject();

                flags.insert(ChatMessage::Flags::YouTubeChatMembership);

                valid = true;
            }
            else if (item.contains("liveChatViewerEngagementMessageRenderer"))
            {
                QJsonObject itemRenderer_ = item.value("liveChatViewerEngagementMessageRenderer").toObject();
                const QString iconType = itemRenderer_.value("icon").toObject().value("iconType").toString().trimmed().toUpper();
                if (iconType == "POLL")
                {
                    itemRenderer = std::move(itemRenderer_);
                    flags.insert(ChatMessage::Flags::PlatformGeneric);
                    valid = true;
                }
                else if (iconType == "YOUTUBE_ROUND")
                {
                    // ignore
                    valid = false;
                }
                else if (iconType.isEmpty())
                {
                    printData(Q_FUNC_INFO + QString(": iconType is empty of object \"%1\"").arg(iconType).arg("liveChatViewerEngagementMessageRenderer"), data);
                    valid = false;
                }
                else
                {
                    printData(Q_FUNC_INFO + QString(": Unknown iconType \"%1\" of object \"%2\"").arg(iconType).arg("liveChatViewerEngagementMessageRenderer"), data);
                    valid = false;
                }
            }
            else if (item.contains("liveChatPlaceholderItemRenderer") ||
                     item.contains("liveChatModeChangeMessageRenderer"))
            {
                // ignore
                valid = false;
            }
            else
            {
                AxelChat::saveDebugDataToFile(FolderLogs, "unknown_item_structure.json", QJsonDocument(item).toJson());
                valid = false;
            }

            messageId = itemRenderer.value("id").toString();

            publishedAt = QDateTime::fromMSecsSinceEpoch(
                        itemRenderer.value("timestampUsec").toString().toLongLong() / 1000,
                        Qt::TimeSpec::UTC).toLocalTime();

            tryAppedToText(authorName, itemRenderer, "authorName", false);

            tryAppedToText(messageText, itemRenderer, "purchaseAmountText", true);
            tryAppedToText(messageText, itemRenderer, "headerPrimaryText",  true);
            tryAppedToText(messageText, itemRenderer, "headerSubtext",      false);
            tryAppedToText(messageText, itemRenderer, "text",               true);
            tryAppedToText(messageText, itemRenderer, "subtext",            false);
            tryAppedToText(messageText, itemRenderer, "message",            false);

            authorChannelId = itemRenderer
                    .value("authorExternalChannelId").toString();

            const QJsonArray& authorPhotoThumbnails = itemRenderer
                    .value("authorPhoto").toObject()
                    .value("thumbnails").toArray();

            if (!authorPhotoThumbnails.isEmpty())
            {
                authorAvatarUrl = createResizedAvatarUrl(authorPhotoThumbnails.first().toObject().value("url").toString(), 300);
            }

            if (itemRenderer.contains("authorBadges"))
            {
                const QJsonArray& authorBadges = itemRenderer.value("authorBadges").toArray();

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
                                authorBadgeUrl = createResizedAvatarUrl(QUrl(thumbnailJson.toObject().value("url").toString()), _badgePixelSize);
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

            const QJsonArray& stickerThumbnails = itemRenderer
                    .value("sticker").toObject()
                    .value("thumbnails").toArray();

            if (!stickerThumbnails.isEmpty())
            {
                const QString stickerUrl = createResizedAvatarUrl(stickerThumbnails.first().toObject().value("url").toString(), _stickerSize).toString();

                if (!messageText.isEmpty())
                {
                    messageText += "<br>";
                }

                messageText += QString(" <img align=\"top\" height=\"%1\" width=\"%1\" src=\"%2\"> ").arg(_stickerSize).arg(stickerUrl);
            }

            //Other colors: headerBackgroundColor headerTextColor bodyBackgroundColor bodyTextColor authorNameTextColor timestampColor backgroundColor moneyChipBackgroundColor moneyChipTextColor

            if (itemRenderer.contains("bodyBackgroundColor"))
            {
                // usually for liveChatPaidMessageRenderer
                forcedColors.insert(ChatMessage::ForcedColorRoles::BodyBackgroundForcedColorRole, itemRenderer.value("bodyBackgroundColor").toVariant().toLongLong());
            }

            if (itemRenderer.contains("backgroundColor"))
            {
                // usually for liveChatPaidStickerRenderer
                forcedColors.insert(ChatMessage::ForcedColorRoles::BodyBackgroundForcedColorRole, itemRenderer.value("backgroundColor").toVariant().toLongLong());
            }

            if (itemRenderer.contains("headerBackgroundColor"))
            {
                // usually for liveChatPaidStickerRenderer
                forcedColors.insert(ChatMessage::ForcedColorRoles::BodyBackgroundForcedColorRole, itemRenderer.value("headerBackgroundColor").toVariant().toLongLong());
            }
        }
        else if (actionObject.contains("markChatItemAsDeletedAction"))
        {
            //Deleted message by unknown

            const QJsonObject& markChatItemAsDeletedAction = actionObject.value("markChatItemAsDeletedAction").toObject();

            tryAppedToText(messageText, markChatItemAsDeletedAction, "deletedStateMessage", false);

            messageId = markChatItemAsDeletedAction.value("targetItemId").toString();

            isDeleter = true;
            valid = true;
        }
        else if (actionObject.contains("markChatItemsByAuthorAsDeletedAction"))
        {
            //ToDo: Это событие удаляет все сообщения этого пользователя
            //Deleted message by author

            const QJsonObject& markChatItemsByAuthorAsDeletedAction = actionObject.value("markChatItemsByAuthorAsDeletedAction").toObject();

            tryAppedToText(messageText, markChatItemsByAuthorAsDeletedAction, "deletedStateMessage", false);

            messageId = markChatItemsByAuthorAsDeletedAction.value("targetItemId").toString();

            isDeleter = true;
            valid = true;
        }
        else if (actionObject.contains("replaceChatItemAction"))
        {
            //ToDo
            //qDebug() << Q_FUNC_INFO << QString(": object \"replaceChatItemAction\" not supported yet");
        }
        else if (actionObject.contains("addLiveChatTickerItemAction"))
        {
            //ToDo
            //qDebug() << Q_FUNC_INFO << QString(": object \"addLiveChatTickerItemAction\" not supported yet");
        }
        else if (actionObject.contains("addToPlaylistCommand") ||
                 actionObject.contains("clickTrackingParams") ||
                 actionObject.contains("addBannerToLiveChatCommand"))
        {
            // ignore
        }
        else
        {
            QJsonDocument doc(actionObject);
            //printData(Q_FUNC_INFO + QString(": unknown json structure of array \"%1\"").arg("actions"), doc.toJson());
            AxelChat::saveDebugDataToFile(FolderLogs, "unknown_action_structure.json", doc.toJson());
        }

        ChatMessage::trimCustom(messageText);

        if (valid)
        {
            if (publishedAt.isValid() && _needRemoveBeforeAt.isValid())
            {
                if (publishedAt < _needRemoveBeforeAt)
                {
                    continue;
                }
            }

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
                            author,
                            flags,
                            forcedColors);

                messages.append(message);

                _messagesReceived++;
            }
        }
    }

    emit readyRead(messages);
    emit stateChanged();
}

bool YouTube::parseViews(const QByteArray &rawData)
{
    static const QByteArray Prefix = "{\"videoViewCountRenderer\":{\"viewCount\":{\"runs\":[";

    const int start = rawData.indexOf(Prefix);
    if (start == -1)
    {
        if (rawData.contains("videoViewCountRenderer\":{\"viewCount\":{}"))
        {
            _info.viewers = 0;
            emit stateChanged();
            return false;
        }

        qDebug() << Q_FUNC_INFO << ": failed to parse videoViewCountRenderer";
        AxelChat::saveDebugDataToFile(FolderLogs, "failed_to_parse_videoViewCountRenderer_from_html_youtube.html", rawData);
        return false;
    }

    int lastPos = -1;
    for (int i = start + Prefix.length(); i < rawData.length(); ++i)
    {
        const QChar& c = rawData[i];
        if (c == ']')
        {
            lastPos = i;
            break;
        }
    }

    if (lastPos == -1)
    {
        qDebug() << Q_FUNC_INFO << ": not found ']'";
        return false;
    }

    const QByteArray data = rawData.mid(start + Prefix.length(), lastPos - (start + Prefix.length()));
    const QByteArray digits = extractDigitsOnly(data);
    if (digits.isEmpty())
    {
        printData(Q_FUNC_INFO + QString(": failed to find digits"), data);

        AxelChat::saveDebugDataToFile(FolderLogs, "failed_to_parse_from_html_no_digits_youtube.html", rawData);
        AxelChat::saveDebugDataToFile(FolderLogs, "failed_to_parse_from_html_no_digits_youtube.json", data);
        return false;
    }

    bool ok = false;
    const int viewers = digits.toInt(&ok);
    if (!ok)
    {
        printData(Q_FUNC_INFO + QString(": failed to convert to number"), data);

        AxelChat::saveDebugDataToFile(FolderLogs, "failed_to_parse_from_html_fail_to_convert_to_number_youtube.html", rawData);
        AxelChat::saveDebugDataToFile(FolderLogs, "failed_to_parse_from_html_fail_to_convert_to_number_youtube.json", data);
        return false;
    }

    _info.viewers = viewers;

    emit stateChanged();

    return true;
}

void YouTube::processBadChatReply()
{
    _badChatReplies++;

    if (_badChatReplies >= MaxBadChatReplies)
    {
        if (_info.broadcastConnected && !_info.broadcastId.isEmpty())
        {
            qWarning() << Q_FUNC_INFO << "too many bad chat replies! Disonnecting...";

            const QString preBroadcastId = _info.broadcastId;
            const QString preUserSpecified = _info.userSpecified;

            _info = AxelChat::YouTubeInfo();

            _info.broadcastConnected = false;

            emit disconnected(preBroadcastId);

            setLink(preUserSpecified);
        }
    }
}

void YouTube::processBadLivePageReply()
{
    _badLivePageReplies++;

    if (_badLivePageReplies >= MaxBadLivePageReplies)
    {
        qWarning() << Q_FUNC_INFO << "too many bad live page replies!";
        _info.viewers = -1;

        emit stateChanged();
    }
}

void YouTube::tryAppedToText(QString &text, const QJsonObject &jsonObject, const QString &varName, bool bold) const
{
    if (!jsonObject.contains(varName))
    {
        return;
    }

    QString newText = parseText(jsonObject.value(varName).toObject());
    ChatMessage::trimCustom(newText);
    if (newText.isEmpty())
    {
        return;
    }

    if (bold)
    {
        newText = QString("<b>") + newText + "</b>";
    }

    if (!text.isEmpty())
    {
        text += "<br>";
    }

    text += newText;
}

QString YouTube::parseText(const QJsonObject &message) const
{
    QString result;

    if (message.contains("simpleText"))
    {
        QString text_ = message.value("simpleText").toString();
        if (text_ == "\n")
        {
            text_ = "<br>";
        }

        result += text_;
    }

    if (message.contains("runs"))
    {
        if (!result.isEmpty() && !result.endsWith("<br>") && !result.endsWith("\n"))
        {
            result += "<br>";
        }

        const QJsonArray& runs = message.value("runs").toArray();

        foreach (const QJsonValue& run, runs)
        {
            const QJsonObject& runObject = run.toObject();
            if (runObject.contains("navigationEndpoint"))
            {
                QString url = runObject.value("navigationEndpoint").toObject()
                        .value("commandMetadata").toObject()
                        .value("webCommandMetadata").toObject()
                        .value("url").toString();
                const QString& text = runObject.value("text").toString();

                if (!url.isEmpty() && !text.isEmpty())
                {
                    if (url.startsWith("/"))
                    {
                        url = "https://www.youtube.com" + url;
                    }

                    result += "<a href=\"" + url + "\">" + text + "</a>";
                    continue;
                }
                else
                {
                    AxelChat::saveDebugDataToFile(FolderLogs, "unknown_message_runs_unknown_navigationEndpoint_structure.json", QJsonDocument(message).toJson());
                }
            }

            if (runObject.contains("text"))
            {
                QString text_ = runObject.value("text").toString();
                if (text_ == "\n")
                {
                    text_ = "<br>";
                }

                result += text_;
            }
            else if (runObject.contains("emoji"))
            {
                const QJsonArray& thumbnails = runObject.value("emoji").toObject()
                        .value("image").toObject()
                        .value("thumbnails").toArray();

                if (!thumbnails.isEmpty())
                {
                    //ToDo: можно использовать большие эмодзи
                    const QString empjiUrl = thumbnails.first().toObject().value("url").toString();
                    if (!empjiUrl.isEmpty())
                    {
                        result += QString(" <img align=\"top\" height=\"%1\" width=\"%1\" src=\"%2\"> ").arg(_emojiPixelSize).arg(empjiUrl);
                    }
                    else
                    {
                        AxelChat::saveDebugDataToFile(FolderLogs, "unknown_message_runs_empty_emoji_structure.json", QJsonDocument(message).toJson());
                    }
                }
                else
                {
                    AxelChat::saveDebugDataToFile(FolderLogs, "unknown_message_runs_empty_thumbnails_structure.json", QJsonDocument(message).toJson());
                }
            }
            else
            {
                AxelChat::saveDebugDataToFile(FolderLogs, "unknown_message_runs_empty_thumbnails_structure.json", QJsonDocument(message).toJson());
            }
        }
    }

    return result;
}

QColor YouTube::intToColor(quint64 rawColor) const
{
    qDebug() << "rawColor" << rawColor;
    return QColor::fromRgba64(QRgba64::fromRgba64(rawColor));
}
