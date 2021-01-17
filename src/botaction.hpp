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

    static const int DEFAULT_INACTIVITY_TIME = 60000;

    QJsonObject toJson() const;

    static BotAction* fromJson(const QJsonObject& object);

    static BotAction* createSoundPlay(QStringList keywords,
                                      QUrl soundUrl,
                                      bool caseSensitive = false);
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

    QUrl soundUrl() const;

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

    int _inactivityPeriod = DEFAULT_INACTIVITY_TIME;
    QTimer _inactivityTimer;
    bool _active = true;
};

class BotActionsModel : public QAbstractListModel
{
    Q_OBJECT
public:
    BotActionsModel(QObject *parent = 0);

    enum BotActionRoles {
        Valid = Qt::UserRole + 1,
        Keywords,
        CaseSensitive
    };

    QHash<int, QByteArray> roleNames() const override {
        return _roleNames;
    }

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

private:
    static const QHash<int, QByteArray> _roleNames;
    QList<QVariant*> _data;//*data
};
