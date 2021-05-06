#ifndef DATABASE_H
#define DATABASE_H

#include "chatmessage.hpp"
#include <QObject>
#include <QSql>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>
#include <QDateTime>

class DataBase : public QObject
{
    Q_OBJECT
public:

    explicit DataBase(QObject *parent = nullptr);
    ~DataBase();

    void addYouTubeBroadcast(const QString& youtubeBroadcasId, const QString& youtubeLinkShort, const QDateTime& connectionDateTime);
    void addYouTubeMessage(const ChatMessage& message);

signals:

protected:

    void addYouTubeAuthor(const ChatMessage& message);
    void restoreDataBase();
    bool openDataBase();

private:

    const QString _databasePath = "D:/Misc/WindowsDesktop/example.sqlite";
    QSqlDatabase _db;
};

#endif // DATABASE_H
