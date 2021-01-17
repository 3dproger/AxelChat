#ifndef CHATBOT_HPP
#define CHATBOT_HPP

#include <QObject>
#include <QMediaPlayer>
#include <QSettings>
#include "chatmessage.hpp"
#include "botaction.hpp"

class ChatBot : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int volume READ volume WRITE setVolume NOTIFY volumeChanged())
    Q_PROPERTY(bool enabledSound READ enabledSound WRITE setEnabledSound NOTIFY enabledSoundChanged())

public:
    explicit ChatBot(QSettings* settings, const QString& settingsGroup, QObject *parent = nullptr);

    int volume() const;
    bool enabledSound() const;
    void setEnabledSound(bool enabledSound);

    static void declareQml()
    {
        qmlRegisterUncreatableType<ChatBot> ("AxelChat.ChatBot",
                                             1, 0, "ChatBot", "Type cannot be created in QML");

        BotAction::declareQML();
    }

    QList<BotAction*> actions() const
    {
        return _actions;
    }

    void addAction(BotAction* action);
    void rewriteAction(int pos, BotAction* action);
    void deleteAction(int pos);
    void executeAction(int pos);

signals:
    void volumeChanged();
    void enabledSoundChanged();

public slots:
    void setVolume(int volume);
    void processMessage(const ChatMessage& message);
    void execute(BotAction& action);
    QString commandsText() const;

private slots:
    void onMediaPlayerError(QMediaPlayer::Error error);
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);

private:
    QMediaPlayer* _mediaPlayer = new QMediaPlayer(this, QMediaPlayer::LowLatency);

    QList<BotAction*> _actions;

    QString _settingsGroupPath = "chat_bot";
    QSettings*  _settings = nullptr;

    const QString _settingsKeyVolume = "volume";
    const QString _settingsKeyEnabledSound = "enabledSound";

    bool _enabledSound = false;
};

#endif // CHATBOT_HPP
