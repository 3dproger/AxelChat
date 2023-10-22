#pragma once

#include "botaction.h"
#include "models/message.h"
#include <QSettings>
#include <QMediaPlayer>

class ChatBot : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int volume READ volume WRITE setVolume NOTIFY volumeChanged())
    Q_PROPERTY(bool enabledCommands READ enabledCommands WRITE setEnabledCommands NOTIFY enabledCommandsChanged())
    Q_PROPERTY(bool includeBuiltInCommands READ includeBuiltInCommands WRITE setIncludeBuiltInCommands NOTIFY includedBuiltInCommandsChanged())

public:
    explicit ChatBot(QSettings& settings, const QString& settingsGroup, QObject *parent = nullptr);

    int volume() const;

    bool enabledCommands() const { return _enabledCommands; }
    void setEnabledCommands(bool enabledCommands);

    bool includeBuiltInCommands() const { return _includeBuiltInCommands; }
    void setIncludeBuiltInCommands(bool includeBuiltInCommands);

    static void declareQml()
    {
        qmlRegisterUncreatableType<ChatBot> ("AxelChat.ChatBot",
                                             1, 0, "ChatBot", "Type cannot be created in QML");

        BotAction::declareQML();
    }

    QList<BotAction*> actions() const;

    void addAction(BotAction* action);
    void rewriteAction(int index, BotAction* action);
    void deleteAction(int index);
    bool executeAction(int index);
    Q_INVOKABLE QString commandsText() const;

signals:
    void volumeChanged();
    void enabledCommandsChanged();
    void includedBuiltInCommandsChanged();

public slots:
    void setVolume(int volume);
    void processMessage(const std::shared_ptr<Message>& message);
    bool execute(BotAction& action);

private:
    bool canExecute(BotAction& action, const Message &message);
    void initBuiltinCommands();
    void saveCommands();
    void loadCommands();
    
    void addCommandsText(QString& text, const BotAction* action) const;

    QSettings& settings;
    const QString SettingsGroupPath = "chat_bot";

    const QString _settingsKeyVolume = "volume";
    const QString _settingsKeyEnabledCommands = "enabled_commands";
    const QString _settingsKeyIncludeBuiltInCommands = "include_builtin_commands";
    const QString _settingsGroupActions = "actions";

    QList<BotAction*> _actions;
    QList<BotAction*> _builtInActions;

    bool _enabledCommands = false;
    bool _includeBuiltInCommands = true;

    int _volume = 100;

    QMediaPlayer _player;
};
