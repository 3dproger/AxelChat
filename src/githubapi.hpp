#ifndef GITGUBAPI_HPP
#define GITGUBAPI_HPP

#include <QObject>
#include <QSettings>
#include <QNetworkAccessManager>
#include <QQmlEngine>

class GitHubApi : public QObject
{
    Q_PROPERTY(QString    lastVersionName        READ lastVersionName        NOTIFY replied)
    Q_PROPERTY(QString    lastVersionDescription READ lastVersionDescription NOTIFY replied)
    Q_PROPERTY(QUrl       lastVersionDownloadUrl READ lastVersionDownloadUrl NOTIFY replied)
    Q_PROPERTY(QUrl       lastVersionHtmlUrl     READ lastVersionHtmlUrl     NOTIFY replied)
    Q_PROPERTY(QDateTime  lastVersionDate        READ lastVersionDateTime    NOTIFY replied)
    Q_PROPERTY(ReplyState replyState             READ replyState             NOTIFY replied)
    Q_PROPERTY(bool       autoRequested          READ autoRequested          NOTIFY replied)

    Q_OBJECT
public:
    enum VersionType{
        UnknownVersionType = 0,
        ReleaseVersionType = 1000,
        BetaVersionType    = 100,
        AlphaVersionType   = 10
    };

    enum class OperatingSystem{
        Unknown,
        Windows,
        Linux,
        MacOS,
        Android,
        IOS
    };

    enum Architecture{
        Unknown,
        x86_32,
        x86_64
        //ToDo: ARM and other
    };

    struct Version{
        bool valid = false;
        QString specifiedName;
        int major = -1;
        int minor = -1;
        int patch = -1;
        bool draft = false;
        bool preRelease = false;
        VersionType type = VersionType::UnknownVersionType;

        static Version fromString(const QString& vesrionName)
        {
            Version version;

            version.specifiedName = vesrionName;

            QString prepared = vesrionName.trimmed();

            QRegExp rx("^(\\d+)\\.(\\d+)\\.(\\d+)((-beta)|(-alpha))?$");
            rx.setCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);

            if (rx.indexIn(vesrionName) != -1)
            {
                version.valid = true;

                bool ok = true;

                version.major = rx.cap(1).toInt(&ok);
                if (!ok)
                {
                    version.valid = false;
                }

                version.minor = rx.cap(2).toInt(&ok);
                if (!ok)
                {
                    version.valid = false;
                }

                version.patch = rx.cap(3).toInt(&ok);
                if (!ok)
                {
                    version.valid = false;
                }

                if (rx.cap(4).toLower() == "-beta")
                {
                    version.type = VersionType::BetaVersionType;
                }
                else if (rx.cap(4).toLower() == "-alpha")
                {
                    version.type = VersionType::AlphaVersionType;
                }
                else
                {
                    version.type = VersionType::ReleaseVersionType;
                }

                //qDebug() << rx.capturedTexts();
            }

            if (version.type == VersionType::UnknownVersionType)
            {
                version.valid = false;
            }

            return version;
        }

        QString toString() const
        {
            QString s;

            if (valid)
            {
                s = QString("%1.%2.%3")
                                    .arg(major)
                                    .arg(minor)
                                    .arg(patch);

                switch (type) {
                case UnknownVersionType:
                    break;
                case ReleaseVersionType:
                    break;
                case BetaVersionType:
                    s += "-beta";
                    break;
                case AlphaVersionType:
                    s += "-alpha";
                    break;
                }
            }
            else
            {
                s = "<invalid_version>";
            }

            return s;
        }

        bool operator< (const Version& right) const
        {
            //Valid
            if (this->valid && !right.valid)
            {
                return false;
            }
            else if (!this->valid && right.valid)
            {
                return true;
            }

            //Draft
            if (this->draft && !right.draft)
            {
                return true;
            }
            else if (!this->draft && right.draft)
            {
                return false;
            }

            //PreRelease
            if (this->preRelease && !right.preRelease)
            {
                return true;
            }
            else if (!this->preRelease && right.preRelease)
            {
                return false;
            }

            //Numbers
            if (this->major < right.major)
            {
                return true;
            }
            else if (this->major > right.major)
            {
                return false;
            }

            else if (this->minor < right.minor)
            {
                return true;
            }
            else if (this->minor > right.minor)
            {
                return false;
            }

            else if (this->patch < right.patch)
            {
                return true;
            }
            else if (this->patch > right.patch)
            {
                return false;
            }
            //Type
            else if (this->type < right.type)
            {
                return true;
            }
            else if (this->type > right.type)
            {
                return false;
            }

            return false;
        }
    };

    struct Asset{
        bool valid = false;
        QString name;
        QString label;
        QUrl downloadUrl;
        QDateTime updatedAt;
        QDateTime createdAt;
        uint64_t size;

        OperatingSystem operatingSystem = OperatingSystem::Unknown;
        Architecture architecture = Architecture::Unknown;

        QString toString() const
        {
            return name;
        }
    };

    struct Release{
        Version version;

        QUrl htmlUrl;
        QString tagName;
        QString name;
        QDateTime publishedAt;
        QDateTime assetReleasedAt;
        QString description;

        Asset suitableAsset;

        QList<Asset> assets;

        bool operator< (const Release& right) const
        {
            return this->version < right.version;
        }

        QString toString() const
        {
            QString s;

            s = version.toString();

            if (version.draft)
            {
                s += ", draft";
            }

            if (version.preRelease)
            {
                s += ", preRelease";
            }

            if (suitableAsset.valid)
            {
                s += QString(", valid asset: %1").arg(suitableAsset.toString());
            }
            else
            {
                s += ", has not valid asset";
            }

            if (!assets.isEmpty())
            {
                s += ", all assets: [";
            }
            else
            {
                s += ", no assets";
            }

            for (int i = 0; i < assets.count(); ++i)
            {
                const Asset& asset = assets.at(i);

                s += asset.toString();

                if (i != assets.count() - 1)
                {
                    s += ", ";
                }
            }

            if (!assets.isEmpty())
            {
                s += "]";
            }

            return s;
        }
    };

    enum ReplyState{
        NoReply,
        NewVersionAvailable,
        NewVersionAvailableButSkipped,
        LatestVersionYouHave,
        RequestLimitsExceeded,
        FailedToConnect
    };
    Q_ENUM(ReplyState)

    explicit GitHubApi(QSettings& settings, const QString& settingsGroup, QNetworkAccessManager& network, QObject *parent = nullptr);

    static void declareQml()
    {
        qmlRegisterUncreatableType<GitHubApi>("AxelChat.UpdateChecker", 1, 0, "UpdateChecker", "Type cannot be created in QML");
    }

    Release latestRelease() const;

    QString lastVersionName() const;
    QString lastVersionDescription() const;
    QUrl lastVersionDownloadUrl() const;
    QUrl lastVersionHtmlUrl() const;
    ReplyState replyState() const;
    QDateTime lastVersionDateTime() const;
    Q_INVOKABLE void setSkipCurrentVersion(bool skip) const;

    bool autoRequested() const;

signals:
    void replied();

public slots:
    void checkForNewVersion();

private slots:
    void onReplyReleases();

private:
    Asset writeArchOS(const QString& fileName, const Asset& asset);

    OperatingSystem currentOperatingSystem() const;
    Architecture currentArchitecture() const;

    QSettings& settings;
    const QString SettingsGroup;
    QNetworkAccessManager& network;

    const QString _settingsKeyUserSpecifiedLink = "skipped_versions";

    const QUrl _releasesUrl = QUrl("https://api.github.com/repos/3dproger/AxelChat/releases");

    ReplyState _replyState = ReplyState::NoReply;
    Release _latestRelease;

    bool _autoRequested = false;

    //Available version rules
    const bool _enabledNotValidVersion      = false;
    const bool _enabledWithoutSuitableAsset = false;
    const bool _enabledWithoutAssets        = false;
    const bool _enabledDraft                = false;
    const bool _enabledPreRelease           = false;
    const bool _enabledBeta                 = false;
    const bool _enabledAlpha                = false;
    const bool _enabledRelease              = true;
};

#endif // GITGUBAPI_HPP
