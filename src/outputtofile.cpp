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

static const QString DateTimeFormat = "yyyy-MM-ddThh:mm:ss.zzz";

}

OutputToFile::OutputToFile(QSettings *settings, const QString &settingsGroupPath, QObject *parent) : QObject(parent)
{
    _settings = settings;
    _settingsGroupPath = settingsGroupPath;

    reinitIni();

    if (_settings)
    {
        setEnabled(_settings->value(_settingsGroupPath + "/" + _settingsKeyEnabled,
                                    false).toBool());

        setOutputFolder(_settings->value(_settingsGroupPath + "/" + _settingsKeyOutputFolder,
                standardOutputFolder()).toString());

        setCodec(_settings->value(_settingsGroupPath + "/" + _settingsKeyCodec,
                                  _codec).toString());
    }
}

OutputToFile::~OutputToFile()
{
    if (_enabled)
    {
        if (_iniCurrent)
        {
            _iniCurrent->setValue("software/started", false);
        }
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
            reinitIni();
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

        reinitIni();

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
    //if (message.wherefrom.trimmed().toLower() == "software")
    if (message.type() == ChatMessage::Type::Unknown ||
        message.type() == ChatMessage::Type::SoftwareNotification ||
        message.type() == ChatMessage::Type::TestMessage)
    {
        return;
    }

    if (_enabled)
    {
        if (_iniMessages)
        {
            if (!_iniMessages->contains("statistic/count"))
            {
                _iniMessages->setValue("statistic/count", 0);
            }

            const QString& group = QString("%1").arg(_iniMessagesCount);

            _iniMessages->setValue(group + "/author",
                                   prepare(author.name()));

            _iniMessages->setValue(group + "/message",
                                   prepare(message.text()));

            _iniMessages->setValue(group + "/author_channel_id",
                                   prepare(message.author().channelId()));

            _iniMessages->setValue(group + "/time", message.publishedAt().toString(DateTimeFormat));

            _iniMessagesCount++;

            _iniMessages->setValue("statistic/count", _iniMessagesCount);
        }
    }
}

void OutputToFile::showInExplorer()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(_outputFolder));
}

QList<QString> OutputToFile::codecs() const
{
    /*QList<QByteArray> codecsBA = QTextCodec::availableCodecs();
    QList<QString> codecsString;
    for (const QByteArray& codec : codecsBA)
    {
        codecsString.append(codec);
    }
    return codecsString;*/

    return {
        "UTF-8",
        "windows-1251"
    };
}

void OutputToFile::setCodec(QString codec)
{
    codec = codec.trimmed();

    _codec = codec;

    reinitIni();

    if (_settings)
    {
        _settings->setValue(_settingsGroupPath + "/" + _settingsKeyCodec, codec);
    }
}

QString OutputToFile::codec() const
{
    return _codec;
}

bool OutputToFile::exportToTxt(QString filePath)
{
    filePath = filePath.trimmed();

    QString normalizedFilePath = filePath;
    if (normalizedFilePath.startsWith("file:///", Qt::CaseInsensitive))
    {
        normalizedFilePath.remove(0, 8);
    }

    QFile file(normalizedFilePath);

    if (!file.open(QIODevice::OpenModeFlag::ReadWrite | QIODevice::OpenModeFlag::Text))
    {
        qDebug() << file.errorString();
        return false;
    }

    QByteArray data;

    if (_iniMessages)
    {
        const int messagesCount = _iniMessages->value("statistic/count").toInt();

        QMap<QString, AuthorInfo> authors;//id, author

        QByteArray messagesData;

        for (int i = 0; i < messagesCount; i++)
        {
            const QString authorName  = _iniMessages->value(QString("%1/author").arg(i)).toString();
            const QString authorId    = _iniMessages->value(QString("%1/author_channel_id").arg(i)).toString();
            const QString messageText = _iniMessages->value(QString("%1/message").arg(i)).toString();

            if (!authorId.isEmpty())
            {
                if (!authors.contains(authorId))
                {
                    AuthorInfo author;
                    author.messagesCount = 1;
                    author.name = authorName;
                    author.youtubeUrl = QString("https://www.youtube.com/channel/%1").arg(authorId);

                    authors.insert(authorId, author);
                }
                else
                {
                    authors[authorId].messagesCount++;
                }
            }

            messagesData.append(QString("\n%1\n%2\n").arg(authorName).arg(messageText).toUtf8());
        }

        data.append(tr("Broadcast URL: %1").arg(_youTubeInfo.broadcastShortUrl.toString()).toUtf8() + "\n");
        data.append(tr("Messages count: %1").arg(messagesCount).toUtf8() + "\n");
        data.append(tr("Participants count: %1").arg(authors.count()).toUtf8() + "\n");
        data.append("==============================\n");
        data.append(tr("Participants:").toUtf8() + "\n");
        for (const AuthorInfo& authorInfo : authors)
        {
            data.append(QString("\n%1\n").arg(authorInfo.name).toUtf8());
            data.append(tr("Channel: %1").arg(authorInfo.youtubeUrl).toUtf8() + "\n");
            data.append(tr("Messages count: %1").arg(authorInfo.messagesCount).toUtf8() + "\n");
        }

        data.append("==============================\n");
        data.append(tr("Messages:").toUtf8() + "\n");
        data.append(messagesData);
    }

    file.write(data);
    file.close();

    return true;
}

void OutputToFile::reinitIni()
{
    //Messages
    _iniMessagesCount = 0;

    if (_iniMessages)
    {
        _iniMessages->sync();
        _iniMessages->deleteLater();
        _iniMessages = nullptr;
    }

    _broadcastFolder = "";

    if (!_youTubeInfo.broadcastId.isEmpty())
    {
        _broadcastFolder = _outputFolder + "/broadcasts/" + _youTubeInfo.broadcastId;

        if (_enabled)
        {
            QDir dir = QDir(_broadcastFolder);
            if (!dir.exists())
            {
                dir.mkpath(_broadcastFolder);
            }
        }

        _iniMessages = new QSettings(_broadcastFolder + "/messages.ini", QSettings::IniFormat, this);
        _iniMessages->setIniCodec(_codec.toUtf8());

        _iniMessagesCount = _iniMessages->value("statistic/count", 0).toInt();
    }

    //Current
    if (_iniCurrent)
    {
        _iniCurrent->sync();
        _iniCurrent->deleteLater();
        _iniCurrent = nullptr;
    }

    _iniCurrent = new QSettings(_outputFolder + "/current.ini", QSettings::IniFormat, this);
    _iniCurrent->setIniCodec(_codec.toUtf8());

    if (_enabled)
    {
        _iniCurrent->setValue("software/started", true);
    }

    writeStartupInfo();
    writeInfo();
}

void OutputToFile::writeStartupInfo()
{
    if (_enabled)
    {
        _iniCurrent->setValue("software/version",                   prepare(QCoreApplication::applicationVersion()));

        _iniCurrent->setValue("software/startup_time",              prepare(_startupDateTime.toString(DateTimeFormat)));
        _iniCurrent->setValue("software/startup_timestamp_utc",     prepare(QString("%1").arg(_startupDateTime.toMSecsSinceEpoch())));

        _iniCurrent->setValue("software/startup_timezone_id",       prepare(QString::fromUtf8(_startupDateTime.timeZone().id())));
        _iniCurrent->setValue("software/startup_timezone_offset_from_utc", prepare(QString("%1")
                              .arg(double(_startupDateTime.timeZone().standardTimeOffset(_startupDateTime)) / float(60 * 60))));
    }
}

void OutputToFile::writeInfo()
{
    if (_enabled)
    {
        _iniCurrent->setValue("youtube/broadcast_connected", _youTubeInfo.broadcastConnected);
        _iniCurrent->setValue("youtube/broadcast_id", prepare(_youTubeInfo.broadcastId));
        _iniCurrent->setValue("youtube/broadcast_user_specified", prepare(_youTubeInfo.userSpecified));
        _iniCurrent->setValue("youtube/broadcast_url", prepare(_youTubeInfo.broadcastLongUrl.toString()));
        _iniCurrent->setValue("youtube/broadcast_chat_url", prepare(_youTubeInfo.broadcastChatUrl.toString()));
        _iniCurrent->setValue("youtube/broadcast_control_panel_url", prepare(_youTubeInfo.controlPanelUrl.toString()));

        _iniCurrent->setValue("twitch/broadcast_connected", _twitchInfo.connected);
        _iniCurrent->setValue("twitch/channel_name", prepare(_twitchInfo.channelName));
        _iniCurrent->setValue("twitch/user_specified", prepare(_twitchInfo.userSpecifiedChannel));
        _iniCurrent->setValue("twitch/channel_url", prepare(_twitchInfo.channelUrl.toString()));
        _iniCurrent->setValue("twitch/chat_url", prepare(_twitchInfo.chatUrl.toString()));
        _iniCurrent->setValue("twitch/control_panel_url", prepare(_twitchInfo.controlPanelUrl.toString()));
    }
}

QString OutputToFile::prepare(const QString &text)
{
    if (_codec.trimmed().toUpper() == "UTF-8")
    {
        return text;
    }

    return convertUtf8ToANSIByNumbers(text);
}

void OutputToFile::setYouTubeInfo(const YouTubeInfo &youTubeCurrent)
{
    _youTubeInfo = youTubeCurrent;

    reinitIni();
}

void OutputToFile::setTwitchInfo(const TwitchInfo &twitchCurrent)
{
    _twitchInfo = twitchCurrent;

    reinitIni();
}
