#ifndef OUTPUTTOFILE_HPP
#define OUTPUTTOFILE_HPP

#include <QObject>
#include <QSettings>
#include "chatmessage.hpp"
#include "types.hpp"

class OutputToFile : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString outputFolderPath READ outputFolder WRITE setOutputFolder NOTIFY outputFolderChanged)
    Q_PROPERTY(QString standardOutputFolder READ standardOutputFolder)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)

public:
    enum OutputToFileCodec
    {
        UTF8Codec = 0,
        ANSICodec = 100,
        ANSIWithUTF8Codec = 200
    };

    explicit OutputToFile(QSettings* settings, const QString& settingsGroupPath, QObject *parent = nullptr);
    ~OutputToFile();

    bool enabled() const;
    void setEnabled(bool enabled);
    QString standardOutputFolder() const;
    QString outputFolder() const;
    void resetSettings();

    void setYouTubeInfo(const AxelChat::YouTubeInfo& youTubeCurrent);
    void setTwitchInfo(const AxelChat::TwitchInfo& twitchCurrent);
    void setGoodGameInfo(const AxelChat::GoodGameInfo& goodGameCurrent);

    Q_INVOKABLE bool setCodecOption(int option, bool applyWithoutReset); // return true if need restart
    Q_INVOKABLE int codecOption() const;

signals:
    void outputFolderChanged();
    void enabledChanged();

public slots:
    void setOutputFolder(QString outputFolder);
    void onMessagesReceived(const ChatMessage& message);
    Q_INVOKABLE void showInExplorer();

private slots:
    void writeMessage(const QList<QPair<QString, QString>> tags /*<tagName, tagValue>*/);

private:
    QByteArray prepare(const QString& text);

    struct AuthorInfo{
        QString name;
        QString youtubeUrl;
        int messagesCount = 0;
    };

    void reinit(bool forceUpdateOutputFolder);
    void writeStartupInfo(const QString& messagesFolder);
    void writeInfo();

    QSettings* _settings = nullptr;
    QString _settingsGroupPath = "output_to_file";

    bool _enabled = false;
    QString _outputFolder = standardOutputFolder();

    QFile* _fileMessagesCount = nullptr;
    QFile* _fileMessages = nullptr;
    QSettings* _iniCurrentInfo   = nullptr;

    const QString _settingsKeyEnabled          = "enabled";
    const QString _settingsKeyOutputFolder     = "output_folder";
    const QString _settingsKeyCodec            = "codec";

    OutputToFileCodec _codec = OutputToFileCodec::UTF8Codec;

    AxelChat::YouTubeInfo _youTubeInfo;
    AxelChat::TwitchInfo _twitchInfo;
    AxelChat::GoodGameInfo _goodGameInfo;

    const QDateTime _startupDateTime = QDateTime::currentDateTime();

    QString _messagesFolder;
};

#endif // OUTPUTTOFILE_HPP
