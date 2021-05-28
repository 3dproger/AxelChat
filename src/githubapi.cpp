#include "githubapi.hpp"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonObject>
#include <QCoreApplication>

GitHubApi::GitHubApi(QSettings* settings, const QNetworkProxy& proxy, const QString& settingsGroup, QObject *parent) : QObject(parent)
{
    _settings = settings;
    _settingsGroup = settingsGroup;

    _manager.setProxy(proxy);

    if (_settings)
    {
        QDateTime latestPollingAt = _settings->value(_settingsGroup + "/latest_polling_at").toDateTime();

        if (latestPollingAt.isValid())
        {
            if (latestPollingAt.addDays(1) <= QDateTime::currentDateTime())//Uncomment for release, comment for testing auto requests
            //if (true)//Uncomment for testing auto requests, comment for release
            {
                checkForNewVersion();
                _autoRequested = true;
            }
        }
        else
        {
            checkForNewVersion();
            _autoRequested = true;
        }
    }
    else
    {
        checkForNewVersion();
        _autoRequested = true;
    }
}

GitHubApi::Release GitHubApi::latestRelease() const
{
    return _latestRelease;
}

void GitHubApi::checkForNewVersion()
{
    _replyState = ReplyState::NoReply;
    _latestRelease = Release();

    QNetworkRequest request = QNetworkRequest(_releasesUrl);

    QNetworkReply* reply = _manager.get(request);

    connect(reply, &QNetworkReply::finished, this, &GitHubApi::onReplyReleases);
}

void GitHubApi::onReplyReleases()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());

    if (reply)
    {
        const QByteArray data = reply->readAll();

        //qDebug(data);

        if (reply->error() == QNetworkReply::NoError)
        {
            _latestRelease = Release();

            const QJsonArray& releasesArray = QJsonDocument::fromJson(data).array();

            QList<Release> releases;

            for (const QJsonValue& releaseValue : releasesArray)
            {
                const QJsonObject& releaseObject = releaseValue.toObject();

                if (releaseObject.contains("tag_name"))
                {
                    Release release;

                    release.htmlUrl     = QUrl(releaseObject.value("html_url").toString());
                    release.tagName     = releaseObject.value("tag_name").toString();
                    release.name        = releaseObject.value("name").toString();
                    release.publishedAt = QDateTime::fromString(releaseObject.value("published_at").toString(), Qt::ISODate).toLocalTime();

                    release.description = releaseObject.value("body").toString();

                    release.version = Version::fromString(release.tagName);
                    if (!release.version.valid)
                    {
                        release.version = Version::fromString(release.name);

                        if (!release.version.valid)
                        {
                            qDebug(QString("Version string \"%1\" (tag) or \"%2\" (version name) not valid!")
                                   .arg(release.tagName)
                                   .arg(release.name)
                                   .toUtf8());
                        }
                    }

                    release.version.draft       = releaseObject.value("draft").toBool();
                    release.version.preRelease  = releaseObject.value("prerelease").toBool();

                   if (release.version.valid)
                   {
                       //qDebug(QString("Found version \"%1\"").arg(release.version.toString()).toUtf8());
                   }

                    const QJsonArray& assetsArray = releaseObject.value("assets").toArray();

                    for (const QJsonValue& assetValue : assetsArray)
                    {
                        const QJsonObject& assetObject = assetValue.toObject();

                        if (assetObject.contains("browser_download_url"))
                        {
                            Asset asset;

                            asset.valid       = true;
                            asset.name        = assetObject.value("name").toString();
                            asset.size        = assetObject.value("size").toInt();
                            asset.updatedAt   = QDateTime::fromString(assetObject.value("updated_at").toString(), Qt::ISODate).toLocalTime();
                            asset.createdAt   = QDateTime::fromString(assetObject.value("created_at").toString(), Qt::ISODate).toLocalTime();
                            asset.downloadUrl = QUrl(assetObject.value("browser_download_url").toString());
                            asset             = writeArchOS(asset.name, asset);

                            release.assets.append(asset);
                        }
                    }

                    //Search for suitable asset
                    if (currentArchitecture() != Architecture::Unknown && currentOperatingSystem() != OperatingSystem::Unknown)
                    {
                        for (const Asset& asset : release.assets)
                        {
                            if (asset.architecture == currentArchitecture() && asset.operatingSystem == currentOperatingSystem())
                            {
                                release.suitableAsset = asset;
                                if (asset.updatedAt.isValid())
                                {
                                    release.assetReleasedAt = asset.updatedAt;
                                }
                                else
                                {
                                    release.assetReleasedAt = asset.createdAt;
                                }

                                break;
                            }
                        }
                    }

                    //Chech if enabled release

                    bool enabled = true;

                    if (!_enabledNotValidVersion && !release.version.valid)
                    {
                        enabled = false;
                    }

                    if (!_enabledWithoutAssets && release.assets.isEmpty())
                    {
                        enabled = false;
                    }

                    if (!_enabledWithoutSuitableAsset && !release.suitableAsset.valid)
                    {
                        enabled = false;
                    }

                    if (!_enabledDraft && release.version.draft)
                    {
                        enabled = false;
                    }

                    if (!_enabledPreRelease && release.version.preRelease)
                    {
                        enabled = false;
                    }

                    if (!_enabledBeta && release.version.type == VersionType::BetaVersionType)
                    {
                        enabled = false;
                    }

                    if (!_enabledAlpha && release.version.type == VersionType::AlphaVersionType)
                    {
                        enabled = false;
                    }

                    if (!_enabledRelease && release.version.type == VersionType::ReleaseVersionType)
                    {
                        enabled = false;
                    }

                    if (enabled)
                    {
                        //qDebug() << "Suitable version:" << release.toString();

                        releases.append(release);
                    }
                    else
                    {
                        //qDebug() << "Non-suitable version:" << release.toString();
                    }
                }
            }

            //Sort and search for suitable release
            if (!releases.isEmpty())
            {
                std::sort(releases.begin(), releases.end());

                //Search for suitable release
                for (int i = releases.count() - 1; i >= 0; --i)
                {
                    const Release& release = releases.at(i);

                    if (release.suitableAsset.valid)
                    {
                        _latestRelease = release;

                        qDebug(QString("Latest available release \"%1\"")
                               .arg(_latestRelease.version.toString())
                               .toUtf8());

                        break;
                    }
                }
            }

            if (_settings)
            {
                _settings->setValue(_settingsGroup + "/latest_polling_at", QDateTime::currentDateTime());
            }

            /*qDebug(QString("Current version \"%1\", latest version \"%2\"")
                   .arg(currentVersion.toString())
                   .arg(_latestRelease.version.toString())
                   .toUtf8());*/

            const Version& currentVersion = Version::fromString(QCoreApplication::applicationVersion());

            if (currentVersion.valid)
            {
                if (_latestRelease.suitableAsset.valid && currentVersion < _latestRelease.version)
                {
                    bool skipThisVersion = false;

                    if (_settings)
                    {
                        const QStringList& skippedList = _settings->value(_settingsGroup + "/" + _settingsKeyUserSpecifiedLink).toStringList();

                        for (const QString& skippedName : skippedList)
                        {
                            const Version& skipped = Version::fromString(skippedName);

                            if (skipped.valid)
                            {
                                if (_latestRelease.version.toString().trimmed().toLower() == skipped.toString().trimmed().toLower())
                                {
                                    skipThisVersion = true;
                                    break;
                                }
                            }
                        }
                    }

                    if (!skipThisVersion)
                    {
                        _replyState = ReplyState::NewVersionAvailable;
                        qDebug(QString("Newes version found \"%1\", current version \"%2\"")
                               .arg(_latestRelease.version.toString())
                               .arg(currentVersion.toString())
                               .toUtf8());
                    }
                    else
                    {
                        _replyState = ReplyState::NewVersionAvailableButSkipped;
                        qDebug(QString("Newes version found \"%1\", current version \"%2\", but \"%1\" skipped")
                               .arg(_latestRelease.version.toString())
                               .arg(currentVersion.toString())
                               .toUtf8());
                    }
                }
                else
                {
                    _replyState = ReplyState::LatestVersionYouHave;
                }
            }
            else
            {
                _replyState = ReplyState::FailedToConnect;
                qDebug(QString("Current version \"%1\" not valid!").arg(QCoreApplication::applicationVersion()).toUtf8());
            }
        }
        else
        {
            bool ok;
            int rateLimitRemaining = reply->rawHeader("X-Ratelimit-Remaining").toInt(&ok);

            if (ok && rateLimitRemaining <= 0)
            {
                qDebug() << "GitHub reply error: RequestLimitsExceeded";
                _replyState = ReplyState::RequestLimitsExceeded;
            }
            else
            {
                qDebug() << "GitHub reply error:" << reply->error();
                _replyState = ReplyState::FailedToConnect;
            }
        }

        reply->deleteLater();
    }

    emit replied();
}

GitHubApi::Asset GitHubApi::writeArchOS(const QString &fileName, const GitHubApi::Asset &asset)
{
    Asset newAsset = asset;

    QString s = fileName;

    if (s.contains('.'))
    {
        s = s.left(s.lastIndexOf('.'));

        if (s.contains('-'))
        {
            s = s.mid(s.lastIndexOf('-') + 1).toLower().trimmed();

            //ToDo: add other archs

            //Windows
            if (s == "win64")
            {
                newAsset.operatingSystem = OperatingSystem::Windows;
                newAsset.architecture    = Architecture::x86_64;
            }
            else if (s == "win32")
            {
                newAsset.operatingSystem = OperatingSystem::Windows;
                newAsset.architecture    = Architecture::x86_32;
            }

            //Linux
            else if (s == "linux32")
            {
                newAsset.operatingSystem = OperatingSystem::Linux;
                newAsset.architecture    = Architecture::x86_32;
            }
            else if (s == "linux64")
            {
                newAsset.operatingSystem = OperatingSystem::Linux;
                newAsset.architecture    = Architecture::x86_64;
            }

            //MacOS
            else if (s == "macos64" || s == "macos" || s == "mac" || s == "mac64" || s == "darwin")
            {
                newAsset.operatingSystem = OperatingSystem::MacOS;
                newAsset.architecture    = Architecture::x86_64;
            }
        }
    }

    return newAsset;
}

GitHubApi::OperatingSystem GitHubApi::currentOperatingSystem() const
{
    #if defined (Q_OS_WIN)
    return OperatingSystem::Windows;
    #elif defined (Q_OS_LINUX)
    return OperatingSystem::Linux;
    #elif defined (Q_OS_MAC)
    return OperatingSystem::MacOS;
    #endif

    //ToDo: add other os

    return OperatingSystem::Unknown;
}

GitHubApi::Architecture GitHubApi::currentArchitecture() const
{
    const QString& arch = QSysInfo::buildCpuArchitecture().toLower().trimmed();

    if (arch == "i386")
    {
        return Architecture::x86_32;
    }
    else if (arch == "x86_64")
    {
        return Architecture::x86_64;
    }

    //ToDo: add other architectures

    return Architecture::Unknown;
}

bool GitHubApi::autoRequested() const
{
    return _autoRequested;
}

QString GitHubApi::lastVersionName() const
{
    return _latestRelease.version.toString();
}

QString GitHubApi::lastVersionDescription() const
{
    return _latestRelease.description;
}

QUrl GitHubApi::lastVersionDownloadUrl() const
{
    return _latestRelease.suitableAsset.downloadUrl;
}

QUrl GitHubApi::lastVersionHtmlUrl() const
{
    return _latestRelease.htmlUrl;
}

GitHubApi::ReplyState GitHubApi::replyState() const
{
    return _replyState;
}

QDateTime GitHubApi::lastVersionDateTime() const
{
    if (_latestRelease.assetReleasedAt.isValid())
    {
        return _latestRelease.assetReleasedAt;
    }
    else
    {
        return _latestRelease.publishedAt;
    }
}

void GitHubApi::setSkipCurrentVersion(bool skip) const
{
    if (_settings)
    {
        QStringList skippedList = _settings->value(_settingsGroup + "/" + _settingsKeyUserSpecifiedLink).toStringList();

        QString currentVersionName = _latestRelease.version.toString();

        if (skip)
        {
            if (!skippedList.contains(currentVersionName))
            {
                skippedList.append(currentVersionName);
            }
        }
        else
        {
            skippedList.removeAll(currentVersionName);
        }

        _settings->setValue(_settingsGroup + "/" + _settingsKeyUserSpecifiedLink, skippedList);
    }
}


