#include "botaction.hpp"
#include "chatbot.hpp"

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
