#pragma once

#include <QString>
#include <QDateTime>
#include <QAbstractListModel>
#include <QUrl>
#include "chatmessage.hpp"

enum class BroadcastState{
    NotConnected,
    Planned,
    Live,
    Finished
};

struct YouTubeInfo{
    QString broadcastName;
    QString broadcastDescription;
    QUrl broadcastLongUrl;
    QUrl broadcastShortUrl;
    BroadcastState broadcastState = BroadcastState::NotConnected;
    QUrl broadcastChatUrl;
    QUrl controlPanelUrl;
    QString broadcastId;
    QString userSpecified;
    bool broadcastConnected = false;

    uint64_t viewers = 0;
    uint64_t views = 0;
    uint64_t likes = 0;
    uint64_t dislikes = 0;
};

struct BroadcastInfo{
    QString id;
    QString liveChatId;
    QString title;
};
