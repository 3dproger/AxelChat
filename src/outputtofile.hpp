#ifndef OUTPUTTOFILE_HPP
#define OUTPUTTOFILE_HPP

#include "chatmessage.hpp"
#include "types.hpp"
#include <QObject>
#include <QSettings>
#include <QRunnable>
#include <QWaitCondition>
#include <QMutex>
#include <QQueue>

class OutputToFile : public QObject, public QRunnable
{
    Q_OBJECT
    Q_PROPERTY(QString outputFolderPath READ outputFolder WRITE setOutputFolder NOTIFY outputFolderChanged)
    Q_PROPERTY(QString standardOutputFolder READ standardOutputFolder)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)

public:
    explicit OutputToFile(QSettings* settings, const QString& settingsGroupPath, QObject *parent = nullptr);
    ~OutputToFile();

    void run();
    void stopThread();

    bool enabled() const;
    void setEnabled(bool enabled);
    QString standardOutputFolder() const;
    QString outputFolder() const;
    void resetSettings();

    void setYouTubeInfo(const YouTubeInfo& youTubeCurrent);

signals:
    void outputFolderChanged();
    void enabledChanged();

public slots:
    void setOutputFolder(const QString &outputFolder);
    void onMessagesReceived(const ChatMessage& message, const MessageAuthor& author);
    Q_INVOKABLE void showInExplorer();
    QList<QString> codecs() const;
    void setCodec(const QString& codec);
    QString codec() const;
    bool exportToTxt(const QString& filePath);

private:
    struct AuthorInfo{
        QString name;
        QString youtubeUrl;
        int messagesCount = 0;
    };

    void saveAuthor(const ChatMessage& message, const MessageAuthor& author);

    void reinitIni();
    void writeStartupInfo();
    void writeYoutubeInfo();

    QSettings* _settings = nullptr;
    QString _settingsGroupPath = "output_to_file";

    bool _enabled = false;
    QString _outputFolder = standardOutputFolder();

    int _iniMessagesCount = 0;
    QSettings* _iniMessages  = nullptr;
    QSettings* _iniCurrent   = nullptr;

    const QString _settingsKeyEnabled          = "enabled";
    const QString _settingsKeyOutputFolder     = "output_folder";
    const QString _settingsKeyCodec            = "codec";

    QString _codec = "UTF-8";

    YouTubeInfo _youTubeInfo;

    QString _broadcastFolder;

    const QDateTime _startupDateTime = QDateTime::currentDateTime();

    QQueue<QPair<ChatMessage, MessageAuthor>> _queueForSave;

    bool _stopThread = false;
    QWaitCondition _waitCondition;
    QMutex _mutex;
};

#endif // OUTPUTTOFILE_HPP
