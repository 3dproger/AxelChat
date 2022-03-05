#ifndef AXELCHAT_GLOBAL_HPP
#define AXELCHAT_GLOBAL_HPP

#include "applicationinfo.hpp"
#include "chathandler.hpp"
#include <QtCore/qglobal.h>
#include <QGuiApplication>
#include <QDir>
#include <QQueue>
#include <QSysInfo>
#include <QThread>
#include <QRunnable>
#include <QDesktopServices>
#include <QLibrary>

#if defined(AXELCHAT_LIBRARY)
#  define DLLEXPORT extern "C" __declspec(dllexport)
#endif

namespace AxelChatLib
{
const QString APPLICATION_NAME    = "AxelChatLib";

bool enableUtf8 = false;
int ARGC = 0;
const char* ARGV = "";

uint64_t lastReadedIdNum = 0;

ChatHandler* chatHandler = nullptr;

QQueue<QString> logMessages;
static const int LogMessagesMaxCount = 100;

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context)

    QString s;
    switch (type)
    {
    case QtWarningMsg:
        s += "W: ";
        break;
    case QtCriticalMsg:
        s += "C: ";
        break;
    case QtFatalMsg:
        s += "F: ";
        break;
    case QtInfoMsg:
        s += "I: ";
        break;
    case QtDebugMsg:
    default:
        s += "D: ";
    }

    s += msg;

    logMessages.enqueue(s);
    if (!logMessages.isEmpty() && logMessages.count() > LogMessagesMaxCount)
    {
        logMessages.dequeue();
    }
}

QByteArray convertToGMTagNamed(uint ucs4)
{
    if (ucs4 == 0x1F680)
    {
        return "<emoji:rocket1>";
    }
    else if (ucs4 == 0x263A)
    {
        return "<emoji:smile1>";
    }
    else if (ucs4 == 0x2639)
    {
        return "<emoji:frowning1>";
    }
    else
    {
        return "<" + QByteArray::number(ucs4) + ">";
    }
}

QByteArray convertToGMTag(uint ucs4)
{
    return "<" + QByteArray::number(ucs4) + ">";
}

const char* convertToGMTags(const QString& string)
{
    const QVector<uint>& ucs4str = string.toUcs4();
    QByteArray ba;
    ba.reserve(ucs4str.count() * 4);

    for (const uint& c : ucs4str)
    {
        if (c >= 32u && c <= 126u && c!= 35u && c!= 60u && c!= 62u)
        {
            ba += (char)c;
        }
        else
        {
            ba += convertToGMTag(c);
        }
    }

    return ba;
}

const char* toOutStr(const QString& string)
{
    if (AxelChatLib::enableUtf8)
    {
        return string.toUtf8();
    }
    else
    {
        return convertToGMTags(string);
    }
}

}

using namespace AxelChatLib;

DLLEXPORT double axelchat_start(double enable_utf8)
{
    qInstallMessageHandler(messageHandler);

    AxelChatLib::enableUtf8 = enable_utf8;

    QCoreApplication::setApplicationName(AxelChatLib::APPLICATION_NAME);
    QCoreApplication::setOrganizationName  (APP_INFO_COMPANYNAME_STR);
    QCoreApplication::setOrganizationDomain(APP_INFO_COMPANYDOMAIN_STR);
    QCoreApplication::setApplicationVersion(APP_INFO_PRODUCTVERSION_STR);
    QCoreApplication::addLibraryPath("./");
    new QGuiApplication(AxelChatLib::ARGC, (char**)&ARGV);

    chatHandler = new ChatHandler(nullptr);

    return 0.0;
}

DLLEXPORT double axelchat_stop()
{
    if (chatHandler)
    {
        delete chatHandler;
        chatHandler = nullptr;
    }
    QCoreApplication::quit();

    return 0.0;
}

DLLEXPORT const char* axelchat_get_version()
{
    return toOutStr(QCoreApplication::applicationVersion());
}

DLLEXPORT const char* axelchat_get_target_cpu_architecture()
{
    return toOutStr(QSysInfo::buildCpuArchitecture());
}

DLLEXPORT double axelchat_log_get_ready_read_count()
{
    return logMessages.count();
}

DLLEXPORT const char* axelchat_log_get_next_text()
{
    if (!logMessages.isEmpty())
    {
        return toOutStr(logMessages.takeFirst()/*dequeue()*/);
    }

    return toOutStr("");
}

DLLEXPORT double axelchat_youtube_set_url(const char* url)
{
    if (chatHandler && chatHandler->youTube())
    {
        chatHandler->youTube()->setLink(url);
    }

    return 0.0f;
}

DLLEXPORT const char* axelchat_youtube_get_broadcast_id()
{
    if (chatHandler && chatHandler->youTube())
        return toOutStr(chatHandler->youTube()->broadcastId());
    else
        return toOutStr("");
}

DLLEXPORT const char* axelchat_youtube_get_broadcast_long_url()
{
    if (chatHandler && chatHandler->youTube())
        return toOutStr(chatHandler->youTube()->broadcastLongUrl().toString());
    else
        return toOutStr("");
}

DLLEXPORT const char* axelchat_youtube_get_broadcast_short_url()
{
    if (chatHandler && chatHandler->youTube())
        return toOutStr(chatHandler->youTube()->broadcastUrl().toString());
    else
        return toOutStr("");
}

DLLEXPORT const char* axelchat_youtube_get_broadcast_control_panel_url()
{
    if (chatHandler && chatHandler->youTube())
        return toOutStr(chatHandler->youTube()->controlPanelUrl().toString());
    else
        return toOutStr("");
}

DLLEXPORT const char* axelchat_youtube_get_broadcast_chat_url()
{
    if (chatHandler && chatHandler->youTube())
        return toOutStr(chatHandler->youTube()->chatUrl().toString());
    else
        return toOutStr("");
}

DLLEXPORT double axelchat_open_url_in_browser(const char* url)
{
    return QDesktopServices::openUrl(QString(url)) ? 1.0 : 0.0;
}

DLLEXPORT double axelchat_youtube_get_connection_status()
{
    if (chatHandler && chatHandler->youTube())
    {
        switch (chatHandler->youTube()->connectionStateType()) {
        case AbstractChatService::NotConnected:
            return 1.0;
        case AbstractChatService::Connecting:
            return 2.0;
        case AbstractChatService::Connected:
            return 10.0;
        }
    }

    return 0.0;
}

DLLEXPORT double axelchat_message_get_ready_read_count()
{
    if (chatHandler)
    {
        return chatHandler->messagesModel()->lastIdNum() - lastReadedIdNum;
    }
    else
    {
        return 0;
    }
}

DLLEXPORT double axelchat_message_get_next()
{
    if (chatHandler)
    {
        if (chatHandler->messagesModel()->lastIdNum() - lastReadedIdNum > 0)
        {
            const int current = lastReadedIdNum;
            lastReadedIdNum++;
            return current;
        }
        else
        {
            return -1;
        }
    }
    else
    {
        return -1;
    }
}

DLLEXPORT const char* axelchat_message_get_text(double id)
{
    return toOutStr(chatHandler->messagesModel()->dataByNumId(id, ChatMessagesModel::ChatMessageRoles::MessageText).toString());
}

DLLEXPORT const char* axelchat_message_get_time(double id)
{
    return toOutStr(chatHandler->messagesModel()->dataByNumId(id, ChatMessagesModel::ChatMessageRoles::MessagePublishedAt)
                    .toDateTime().time().toString("HH:mm:ss.zzz"));
}

DLLEXPORT const char* axelchat_message_get_author_name(double id)
{
    return toOutStr(chatHandler->messagesModel()->dataByNumId(id, ChatMessagesModel::ChatMessageRoles::AuthorName).toString());
}

DLLEXPORT const char* axelchat_message_get_author_channel_id(double id)
{
    return toOutStr(chatHandler->messagesModel()->dataByNumId(id, ChatMessagesModel::ChatMessageRoles::AuthorChannelId).toString());
}

DLLEXPORT const char* axelchat_message_get_author_channel_url(double id)
{
    return toOutStr(chatHandler->messagesModel()->dataByNumId(id, ChatMessagesModel::ChatMessageRoles::AuthorPageUrl).toString());
}

DLLEXPORT const char* axelchat_message_get_author_avatar_url(double id)
{
    return toOutStr(chatHandler->messagesModel()->dataByNumId(id, ChatMessagesModel::ChatMessageRoles::AuthorAvatarUrl).toString());
}

DLLEXPORT const char* axelchat_message_get_author_custom_badge_url(double id)
{
    return toOutStr(chatHandler->messagesModel()->dataByNumId(id, ChatMessagesModel::ChatMessageRoles::AuthorCustomBadgeUrl).toString());
}

DLLEXPORT const char* axelchat_message_get_chat_service(double id)
{
    return toOutStr(chatHandler->messagesModel()->dataByNumId(id, ChatMessagesModel::ChatMessageRoles::MessageType).toString());
}

DLLEXPORT const char* axelchat_message_get_flags(double id)
{
    return "ToDo"; // ToDo
}

#endif // AXELCHAT_GLOBAL_HPP
