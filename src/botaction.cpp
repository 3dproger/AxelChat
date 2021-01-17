#include "botaction.hpp"
#include "chatbot.hpp"
#include <QJsonArray>

namespace {
static const QString JSON_VAR_KEYWORDS = "keywords";
static const QString JSON_VAR_CASE_SENSITIVE = "case_sensitive";
static const QString JSON_VAR_INACTIVITY_PERIOD = "inactivity_period";
static const QString JSON_VAR_ACTION = "action";
static const QString JSON_VAR_ACTION_TYPE = "type";
static const QString JSON_VAR_SOUND_URL = "sound_url";

static const QString JSON_ACTION_TYPE_SOUND_PLAY = "sound_play";
static const QString JSON_ACTION_TYPE_UNKNOWN = "unknown";
}

BotAction::BotAction()
{

}

QUrl BotAction::soundUrl() const
{
    return _soundUrl;
}

BotAction::ActionType BotAction::type() const
{
    return _type;
}

BotAction *BotAction::createSoundPlay(QStringList keywords,
                                      QUrl soundUrl,
                                      bool caseSensitive)
{
    BotAction* action      = new BotAction();

    action->_valid         = true;
    action->_keywords      = keywords;
    action->_soundUrl      = soundUrl;
    action->_caseSensitive = caseSensitive;

    connect(&action->_inactivityTimer, &QTimer::timeout, action, &BotAction::onTimeout);

    return action;
}

bool BotAction::caseSensitive() const
{
    return _caseSensitive;
}

QStringList BotAction::keywords() const
{
    return _keywords;
}

int BotAction::inactivityPeriod() const
{
    return _inactivityPeriod;
}

void BotAction::setInactivityPeriod(int inactivityPeriod)
{
    if (_inactivityPeriod != inactivityPeriod)
    {
        _inactivityPeriod = inactivityPeriod;
        _active = true;
        _inactivityTimer.stop();
    }
}

void BotAction::onTimeout()
{
    _active = true;
}

QString BotAction::typeToJson(const BotAction::ActionType &type)
{
    switch (type) {
    case ActionType::SoundPlay:
        return JSON_ACTION_TYPE_SOUND_PLAY;
    case ActionType::Unknown:
        break;
    }

    return JSON_ACTION_TYPE_UNKNOWN;
}

BotAction::ActionType BotAction::typeFromJson(const QString &type)
{
    if (type == JSON_ACTION_TYPE_SOUND_PLAY)
    {
        return ActionType::SoundPlay;
    }

    return ActionType::Unknown;
}

const QHash<int, QByteArray> BotActionsModel::_roleNames = QHash<int, QByteArray>{
    {Valid ,            "valid"},
    {Keywords ,         "keywords"},
    {CaseSensitive ,    "caseSensitive"}
};

int BotActionsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return _data.count();
}

QVariant BotActionsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() >= _data.size())
    {
        return QVariant();
    }

    /*const QVariant* data = _data.value(index.row());

    const BotAction& action = qvariant_cast<BotAction>(*data);

    switch (role) {
    case Valid:
        return action.valid();
    case Keywords:
        return action.keywords();
    case CaseSensitive:
        return action.caseSensitive();
    default:
        return QVariant();
    }*/

    return QVariant();
}

QJsonObject BotAction::toJson() const
{
    QJsonObject rootObject;

    rootObject.insert(JSON_VAR_KEYWORDS, QJsonArray::fromStringList(_keywords));
    rootObject.insert(JSON_VAR_CASE_SENSITIVE, _caseSensitive);
    rootObject.insert(JSON_VAR_INACTIVITY_PERIOD, _inactivityPeriod);

    QJsonObject actionObject;

    actionObject.insert(JSON_VAR_ACTION_TYPE, typeToJson(_type));
    switch (_type) {
    case ActionType::SoundPlay:
        actionObject.insert(JSON_VAR_SOUND_URL, _soundUrl.toString());
        break;
    case ActionType::Unknown:
        qDebug() << "unknown action type";
        break;
    }

    rootObject.insert(JSON_VAR_ACTION, actionObject);

    return rootObject;
}

BotAction *BotAction::fromJson(const QJsonObject &object)
{
    BotAction* action = nullptr;

    const QJsonArray& keywordsArray = object.value(JSON_VAR_KEYWORDS).toArray();
    QStringList keywords;
    for (const QJsonValue& v : keywordsArray)
    {
        const QString& keyword = v.toString().trimmed();
        if (!keyword.isEmpty())
        {
            keywords.append(keyword);
        }
    }

    if (keywords.isEmpty())
    {
        qDebug() << "keywords is empty";
        return action;
    }

    const bool caseSensitive = object.value(JSON_VAR_CASE_SENSITIVE).toBool(false);
    const int inactivityPeriod = object.value(JSON_VAR_INACTIVITY_PERIOD).toInt(DEFAULT_INACTIVITY_TIME);

    const QJsonObject& actionObject = object.value(JSON_VAR_ACTION).toObject();

    const ActionType& type = typeFromJson(actionObject.value(JSON_VAR_ACTION_TYPE).toString());

    switch (type) {
    case ActionType::SoundPlay:
    {
        const QUrl& soundUrl = QUrl(actionObject.value(JSON_VAR_SOUND_URL).toString());
        action = BotAction::createSoundPlay(keywords, soundUrl, caseSensitive);
        break;
    }
    case BotAction::Unknown:
    {
        qDebug() << "unknown action type";
        break;
    }
    }

    return action;
}
