#include "chatbot.hpp"
#include <QSound>
#include <QSoundEffect>
#include <QJsonObject>
#include <QJsonArray>

ChatBot::ChatBot(QSettings* settings, const QString& settingsGroup, QObject *parent)
    : QObject(parent)
    , _settingsGroupPath(settingsGroup)
    , _settings(settings)

{
    loadCommands();

    if (_settings)
    {
        setVolume(_settings->value(_settingsGroupPath + "/" + _settingsKeyVolume, _volume).toInt());

        setEnabledSound(_settings->value(_settingsGroupPath + "/" + _settingsKeyEnabledSound, false).toBool());
    }
}

int ChatBot::volume() const
{
    return _volume;
}

bool ChatBot::enabledSound() const
{
    return _enabledSound;
}

void ChatBot::setEnabledSound(bool enabledSound)
{
    if (_enabledSound != enabledSound)
    {
        _enabledSound = enabledSound;

        if (_settings)
        {
            _settings->setValue(_settingsGroupPath + "/" + _settingsKeyEnabledSound, enabledSound);
        }

        emit enabledSoundChanged();
    }
}

QList<BotAction *> ChatBot::actions() const
{
    return _actions;
}

void ChatBot::addAction(BotAction *action)
{
    if (!action)
    {
        qDebug() << "!action";
        return;
    }

    if (!_settings)
    {
        qDebug() << "!_settings";
        return;
    }

    _actions.append(action);
    saveCommands();
}

void ChatBot::rewriteAction(int pos, BotAction *action)
{
    if (!action)
    {
        qDebug() << "!action";
        return;
    }

    if (pos < 0)
    {
        qDebug() << "pos < 0";
        return;
    }

    if (pos >= _actions.count())
    {
        qDebug() << "pos >= _actions.count()";
        return;
    }

    _actions[pos] = action;

    saveCommands();
}

void ChatBot::deleteAction(int pos)
{
    if (pos < 0)
    {
        qDebug() << "pos < 0";
        return;
    }

    if (pos >= _actions.count())
    {
        qDebug() << "pos >= _actions.count()";
        return;
    }

    delete _actions[pos];
    _actions.removeAt(pos);

    saveCommands();
}

void ChatBot::executeAction(int pos)
{
    if (pos < 0)
    {
        qDebug() << "pos < 0";
        return;
    }

    if (pos >= _actions.count())
    {
        qDebug() << "pos >= _actions.count()";
        return;
    }

    execute(*_actions[pos]);
}

void ChatBot::setVolume(int volume)
{
    if (_volume != volume)
    {
        _volume = volume;

        if (_settings)
        {
            _settings->setValue(_settingsGroupPath + "/" + _settingsKeyVolume, volume);
        }

        emit volumeChanged();
    }
}

void ChatBot::processMessage(const ChatMessage &message)
{
    const QString& trimmed = message.text().trimmed();
    const QString& lowered = trimmed.toLower();

    for (BotAction* action : _actions)
    {
        if (action->caseSensitive())
        {
            for (const QString& keyword : action->keywords())
            {
                if (keyword == trimmed)
                {
                    message.setIsBotCommand(true);
                    execute(*action);
                }
            }
        }
        else
        {
            for (const QString& keyword : action->keywords())
            {
                if (keyword.toLower() == lowered)
                {
                    message.setIsBotCommand(true);
                    execute(*action);
                }
            }
        }
    }
}

void ChatBot::execute(BotAction &action)
{
    if (!action._active)
    {
        qDebug() << "The period of inactivity has not yet passed";
        return;
    }

    switch (action._type)
    {
    case BotAction::ActionType::SoundPlay:
    {
        if (_enabledSound)
        {
            QSound::play(action._soundUrl.toString());
            qDebug() << "Playing" << action._soundUrl.toString();
        }
    }
        break;
    case BotAction::ActionType::Unknown:
    {
        qDebug() << "unknown action type";
    }
        break;
    }

    if (action._inactivityPeriod > 0)
    {
        action._active = false;
        action._inactivityTimer.start(action._inactivityPeriod);
    }
}

QString ChatBot::commandsText() const
{
    QString s;

    for (int i = 0; i < _actions.count(); ++i)
    {
        const BotAction* action = _actions.at(i);
        for (int j = 0; j < action->_keywords.count(); ++j)
        {
            const QString& keyword = action->_keywords.at(j);

            s += keyword;

            if (j < action->_keywords.count() - 1)
            {
                s += " = ";
            }
        }

        s += "\n";
    }

    return s;
}

QStringList keysInGroup(QSettings& settings, const QString& path)
{
    int depth = 0;
    QString group;
    for (int i = 0; i < path.count(); ++i)
    {
        const QChar& c = path[i];

        if (c == '/')
        {
            settings.beginGroup(group);
            group.clear();
            depth++;
        }
        else
        {
            group += c;
        }
    }

    if (!group.isEmpty())
    {
        settings.beginGroup(group);
        group.clear();
        depth++;
    }

    const QStringList& keys = settings.allKeys();

    for (int i = 0; i < depth; ++i)
    {
        settings.endGroup();
    }

    return keys;
}

void ChatBot::saveCommands()
{
    if (!_settings)
    {
        qDebug() << "!_settings";
        return;
    }

    for (int i = 0; i < _actions.count(); ++i)
    {
        BotAction* action = _actions[i];
        _settings->setValue(_settingsGroupPath + "/" + _settingsGroupActions + QString("/%1").arg(i),
                            action->toJson());
    }

    const QStringList& keysToRemove = keysInGroup(*_settings, _settingsGroupPath + "/" + _settingsGroupActions);

    for (int i = _actions.count(); i < keysToRemove.count(); ++i)
    {
        _settings->remove(_settingsGroupPath + "/" + _settingsGroupActions + "/" + keysToRemove[i]);
    }
}

void ChatBot::loadCommands()
{
    if (!_settings)
    {
        qDebug() << "!_settings";
        return;
    }

    const QStringList& commandsGroups = keysInGroup(*_settings, _settingsGroupPath + "/" + _settingsGroupActions);

    for (const QString& commandGroup : commandsGroups)
    {        const QJsonObject& object = _settings->value(_settingsGroupPath + "/" + _settingsGroupActions + "/" + commandGroup,
                         QJsonObject())
                .toJsonObject();

        if (!object.isEmpty())
        {
            BotAction* action = BotAction::fromJson(object);
            if (action)
            {
                _actions.append(action);
            }
        }
    }
}

