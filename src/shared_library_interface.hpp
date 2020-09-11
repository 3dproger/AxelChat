#ifndef AXELCHAT_GLOBAL_HPP
#define AXELCHAT_GLOBAL_HPP

#include <QtCore/qglobal.h>
#include <QApplication>
#include "constants.hpp"
#include <QtWebEngine/QtWebEngine>
#include <QDir>
#include "githubapi.hpp"
#include <QSysInfo>
#include "chathandler.hpp"
#include <QThread>
#include <QRunnable>
#include <QMainWindow>
#include <QDesktopServices>

#if defined(AXELCHAT_LIBRARY)
#  define DLLEXPORT extern "C" __declspec(dllexport)
#endif

namespace AxelChatLib
{
const QString APPLICATION_NAME    = "AxelChatLib";
const QString APPLICATION_VERSION = AxelChat::APPLICATION_VERSION;
const QString ORGANIZATION_NAME   = AxelChat::ORGANIZATION_NAME;
const QString ORGANIZATION_DOMAIN = AxelChat::ORGANIZATION_DOMAIN;
bool enableUtf8 = false;
int ARGC = 0;
const char* ARGV = "";

uint64_t lastReadedIdNum = 0;

ChatHandler* chatHandler = nullptr;

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
    AxelChatLib::enableUtf8 = enable_utf8;
    QApplication::setApplicationName(AxelChatLib::APPLICATION_NAME);
    QApplication::setApplicationVersion(AxelChatLib::APPLICATION_VERSION);
    QApplication::setOrganizationName(AxelChatLib::ORGANIZATION_NAME);
    QApplication::setOrganizationDomain(AxelChatLib::ORGANIZATION_DOMAIN);

    QApplication::addLibraryPath("./");
    new QApplication(AxelChatLib::ARGC, (char**)&ARGV);
    QtWebEngine::initialize();
    chatHandler = new ChatHandler(nullptr, "chat_handler");

    return 0.0;
}

DLLEXPORT double axelchat_stop()
{
    if (chatHandler)
    {
        delete chatHandler;
        chatHandler = nullptr;
    }
    QApplication::quit();

    return 0.0;
}

DLLEXPORT double axelchat_get_version_major()
{
    return GitHubApi::Version::fromString(QCoreApplication::applicationVersion()).major;
}

DLLEXPORT double axelchat_get_version_minor()
{
    return GitHubApi::Version::fromString(QCoreApplication::applicationVersion()).minor;
}

DLLEXPORT double axelchat_get_version_patch()
{
    return GitHubApi::Version::fromString(QCoreApplication::applicationVersion()).patch;
}

DLLEXPORT const char* axelchat_get_version_string()
{
    return toOutStr(QCoreApplication::applicationVersion());
}

DLLEXPORT const char* axelchat_get_target_cpu_architecture()
{
    return toOutStr(QSysInfo::buildCpuArchitecture());
}

DLLEXPORT double axelchat_youtube_set_url(const char* url)
{
    if (chatHandler && chatHandler->youTubeInterceptor())
    {
        chatHandler->youTubeInterceptor()->setLink(url);
    }

    return 0.0f;
}

DLLEXPORT const char* axelchat_youtube_get_broadcast_id()
{
    if (chatHandler && chatHandler->youTubeInterceptor())
        return toOutStr(chatHandler->youTubeInterceptor()->broadcastId());
    else
        return toOutStr("");
}

DLLEXPORT const char* axelchat_youtube_get_broadcast_long_url()
{
    if (chatHandler && chatHandler->youTubeInterceptor())
        return toOutStr(chatHandler->youTubeInterceptor()->broadcastLongUrl().toString());
    else
        return toOutStr("");
}

DLLEXPORT const char* axelchat_youtube_get_broadcast_short_url()
{
    if (chatHandler && chatHandler->youTubeInterceptor())
        return toOutStr(chatHandler->youTubeInterceptor()->broadcastShortUrl().toString());
    else
        return toOutStr("");
}

DLLEXPORT const char* axelchat_youtube_get_broadcast_control_panel_url()
{
    if (chatHandler && chatHandler->youTubeInterceptor())
        return toOutStr(chatHandler->youTubeInterceptor()->controlPanelUrl().toString());
    else
        return toOutStr("");
}

DLLEXPORT const char* axelchat_youtube_get_broadcast_chat_url()
{
    if (chatHandler && chatHandler->youTubeInterceptor())
        return toOutStr(chatHandler->youTubeInterceptor()->chatUrl().toString());
    else
        return toOutStr("");
}

DLLEXPORT double axelchat_open_url_in_browser(const char* url)
{
    return QDesktopServices::openUrl(QString(url)) ? 1.0 : 0.0;
}

DLLEXPORT double axelchat_youtube_get_connection_status()
{
    if (chatHandler && chatHandler->youTubeInterceptor())
    {
        if (chatHandler->youTubeInterceptor()->isConnected())
        {
            return 10.0;
        }
        else if (!chatHandler->youTubeInterceptor()->broadcastId().isEmpty())
        {
            return 1.0;
        }
        else
        {
            return 0.0;
        }
    }
    else
    {
        return 0.0;
    }
}

DLLEXPORT double axelchat_get_messages_ready_read()
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

DLLEXPORT double axelchat_get_message_next()
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

DLLEXPORT const char* axelchat_get_message_text(double id)
{
    return toOutStr(chatHandler->messagesModel()->dataByNumId(id, ChatMessagesModel::ChatMessageRoles::MessageText).toString());
}

DLLEXPORT const char* axelchat_get_message_send_time_str(double id)
{
    return toOutStr(chatHandler->messagesModel()->dataByNumId(id, ChatMessagesModel::ChatMessageRoles::MessagePublishedAt)
                    .toDateTime().time().toString("HH:mm:ss.zzz"));
}

DLLEXPORT const char* axelchat_get_message_received_time_str(double id)
{
    return toOutStr(chatHandler->messagesModel()->dataByNumId(id, ChatMessagesModel::ChatMessageRoles::MessageReceivedAt)
                    .toDateTime().time().toString("HH:mm:ss.zzz"));
}

DLLEXPORT const char* axelchat_get_message_author_name(double id)
{
    return toOutStr(chatHandler->messagesModel()->dataByNumId(id, ChatMessagesModel::ChatMessageRoles::AuthorName).toString());
}

DLLEXPORT const char* axelchat_get_message_author_channel_id(double id)
{
    return toOutStr(chatHandler->messagesModel()->dataByNumId(id, ChatMessagesModel::ChatMessageRoles::AuthorChannelId).toString());
}

DLLEXPORT const char* axelchat_get_message_author_channel_url(double id)
{
    return toOutStr(chatHandler->messagesModel()->dataByNumId(id, ChatMessagesModel::ChatMessageRoles::AuthorPageUrl).toString());
}

DLLEXPORT const char* axelchat_get_message_author_avatar_url(double id)
{
    return toOutStr(chatHandler->messagesModel()->dataByNumId(id, ChatMessagesModel::ChatMessageRoles::AuthorAvatarUrl).toString());
}

DLLEXPORT const char* axelchat_get_message_author_custom_badge_url(double id)
{
    return toOutStr(chatHandler->messagesModel()->dataByNumId(id, ChatMessagesModel::ChatMessageRoles::AuthorCustomBadgeUrl).toString());
}

DLLEXPORT double axelchat_get_message_author_is_owner(double id)
{
    return chatHandler->messagesModel()->dataByNumId(id, ChatMessagesModel::ChatMessageRoles::AuthorIsChatOwner).toBool();
}

DLLEXPORT double axelchat_get_message_author_is_moderator(double id)
{
    return chatHandler->messagesModel()->dataByNumId(id, ChatMessagesModel::ChatMessageRoles::AuthorChatModerator).toBool();
}

DLLEXPORT double axelchat_get_message_author_is_sponsor(double id)
{
    return chatHandler->messagesModel()->dataByNumId(id, ChatMessagesModel::ChatMessageRoles::AuthorChatSponsor).toBool();
}

DLLEXPORT double axelchat_get_message_author_is_verified(double id)
{
    return chatHandler->messagesModel()->dataByNumId(id, ChatMessagesModel::ChatMessageRoles::AuthorIsVerified).toBool();
}

#endif // AXELCHAT_GLOBAL_HPP


/*
axelchat_available_new_version();

axelchat_get_target_os();//windows, linux, macos, android, ios...

axelchat_get_qt_version();

*/
