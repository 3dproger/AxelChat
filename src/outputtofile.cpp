#include "outputtofile.hpp"
#include <QStandardPaths>
#include <QGuiApplication>
#include <QTextCodec>
#include <QDesktopServices>
#include <QTimeZone>
#include <QDir>
#include <QTextCodec>
#include <QDebug>
#include "utils_axelchat.hpp"

namespace
{


static const QString DateTimeFileNameFormat = "yyyy-MM-ddThh-mm-ss.zzz";
static const QString MessagesFileName = "messages.ini";
static const QString MessagesCountFileName = "count.txt";

static QString dateTimeToStr(const QDateTime& dateTime)
{
    static const QString DateTimeMessagePrepareFormat = "yyyy-MM-ddThh:mm:ss.zzz";
    return dateTime.toString(DateTimeMessagePrepareFormat) + "Z";
}

}

OutputToFile::OutputToFile(QSettings *settings, const QString &settingsGroupPath, QObject *parent) : QObject(parent)
{
    _settings = settings;
    _settingsGroupPath = settingsGroupPath;

    reinit(true);

    if (_settings)
    {
        setEnabled(_settings->value(_settingsGroupPath + "/" + _settingsKeyEnabled,
                                    false).toBool());

        setOutputFolder(_settings->value(_settingsGroupPath + "/" + _settingsKeyOutputFolder,
                standardOutputFolder()).toString());

        setCodecOption(_settings->value(_settingsGroupPath + "/" + _settingsKeyCodec,
                                  _codec).toInt(), true);
    }
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

    if (_fileMessages->isOpen())
    {
        _fileMessages->close();
    }

    if (_fileMessagesCount->isOpen())
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
        if (_settings)
        {
            _settings->setValue(_settingsGroupPath + "/" + _settingsKeyEnabled, enabled);
        }

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

        if (_settings)
        {
            _settings->setValue(_settingsGroupPath + "/" + _settingsKeyOutputFolder, outputFolder);
        }

        reinit(true);

        //qDebug(QString("OutputToFile: outputFolder: \"%1\"").arg(_outputFolder).toUtf8());

        emit outputFolderChanged();
    }
}

void OutputToFile::resetSettings()
{
    setOutputFolder(standardOutputFolder());
}

void OutputToFile::onMessagesReceived(const ChatMessage &message, const MessageAuthor& author)
{
    const ChatMessage::Type type = message.type();

    if (type == ChatMessage::Type::Unknown ||
        type == ChatMessage::Type::TestMessage)
    {
        return;
    }

    if (_enabled)
    {
        QList<QPair<QString, QString>> tags;

        {
            // author name

            tags.append(QPair<QString, QString>("author", author.name()));
        }

        {
            // message text

            tags.append(QPair<QString, QString>("message", message.text()));
        }

        {
            // time

            tags.append(QPair<QString, QString>("time", dateTimeToStr(message.publishedAt())));
        }

        {
            // user url

            tags.append(QPair<QString, QString>("user_url", message.author().pageUrl().toString()));
        }

        {
            // chat service

            QString chatServie;
            switch (type) {
            case ChatMessage::SoftwareNotification:
                chatServie = "softwarenotification";
                break;
            case ChatMessage::TestMessage:
                chatServie = "testmessage";
                break;
            case ChatMessage::YouTube:
                chatServie = "youtube";
                break;
            case ChatMessage::Twitch:
                chatServie = "twitch";
                break;
            case ChatMessage::Unknown:
            default:
                chatServie = "unknown";
                break;
            }
            tags.append(QPair<QString, QString>("chat_service", chatServie));
        }

        // chat-service specific data
        switch (type) {
        case ChatMessage::YouTube:
            tags.append(QPair<QString, QString>("youtube_channel_id", message.author().channelId()));
            break;

        case ChatMessage::Twitch:
            tags.append(QPair<QString, QString>("twitch_login", message.author().channelId()));
            break;

        case ChatMessage::Unknown:
        case ChatMessage::SoftwareNotification:
        case ChatMessage::TestMessage:
            break;
        }

        writeMessage(tags);
    }
}

void OutputToFile::showInExplorer()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(_outputFolder));
}

void OutputToFile::setCodecOption(int option, bool applyWithoutReset)
{
    if (option == _codec)
    {
        return;
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
            return;
        }

        reinit(true);
    }

    if (_settings)
    {
        _settings->setValue(_settingsGroupPath + "/" + _settingsKeyCodec, option);
    }
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

    if (forceUpdateOutputFolder || _messagesFolder.isEmpty())
    {
        _messagesFolder = _outputFolder + "/messages/" + _startupDateTime.toString(DateTimeFileNameFormat);
    }

    if (_enabled)
    {
        QDir dir = QDir(_messagesFolder);
        if (!dir.exists())
        {
            dir.mkpath(_messagesFolder);
        }
    }

    _fileMessages = new QFile(_messagesFolder + "/" + MessagesFileName, this);
    _fileMessagesCount = new QFile(_messagesFolder + "/" + MessagesCountFileName, this);
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
        _iniCurrentInfo->setValue("software/started", true);
    }

    writeStartupInfo(_messagesFolder);
    writeInfo();
}

void OutputToFile::writeStartupInfo(const QString& messagesFolder)
{
    if (_enabled)
    {
        _iniCurrentInfo->setValue("software/version",                   QCoreApplication::applicationVersion());

        _iniCurrentInfo->setValue("software/current_messages_folder",   messagesFolder);

        _iniCurrentInfo->setValue("software/startup_time",              dateTimeToStr(_startupDateTime));
        _iniCurrentInfo->setValue("software/startup_timestamp_utc",     QString("%1").arg(_startupDateTime.toMSecsSinceEpoch()));

        _iniCurrentInfo->setValue("software/startup_timezone_id",       QString::fromUtf8(_startupDateTime.timeZone().id()));
        _iniCurrentInfo->setValue("software/startup_timezone_offset_from_utc", QString("%1")
                              .arg(double(_startupDateTime.timeZone().standardTimeOffset(_startupDateTime)) / float(60 * 60)));
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