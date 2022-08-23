#include "authorqmlprovider.h"
#include "models/chatauthor.h"
#include "chat_services/youtube.hpp"
#include "chathandler.hpp"
#include <QDesktopServices>

namespace
{

static const int YouTubeAvatarSize = 1080;

}

AuthorQMLProvider::AuthorQMLProvider(const ChatHandler& chantHandler, const ChatMessagesModel& messagesModel_, const OutputToFile& outputToFile_, QObject *parent)
    : QObject{parent}
    , messagesModel(messagesModel_)
    , outputToFile(outputToFile_)
{
    connect(&chantHandler, &ChatHandler::messagesDataChanged, this, [this]()
    {
        emit changed();
    });
}

void AuthorQMLProvider::setSelectedAuthorId(const QString &authorId_)
{
    authorId = authorId_;
    emit changed();
}

QString AuthorQMLProvider::getName() const
{
    const ChatAuthor* author = messagesModel.getAuthor(authorId);
    if (!author)
    {
        return QString();
    }

    return author->getName();
}

int AuthorQMLProvider::getServiceType() const
{
    const ChatAuthor* author = messagesModel.getAuthor(authorId);
    if (!author)
    {
        return (int)ChatService::ServiceType::Unknown;
    }

    return (int)author->getServiceType();
}

QUrl AuthorQMLProvider::getAvatarUrl() const
{
    const ChatAuthor* author = messagesModel.getAuthor(authorId);
    if (!author)
    {
        return QUrl();
    }

    return author->getAvatarUrl();
}

int AuthorQMLProvider::getMessagesCount() const
{
    const ChatAuthor* author = messagesModel.getAuthor(authorId);
    if (!author)
    {
        return -1;
    }

    return author->getMessagesCount();
}

bool AuthorQMLProvider::openAvatar() const
{
    const ChatAuthor* author = messagesModel.getAuthor(authorId);
    if (!author)
    {
        return false;
    }

    QUrl url = author->getAvatarUrl();

    if (author->getServiceType() == ChatService::ServiceType::YouTube)
    {
        const QUrl url_ = YouTube::createResizedAvatarUrl(author->getAvatarUrl(), YouTubeAvatarSize);
        if (url_.isValid())
        {
            url = url_;
        }
    }

    return QDesktopServices::openUrl(url);
}

bool AuthorQMLProvider::openPage() const
{
    const ChatAuthor* author = messagesModel.getAuthor(authorId);
    if (!author)
    {
        return false;
    }

    return QDesktopServices::openUrl(author->getPageUrl());
}

bool AuthorQMLProvider::openFolder() const
{
    const ChatAuthor* author = messagesModel.getAuthor(authorId);
    if (!author)
    {
        return false;
    }

    return QDesktopServices::openUrl(QUrl::fromLocalFile(outputToFile.getAuthorDirectory(author->getServiceType(), author->getId())));
}
