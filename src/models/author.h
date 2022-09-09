#pragma once

#include "chat_services/chatservicestypes.h"
#include <QString>
#include <QUrl>
#include <QColor>
#include <set>

class Author
{
    Q_GADGET
public:
    enum class Role {
        ServiceType = Qt::UserRole + 1 + 1024,
        Id,
        PageUrl,
        Name,
        NicknameColor,
        AvatarUrl,
        LeftBadgesUrls,
        RightBadgesUrls,

        IsVerified,
        IsChatOwner,
        Sponsor,
        Moderator
    };
    Q_ENUM(Role)

    enum class Flag {
        ChatOwner,
        Moderator,
        Sponsor,
        Verified,
    };
    Q_ENUM(Flag)

    Author() { };
    Author(AxelChat::ServiceType serviceType,
               const QString& name,
               const QString& authorId,
               const QUrl& avatarUrl = QUrl(),
               const QUrl& pageUrl = QUrl(),
               const QStringList& leftBadgesUrls = {},
               const QStringList& rightBadgesUrls = {},
               const std::set<Flag>& flags = {},
               const QColor& customNicknameColor = QColor());

    static const Author& getSoftwareAuthor();

    inline const QString& getId() const
    {
        return authorId;
    }
    inline const QString& getName() const
    {
        return name;
    }
    inline const QUrl& getAvatarUrl() const
    {
        return avatarUrl;
    }

    inline const QUrl& getPageUrl() const
    {
        return pageUrl;
    }
    inline const QColor& getCustomNicknameColor() const
    {
        return customNicknameColor;
    }
    inline AxelChat::ServiceType getServiceType() const
    {
        return serviceType;
    }
    inline const QStringList& getLeftBadgesUrls() const
    {
        return leftBadgesUrls;
    }
    inline const QStringList& getRightBadgesUrls() const
    {
        return rightBadgesUrls;
    }
    inline bool isHasFlag(const Flag flag) const
    {
        return flags.find(flag) != flags.end();
    }

    static QString flagToString(const Flag flag);

    inline const std::set<uint64_t>& getMessagesIds() const
    {
        return messagesIds;
    }

    inline std::set<uint64_t>& getMessagesIds()
    {
        return messagesIds;
    }

    bool setValue(const Role role, const QVariant& value);

    QVariant getValue(const Role role) const;

    inline static const std::set<Role> UpdateableRoles =
    {
        Role::PageUrl,
        Role::Name,
        Role::NicknameColor,
        Role::AvatarUrl,
        Role::LeftBadgesUrls,
        Role::RightBadgesUrls,

        Role::IsVerified,
        Role::IsChatOwner,
        Role::Sponsor,
        Role::Moderator
    };

private:
    AxelChat::ServiceType serviceType = AxelChat::ServiceType::Unknown;
    QString name;
    QString authorId;
    QUrl avatarUrl;
    QUrl pageUrl;
    QStringList leftBadgesUrls;
    QStringList rightBadgesUrls;
    std::set<Flag> flags;
    QColor customNicknameColor;
    std::set<uint64_t> messagesIds;
};

Q_DECLARE_METATYPE(Author::Role)
