#pragma once

#include "chatmessage.hpp"
#include <QString>
#include <QDateTime>
#include <QAbstractListModel>
#include <QUrl>
#include <QFile>
#include <QDir>

namespace AxelChat
{

static const QByteArray UserAgentNetworkHeaderName = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.77 Safari/537.36";

struct YouTubeInfo {
    QString channelId;
    QString broadcastName;
    QString broadcastDescription;
    QUrl broadcastLongUrl;
    QUrl broadcastShortUrl;
    QUrl broadcastChatUrl;
    QUrl controlPanelUrl;
    QString broadcastId;
    QString userSpecified;
    QString detailedInformation;
    bool broadcastConnected = false;
    int viewers = -1;
};

struct TwitchInfo {
    QString channelLogin;
    QString broadcasterId;
    QString userSpecifiedChannel;
    QUrl channelUrl;
    QUrl chatUrl;
    QUrl controlPanelUrl;
    QString oauthToken;
    QString detailedInformation;
    bool connected = false;
    int viewers = -1;

    /*bool operator==(const TwitchInfo& other) const
    {
        return  channelLogin == other.channelLogin &&
                userSpecifiedChannel == other.userSpecifiedChannel &&
                oauthToken == other.oauthToken &&
                connected == other.connected &&
                viewers == other.viewers;
    }

    bool operator!=(const TwitchInfo& other) const
    {
        return !(*this == other);
    }*/
};

struct GoodGameInfo {
    bool connected = false;
    QString channelId;
    double protocolVersion = 0;
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

static void saveDebugDataToFile(const QString& folder, const QString& fileName, const QByteArray& data)
{
#ifndef AXELCHAT_LIBRARY
#ifndef QT_NO_DEBUG
    QDir().mkpath(folder);

    QFile file(folder + "/" + fileName);
    if (file.open(QFile::OpenModeFlag::WriteOnly | QFile::OpenModeFlag::Text))
    {
        file.write(data);
        file.close();
        qDebug() << "Saved to" << file.fileName();
    }
    else
    {
        qDebug() << "Failed to save" << file.fileName();
    }
#endif
#endif
}

}
