#pragma once

#include <QStringList>
#include <QUrl>
#include <QDateTime>
#include <QTimer>
#include <QAbstractListModel>
#include <QQmlEngine>
#include <QJsonObject>

class ChatBot;

class BotAction : public QObject
{
    Q_OBJECT
public:
    enum ActionType {
        SoundPlay,
        //ToDo: добавить новый функционал
        Unknown
    };
    Q_ENUMS(ActionType)

    static const int DEFAULT_INACTIVITY_TIME = 10;

    QJsonObject toJson() const;

    static BotAction* fromJson(const QJsonObject& object);

    static BotAction* createSoundPlay(const QStringList& keywords, const QString& soundFile, bool caseSensitive = false);
    static BotAction* createSoundPlay(const QStringList& keywords, const QUrl& soundUrl, bool caseSensitive = false);

    bool caseSensitive() const;

    QStringList keywords() const;

    int inactivityPeriod() const;
    void setInactivityPeriod(int inactivityPeriod);

    inline bool valid() const
    {
        return _valid;
    }

    static void declareQML()
    {
        qmlRegisterUncreatableType<BotAction>("AxelChat.BotAction",
                                     1, 0, "BotAction", "Type cannot be created in QML");
    }

    ActionType type() const;

    QUrl soundUrl() const { return _soundUrl; }

    bool exclusiveInactivityPeriod() const;
    void setExclusiveInactivityPeriod(bool exclusiveInactivityPeriod);

protected:

private slots:
    void onTimeout();

private:
    static QString typeToJson(const ActionType& type);
    static ActionType typeFromJson(const QString& type);

    BotAction();

    friend class ChatBot;

    bool _valid = false;

    QStringList _keywords;
    bool _caseSensitive = false;
    ActionType _type = ActionType::SoundPlay;
    QUrl _soundUrl;

    bool _exclusiveInactivityPeriod = false;
    int _inactivityPeriod = DEFAULT_INACTIVITY_TIME;
    QTimer _inactivityTimer;
    bool _active = true;
};

