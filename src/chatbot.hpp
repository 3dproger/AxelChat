#ifndef CHATBOT_HPP
#define CHATBOT_HPP

#include <QSettings>
#include "chatmessage.hpp"
#include "botaction.hpp"

class ChatBot : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int volume READ volume WRITE setVolume NOTIFY volumeChanged())
    Q_PROPERTY(bool enabledCommands READ enabledCommands WRITE setEnabledCommands NOTIFY enabledCommandsChanged())

public:
    explicit ChatBot(QSettings* settings, const QString& settingsGroup, QObject *parent = nullptr);

    int volume() const;
    bool enabledCommands() const;
    void setEnabledCommands(bool enabledCommands);

    static void declareQml()
    {
        qmlRegisterUncreatableType<ChatBot> ("AxelChat.ChatBot",
                                             1, 0, "ChatBot", "Type cannot be created in QML");

        BotAction::declareQML();
    }

    QList<BotAction*> actions() const;

    void addAction(BotAction* action);
    void rewriteAction(int pos, BotAction* action);
    void deleteAction(int pos);
    void executeAction(int pos);

signals:
    void volumeChanged();
    void enabledCommandsChanged();

public slots:
    void setVolume(int volume);
    void processMessage(const ChatMessage& message);
    void execute(BotAction& action);
    QString commandsText() const;

private:
    void saveCommands();
    void loadCommands();

    QString _settingsGroupPath = "chat_bot";
    QSettings*  _settings = nullptr;

    const QString _settingsKeyVolume = "volume";
    const QString _settingsKeyEnabledCommands = "enabled_commands";
    const QString _settingsGroupActions = "actions";

    QList<BotAction*> _actions;

    bool _enabledCommands = false;

    int _volume = 100;
};

#endif // CHATBOT_HPP
