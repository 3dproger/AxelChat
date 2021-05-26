#pragma once

#include <QString>
#include <QDateTime>
#include <QAbstractListModel>
#include <QUrl>
#include "chatmessage.hpp"

struct YouTubeInfo {
    QString broadcastName;
    QString broadcastDescription;
    QUrl broadcastLongUrl;
    QUrl broadcastShortUrl;
    QUrl broadcastChatUrl;
    QUrl controlPanelUrl;
    QString broadcastId;
    QString userSpecified;
    bool broadcastConnected = false;
};

struct TwitchInfo {
    QString channelName;
    QString userSpecifiedChannel;
    QString oauthToken;
    bool connected = false;

    bool operator==(const TwitchInfo& other) const
    {
        return  channelName == other.channelName &&
                userSpecifiedChannel == other.userSpecifiedChannel &&
                oauthToken == other.oauthToken &&
                connected == other.connected;
    }

    bool operator!=(const TwitchInfo& other) const
    {
        return !(*this == other);
    }
};

static QString simplifyUrl(const QString& url)
{
    QString withoutHttpsWWW = url;

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

    return withoutQuery;
}
