#include "database.h"
#include <QFile>
#include <QDebug>
#include <QTimeZone>

namespace
{

static const QString DBHostName = "AxelChatBroadcastsDataBase";

static const QString DBYouTubeBroadcastsTable = "YouTubeBroadcasts";
static const QString DBYouTubeMessagesTable = "YouTubeMessages";
static const QString DBYouTubeAuthorsTable = "YouTubeAuthors";

QString prepareDateTime(const QDateTime& dateTime)
{
    QString result = dateTime.toString("yyyy.MM.dd hh:mm:ss.zzz ");
    const int offsetFromUtc = dateTime.timeZone().offsetFromUtc(dateTime);
    if (offsetFromUtc >= 0)
    {
        result += QString("UTC+%1ms").arg(offsetFromUtc);
    }
    else
    {
        result += QString("UTC%1ms").arg(offsetFromUtc);
    }

    return result;
}

QString prepareBool(const bool& value)
{
    return value ? "1" : "0";
}

}

DataBase::DataBase(QObject *parent) : QObject(parent)
{
    if (!QFile(_databasePath).exists())
    {
        restoreDataBase();
    }
    else
    {
        openDataBase();
    }
}

DataBase::~DataBase()
{
    _db.close();
}

bool DataBase::openDataBase()
{
    _db = QSqlDatabase::addDatabase("QSQLITE");
    _db.setHostName(DBHostName);
    _db.setDatabaseName(_databasePath);

    if (_db.open())
    {
        return true;
    }
    else
    {
        qDebug() << Q_FUNC_INFO << ": failed to open database: " << _db.lastError().text();
        return false;
    }
}

void DataBase::restoreDataBase()
{
    openDataBase();

    {
        QSqlQuery query;
        if (!query.exec("CREATE TABLE '" + DBYouTubeBroadcastsTable + "' ("
                            "broadcast_id                   TEXT PRIMARY KEY, "
                            "link_short                     TEXT, "
                            "firts_connection_date_time     TEXT"
                        ")"
            ))
        {
            qDebug() << Q_FUNC_INFO << ": DataBase: error of create table " + DBYouTubeBroadcastsTable + ": " + query.lastError().text();
        }
    }

    {
        QSqlQuery query;
        if (!query.exec("CREATE TABLE '" + DBYouTubeMessagesTable + "' ("
                            "author_name                    TEXT, "
                            "text                           TEXT, "
                            "message_id                     TEXT PRIMARY KEY, "
                            "broadcast_id                   TEXT, "
                            "published_date_time            TEXT, "
                            "received_date_time             TEXT, "
                            "marked_as_deleted              INTEGER, "

                            "author_channel_id              TEXT, "
                            "author_avatar_url              TEXT, "
                            "author_custom_badge_url        TEXT, "
                            "author_is_verified             INTEGER, "
                            "author_is_chat_owner           INTEGER, "
                            "author_is_sponsor              INTEGER, "
                            "author_is_moderator            INTEGER"
                        ")"
            ))
        {
            qDebug() << Q_FUNC_INFO << ": DataBase: error of create table " + DBYouTubeMessagesTable + ": " + query.lastError().text();
        }
    }

    {
        QSqlQuery query;
        if (!query.exec("CREATE TABLE '" + DBYouTubeAuthorsTable + "' ("
                            "name                           TEXT, "
                            "channel_id                     TEXT PRIMARY KEY, "
                            "last_message_date_time         TEXT, "
                            "page_url                       TEXT, "

                            "avatar_url                     TEXT, "
                            "custom_badge_url               TEXT, "
                            "is_verified                    INTEGER, "
                            "is_chat_owner                  INTEGER, "
                            "is_sponsor                     INTEGER, "
                            "is_moderator                   INTEGER"
                        ")"
            ))
        {
            qDebug() << Q_FUNC_INFO << ": DataBase: error of create table " + DBYouTubeAuthorsTable + ": " + query.lastError().text();
        }
    }
}

void DataBase::addYouTubeBroadcast(const QString& youtubeBroadcasId, const QString& youtubeLinkShort, const QDateTime& connectionDateTime)
{
    QSqlQuery query;
    query.prepare("INSERT INTO '" + DBYouTubeBroadcastsTable + "' ("
                                "broadcast_id, "
                                "link_short, "
                                "firts_connection_date_time) "
                  "VALUES ('"   + youtubeBroadcasId + "', '"
                                + youtubeLinkShort + "', '"
                                + prepareDateTime(connectionDateTime) + //ToDo переделать
                        "')");

    if (!query.exec())
    {
        qDebug() << Q_FUNC_INFO << ": failed to add broadcast: " + query.lastError().text();
    }
}

void DataBase::addYouTubeMessage(const ChatMessage& message)
{
    if (message.type() != ChatMessage::Type::YouTube)
    {
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT or IGNORE INTO '" + DBYouTubeMessagesTable + "' ("
                                    "author_name, "
                                    "text, "
                                    "message_id, "
                                    "broadcast_id, "
                                    "published_date_time, "
                                    "received_date_time, "
                                    "marked_as_deleted, "
                                    "author_channel_id, "
                                    "author_avatar_url, "
                                    "author_custom_badge_url, "
                                    "author_is_verified, "
                                    "author_is_chat_owner, "
                                    "author_is_sponsor, "
                                    "author_is_moderator"
                            ") "
                      "VALUES ("
                                    ":_author_name, "
                                    ":_text, "
                                    ":_message_id, "
                                    ":_broadcast_id, "
                                    ":_published_date_time, "
                                    ":_received_date_time, "
                                    ":_marked_as_deleted, "
                                    ":_author_channel_id, "
                                    ":_author_avatar_url, "
                                    ":_author_custom_badge_url, "
                                    ":_author_is_verified, "
                                    ":_author_is_chat_owner, "
                                    ":_author_is_sponsor, "
                                    ":_author_is_moderator"
                            ")");

    query.addBindValue(message.author().name());
    query.addBindValue(message.text());
    query.addBindValue(message.id());
    query.addBindValue("!!!");//ToDo: переделать
    query.addBindValue(prepareDateTime(message.publishedAt()));
    query.addBindValue(prepareDateTime(message.receivedAt()));
    query.addBindValue(message.markedAsDeleted());//ToDo: переделать
    query.addBindValue(message.author().channelId());
    query.addBindValue(message.author().avatarUrl().toString());
    query.addBindValue(message.author().customBadgeUrl().toString());
    query.addBindValue(message.author().isVerified());
    query.addBindValue(message.author().isChatOwner());
    query.addBindValue(message.author().isChatSponsor());
    query.addBindValue(message.author().isChatModerator());

    if (!query.exec())
    {
        qDebug() << Q_FUNC_INFO << ": failed to add message '" + message.text() + "': " + query.lastError().text();
    }

    addYouTubeAuthor(message);
}

void DataBase::addYouTubeAuthor(const ChatMessage& message)
{
    const MessageAuthor& author = message.author();

    QSqlQuery query;
    //ToDo: query.execBatch()
    query.prepare("INSERT or REPLACE INTO '" + DBYouTubeAuthorsTable + "' ("
                                    "name, "
                                    "channel_id, "
                                    "last_message_date_time, "
                                    "page_url, "

                                    "avatar_url, "
                                    "custom_badge_url, "
                                    "is_verified, "
                                    "is_chat_owner, "
                                    "is_sponsor, "
                                    "is_moderator"
                            ") "
                      "VALUES ("
                                    ":_name, "
                                    ":_channel_id, "
                                    ":_last_message_date_time, "
                                    ":_page_url, "

                                    ":_avatar_url, "
                                    ":_custom_badge_url, "
                                    ":_is_verified, "
                                    ":_is_chat_owner, "
                                    ":_is_sponsor, "
                                    ":_is_moderator"
                            ")");

    query.addBindValue(author.name());
    query.addBindValue(author.channelId());
    query.addBindValue(prepareDateTime(message.publishedAt()));
    query.addBindValue(author.pageUrl());
    query.addBindValue(author.avatarUrl());
    query.addBindValue(author.customBadgeUrl());
    query.addBindValue(author.isVerified());
    query.addBindValue(author.isChatOwner());
    query.addBindValue(author.isChatSponsor());
    query.addBindValue(author.isChatModerator());

    if (!query.exec())
    {
        qDebug() << Q_FUNC_INFO << ": failed to add author: " + query.lastError().text();
    }
}
