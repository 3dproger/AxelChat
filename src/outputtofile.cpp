#include "outputtofile.hpp"
#include "models/chatauthor.h"
#include "models/chatmessage.h"
#include <QStandardPaths>
#include <QGuiApplication>
#include <QTextCodec>
#include <QDesktopServices>
#include <QTimeZone>
#include <QDir>
#include <QTextCodec>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QImage>
#include <QImageReader>
#include "utils_axelchat.hpp"

namespace
{

static const QString SK_Enabled                     = "enabled";
static const QString SK_OutputFolder                = "output_folder";
static const QString SK_Codec                       = "codec";
static const QString SK_YouTubeLastMessageSavedId   = "youtube_last_saved_message_id";

static const QString DateTimeFileNameFormat = "yyyy-MM-ddThh-mm-ss.zzz";
static const QString MessagesFileName = "messages.ini";
static const QString MessagesCountFileName = "messages_count.txt";
static const QString YouTubeLastMessageId = "youtube_last_message_id.txt";

int getStatusCode(const QNetworkReply& reply)
{
    QVariant statusCode = reply.attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if (statusCode.isValid())
    {
        bool ok = false;
        const int result = statusCode.toInt(&ok);
        if (ok)
        {
            return result;
        }
    }

    return -1;
}

}

OutputToFile::OutputToFile(QSettings &settings_, const QString &settingsGroupPath, QNetworkAccessManager& network_, const ChatMessagesModel& messagesModel_, QObject *parent)
    : QObject(parent)
    , settings(settings_)
    , SettingsGroupPath(settingsGroupPath)
    , network(network_)
    , messagesModel(messagesModel_)
{
    reinit(true);

    setEnabled(settings.value(SettingsGroupPath + "/" + SK_Enabled, false).toBool());

    setOutputFolder(settings.value(SettingsGroupPath + "/" + SK_OutputFolder, standardOutputFolder()).toString());

    setCodecOption(settings.value(SettingsGroupPath + "/" + SK_Codec, _codec).toInt(), true);
}

OutputToFile::~OutputToFile()
{
    if (_enabled)
    {
        if (_iniCurrentInfo)
        {
            _iniCurrentInfo->setValue("software/started", false);
        }
    }

    if (_fileMessages && _fileMessages->isOpen())
    {
        _fileMessages->close();
    }

    if (_fileMessagesCount && _fileMessagesCount->isOpen())
    {
        _fileMessagesCount->close();
    }
}

bool OutputToFile::enabled() const
{
    return _enabled;
}

void OutputToFile::setEnabled(bool enabled)
{
    if (_enabled != enabled)
    {
        _enabled = enabled;

        settings.setValue(SettingsGroupPath + "/" + SK_Enabled, enabled);

        //qDebug(QString("OutputToFile: %1").arg(_enabled ? "enabled" : "disabled").toUtf8());

        if (_enabled)
        {
            reinit(false);
        }

        emit enabledChanged();
    }
}

QString OutputToFile::standardOutputFolder() const
{
    return QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/" + QGuiApplication::applicationName() + "/output";
}

QString OutputToFile::outputFolder() const
{
    return _outputFolder;
}

void OutputToFile::setOutputFolder(QString outputFolder)
{
    outputFolder = outputFolder.trimmed();

    if (_outputFolder != outputFolder)
    {
        _outputFolder = outputFolder;

        settings.setValue(SettingsGroupPath + "/" + SK_OutputFolder, outputFolder);

        reinit(true);

        //qDebug(QString("OutputToFile: outputFolder: \"%1\"").arg(_outputFolder).toUtf8());

        emit outputFolderChanged();
    }
}

void OutputToFile::resetSettings()
{
    setOutputFolder(standardOutputFolder());
}

void OutputToFile::writeMessages(const QList<ChatMessage>& messages)
{
    if (!_enabled)
    {
        return;
    }

    int firstValidMessage = 0;

    if (!_youTubeLastMessageId.isEmpty())
    {
        for (int i = 0; i < messages.count(); ++i)
        {
            const ChatMessage& message = messages[i];

            if (message.id() == _youTubeLastMessageId)
            {
                qDebug() << "found youtube message with id" << message.id() << ", ignore saving messages before it, index =" << i;
                firstValidMessage = i;
                _youTubeLastMessageId.clear();
                break;
            }
        }
    }

    QString currentLastYouTubeMessageId;

    for (int i = firstValidMessage; i < messages.count(); ++i)
    {
        const ChatMessage& message = messages[i];

        const QString authorId = message.authorId();
        const ChatAuthor* author = messagesModel.getAuthor(authorId);
        if (!author)
        {
            qWarning() << "Not found author id" << message.authorId();
            continue;
        }

        const AbstractChatService::ServiceType type = author->getServiceType();

        if (type == AbstractChatService::ServiceType::Unknown ||
            type == AbstractChatService::ServiceType::Test)
        {
            continue;
        }

        QList<QPair<QString, QString>> tags;

        tags.append(QPair<QString, QString>("author", author->name()));
        tags.append(QPair<QString, QString>("author_id", authorId));
        tags.append(QPair<QString, QString>("message", message.text()));
        tags.append(QPair<QString, QString>("time", message.publishedAt().toTimeZone(QTimeZone::systemTimeZone()).toString(Qt::DateFormat::ISODateWithMs)));
        tags.append(QPair<QString, QString>("service", AbstractChatService::serviceTypeToString(type)));

        writeMessage(tags);

        if (author->getServiceType() == AbstractChatService::ServiceType::YouTube)
        {
            const QString id = message.id();
            if (!id.isEmpty())
            {
                currentLastYouTubeMessageId = id;
            }
        }

        switch (type)
        {
        case AbstractChatService::ServiceType::GoodGame:
        case AbstractChatService::ServiceType::YouTube:
        case AbstractChatService::ServiceType::Twitch:
            tryDownloadAvatar(authorId, author->avatarUrl(), type);
            saveAuthorInfo(*author);
            break;

        case AbstractChatService::ServiceType::Unknown:
        case AbstractChatService::ServiceType::Software:
        case AbstractChatService::ServiceType::Test:
            break;
        }
    }

    if (!currentLastYouTubeMessageId.isEmpty())
    {
        settings.setValue(SettingsGroupPath + "/" + SK_YouTubeLastMessageSavedId, currentLastYouTubeMessageId);
    }
}

void OutputToFile::showInExplorer()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(QString("file:///") + _outputFolder));
}

void OutputToFile::saveAuthorInfo(const ChatAuthor& author)
{
    const QString fileName = getAuthorDirectory(author.getServiceType(), author.authorId()) + "/info.ini";
}

void OutputToFile::tryDownloadAvatar(const QString& authorId, const QUrl& url, const AbstractChatService::ServiceType service)
{
    if (url.isEmpty() || downloadedAvatarsAuthorId.contains(authorId) || !_enabled)
    {
        return;
    }

    const QString urlStr = url.toString();
    if (!urlStr.contains('/'))
    {
        qWarning() << "Url not contains '/'";
        return;
    }

    const QString avatarName = urlStr.mid(urlStr.lastIndexOf('/') + 1);

    //qDebug() << "Load avatar " << avatarName << "from" << urlStr;

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader, AxelChat::UserAgentNetworkHeaderName);
    QNetworkReply* reply = network.get(request);
    connect(reply, &QNetworkReply::finished, this, [this, service, authorId, avatarName]()
    {
        QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
        if (!reply)
        {
            return;
        }

        if (reply->bytesAvailable() <= 0)
        {
            qDebug() << "Failed download, reply is empty, status code =" << reply->errorString() << " avatar =" << avatarName;
            return;
        }

        QByteArray format;

        {
            format = QImageReader(reply).format();
        }

        format = format.trimmed().toLower();

        if (format.isEmpty())
        {
            qDebug() << "Failed to detect avatar format" << avatarName;
            return;
        }

        const QString avatarsDirectory = getAuthorDirectory(service, authorId) + "/avatars";
        QDir dir(avatarsDirectory);
        if (!dir.exists())
        {
            if (!dir.mkpath(avatarsDirectory))
            {
                qDebug() << "Failed to make path" << avatarsDirectory;
            }
        }

        QString fileName = avatarsDirectory + "/" + avatarName;

        if (!avatarName.endsWith("." + format, Qt::CaseSensitivity::CaseInsensitive))
        {
            fileName += "." + format;
        }

        QFile file(fileName);
        if (file.open(QFile::OpenModeFlag::WriteOnly))
        {
            file.write(reply->readAll());
            file.close();

            //qDebug() << "Saved avatar" << avatarName;
        }
        else
        {
            qDebug() << "Failed to save avatar" << fileName;
        }

        downloadedAvatarsAuthorId.insert(authorId);

        reply->deleteLater();
    });
}

bool OutputToFile::setCodecOption(int option, bool applyWithoutReset)
{
    if (option == _codec)
    {
        return false;
    }

    if (applyWithoutReset)
    {
        switch (option) {
        case 0:
            _codec = OutputToFileCodec::UTF8Codec;
            break;
        case 100:
            _codec = OutputToFileCodec::ANSICodec;
            break;
        case 200:
            _codec = OutputToFileCodec::ANSIWithUTF8Codec;
            break;
        default:
            qCritical() << "unknown codec option" << option << ", ignore";
            return false;
        }

        reinit(true);
    }

    settings.setValue(SettingsGroupPath + "/" + SK_Codec, option);

    return true;
}

int OutputToFile::codecOption() const
{
    return _codec;
}

void OutputToFile::writeMessage(const QList<QPair<QString, QString>> tags /*<tagName, tagValue>*/)
{
    if (!_fileMessages)
    {
        qDebug() << Q_FUNC_INFO << "!_fileMessages";
        return;
    }

    if (!_fileMessagesCount)
    {
        qDebug() << Q_FUNC_INFO << "!_fileMessagesStatistics";
        return;
    }

    if (!_fileMessages->isOpen())
    {
        if (!_fileMessages->open(QIODevice::OpenModeFlag::Text | QIODevice::OpenModeFlag::Append))
        {
            qWarning() << Q_FUNC_INFO << "failed to open file" << _fileMessages->fileName() << ":" << _fileMessages->errorString();
            return;
        }
    }

    if (!_fileMessagesCount->isOpen())
    {
        if (!_fileMessagesCount->open(QIODevice::OpenModeFlag::Text | QIODevice::OpenModeFlag::ReadWrite))
        {
            qWarning() << Q_FUNC_INFO << "failed to open file" << _fileMessagesCount->fileName() << ":" << _fileMessagesCount->errorString();
            return;
        }
    }

    int currentCount = 0;

    _fileMessagesCount->seek(0);
    const QByteArray countData = _fileMessagesCount->read(256);

    bool ok = false;
    currentCount = countData.trimmed().toInt(&ok);
    if (!ok || currentCount < 0)
    {
        currentCount = 0;
    }

    QByteArray data = "\n[" + QByteArray::number(currentCount) + "]\n";

    for (int i = 0; i < tags.count(); ++i)
    {
        const QPair<QString, QString>& tag = tags[i];
        data += tag.first.toLatin1() + "=" + prepare(tag.second) + "\n";
    }

    data += "\n";

    _fileMessages->write(data);

    if (!_fileMessages->flush())
    {
        qWarning() << "failed to flush file" << _fileMessages->fileName();
    }

    currentCount++;

    _fileMessagesCount->seek(0);
    _fileMessagesCount->write(QString("%1").arg(currentCount).toUtf8());
    if (!_fileMessagesCount->flush())
    {
        qWarning() << "failed to flush file" << _fileMessagesCount->fileName();
    }
}

QByteArray OutputToFile::prepare(const QString &text_)
{
    QString text = text_;

    if (_codec != OutputToFileCodec::ANSIWithUTF8Codec)
    {
        text.replace('\n', "\\n");
        text.replace('\r', "\\r");
    }

    if (_codec == OutputToFileCodec::UTF8Codec)
    {
        return text.toUtf8();
    }
    else if (_codec == OutputToFileCodec::ANSICodec)
    {
        return text.toLocal8Bit();
    }
    else if (_codec == OutputToFileCodec::ANSIWithUTF8Codec)
    {
        return convertANSIWithUtf8Numbers(text);
    }

    qWarning() << Q_FUNC_INFO << "unknown codec";

    return text.toUtf8();
}

QString OutputToFile::getAuthorDirectory(const AbstractChatService::ServiceType serviceType, const QString &authorId) const
{
    return _outputFolder + "/authors/" + AbstractChatService::serviceTypeToString(serviceType) + "/" + authorId;
}

void OutputToFile::reinit(bool forceUpdateOutputFolder)
{
    //Messages
    if (_fileMessages)
    {
        _fileMessages->close();
        _fileMessages->deleteLater();
        _fileMessages = nullptr;
    }

    if (_fileMessagesCount)
    {
        _fileMessagesCount->close();
        _fileMessagesCount->deleteLater();
        _fileMessagesCount = nullptr;
    }

    if (forceUpdateOutputFolder || _sessionFolder.isEmpty())
    {
        _sessionFolder = _outputFolder + "/sessions/" + _startupDateTime.toString(DateTimeFileNameFormat);
    }

    if (_enabled)
    {
        QDir dir;

        dir = QDir(_sessionFolder);
        if (!dir.exists())
        {
            dir.mkpath(_sessionFolder);
        }
    }

    _fileMessages               = new QFile(_sessionFolder + "/" + MessagesFileName,       this);
    _fileMessagesCount          = new QFile(_sessionFolder + "/" + MessagesCountFileName,  this);

    //Current
    if (_iniCurrentInfo)
    {
        _iniCurrentInfo->sync();
        _iniCurrentInfo->deleteLater();
        _iniCurrentInfo = nullptr;
    }

    _iniCurrentInfo = new QSettings(_outputFolder + "/current.ini", QSettings::IniFormat, this);
    _iniCurrentInfo->setIniCodec("UTF-8");

    if (_enabled)
    {
        _youTubeLastMessageId = settings.value(SettingsGroupPath + "/" + SK_YouTubeLastMessageSavedId).toString();

        _iniCurrentInfo->setValue("software/started", true);
    }

    writeStartupInfo(_sessionFolder);
    writeInfo();
}

void OutputToFile::writeStartupInfo(const QString& messagesFolder)
{
    if (_enabled)
    {
        _iniCurrentInfo->setValue("software/version",                   QCoreApplication::applicationVersion());

        _iniCurrentInfo->setValue("software/current_messages_folder",   messagesFolder);

        _iniCurrentInfo->setValue("software/startup_time",              _startupDateTime.toTimeZone(QTimeZone::systemTimeZone()).toString(Qt::DateFormat::ISODateWithMs));
    }
}

void OutputToFile::writeInfo()
{
    if (_enabled)
    {
        _iniCurrentInfo->setValue("youtube/broadcast_connected", _youTubeInfo.broadcastConnected);
        _iniCurrentInfo->setValue("youtube/broadcast_id", _youTubeInfo.broadcastId);
        _iniCurrentInfo->setValue("youtube/broadcast_user_specified", _youTubeInfo.userSpecified);
        _iniCurrentInfo->setValue("youtube/broadcast_url", _youTubeInfo.broadcastLongUrl.toString());
        _iniCurrentInfo->setValue("youtube/broadcast_chat_url", _youTubeInfo.broadcastChatUrl.toString());
        _iniCurrentInfo->setValue("youtube/broadcast_control_panel_url", _youTubeInfo.controlPanelUrl.toString());
        _iniCurrentInfo->setValue("youtube/viewers_count", _youTubeInfo.viewers);

        _iniCurrentInfo->setValue("twitch/broadcast_connected", _twitchInfo.connected);
        _iniCurrentInfo->setValue("twitch/channel_name", _twitchInfo.channelLogin);
        _iniCurrentInfo->setValue("twitch/user_specified", _twitchInfo.userSpecifiedChannel);
        _iniCurrentInfo->setValue("twitch/channel_url", _twitchInfo.channelUrl.toString());
        _iniCurrentInfo->setValue("twitch/chat_url", _twitchInfo.chatUrl.toString());
        _iniCurrentInfo->setValue("twitch/control_panel_url", _twitchInfo.controlPanelUrl.toString());
        _iniCurrentInfo->setValue("twitch/viewers_count", _twitchInfo.viewers);
    }
}

void OutputToFile::setYouTubeInfo(const AxelChat::YouTubeInfo &youTubeCurrent)
{
    _youTubeInfo = youTubeCurrent;

    reinit(false);
}

void OutputToFile::setTwitchInfo(const AxelChat::TwitchInfo &twitchCurrent)
{
    _twitchInfo = twitchCurrent;

    reinit(false);
}

void OutputToFile::setGoodGameInfo(const AxelChat::GoodGameInfo &goodGameCurrent)
{
    _goodGameInfo = goodGameCurrent;

    reinit(false);
}
