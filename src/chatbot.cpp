#include "chatbot.hpp"
#include <QSoundEffect>
#include <QJsonObject>
#include <QJsonArray>
#include <QMediaContent>

ChatBot::ChatBot(QSettings* settings, const QString& settingsGroup, QObject *parent)
    : QObject(parent)
    , _settingsGroupPath(settingsGroup)
    , _settings(settings)

{
    initBuiltinCommands();
    loadCommands();

    if (_settings)
    {
        setVolume(_settings->value(_settingsGroupPath + "/" + _settingsKeyVolume, _volume).toInt());

        setEnabledCommands(_settings->value(_settingsGroupPath + "/" + _settingsKeyEnabledCommands, _enabledCommands).toBool());

        setIncludeBuiltInCommands(_settings->value(_settingsGroupPath + "/" + _settingsKeyIncludeBuiltInCommands, _includeBuiltInCommands).toBool());
    }
}

int ChatBot::volume() const
{
    return _volume;
}

void ChatBot::setEnabledCommands(bool enabledCommands)
{
    if (_enabledCommands != enabledCommands)
    {
        _enabledCommands = enabledCommands;

        if (_settings)
        {
            _settings->setValue(_settingsGroupPath + "/" + _settingsKeyEnabledCommands, enabledCommands);
        }

        emit enabledCommandsChanged();
    }
}

void ChatBot::setIncludeBuiltInCommands(bool includeBuiltInCommands)
{
    if (_includeBuiltInCommands != includeBuiltInCommands)
    {
        _includeBuiltInCommands = includeBuiltInCommands;

        if (_settings)
        {
            _settings->setValue(_settingsGroupPath + "/" + _settingsKeyIncludeBuiltInCommands, includeBuiltInCommands);
        }

        emit includedBuiltInCommandsChanged();
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

void ChatBot::rewriteAction(int index, BotAction *action)
{
    if (!action)
    {
        qDebug() << "!action";
        return;
    }

    if (index < 0)
    {
        qDebug() << "index < 0";
        return;
    }

    if (index >= _actions.count())
    {
        qDebug() << "index >= _actions.count()";
        return;
    }

    _actions[index] = action;

    saveCommands();
}

void ChatBot::deleteAction(int index)
{
    if (index < 0)
    {
        qDebug() << "index < 0";
        return;
    }

    if (index >= _actions.count())
    {
        qDebug() << "index >= _actions.count()";
        return;
    }

    delete _actions[index];
    _actions.removeAt(index);

    saveCommands();
}

void ChatBot::executeAction(int index)
{
    if (index < 0)
    {
        qDebug() << "index < 0";
        return;
    }

    if (index >= _actions.count())
    {
        qDebug() << "index >= _actions.count()";
        return;
    }

    execute(*_actions[index]);
}

void ChatBot::setVolume(int volume)
{
    if (_volume != volume)
    {
        _volume = volume;
        _player.setVolume(volume);

        if (_settings)
        {
            _settings->setValue(_settingsGroupPath + "/" + _settingsKeyVolume, volume);
        }

        emit volumeChanged();
    }
}

void ChatBot::processMessage(const ChatMessage &message)
{
    for (BotAction* action : _actions)
    {
        if (canExecute(*action, message))
        {
            message.setIsBotCommand(true);
            execute(*action);
        }
    }

    if (_includeBuiltInCommands)
    {
        for (BotAction* action : _builtInActions)
        {
            if (canExecute(*action, message))
            {
                message.setIsBotCommand(true);
                execute(*action);
            }
        }
    }
}

bool ChatBot::canExecute(BotAction& action, const ChatMessage &message)
{
    const QString& trimmed = message.text().trimmed();
    const QString& lowered = trimmed.toLower();

    if (action.caseSensitive())
    {
        for (const QString& keyword : action.keywords())
        {
            if (keyword == trimmed)
            {
                return true;
            }
        }
    }
    else
    {
        for (const QString& keyword : action.keywords())
        {
            if (keyword.toLower() == lowered)
            {
                return true;
            }
        }
    }

    return false;
}

void ChatBot::execute(BotAction &action)
{
    if (!_enabledCommands)
    {
        qDebug() << "commands is disabled!";
        return;
    }

    if (!action._active)
    {
        qDebug() << "The period of inactivity has not yet passed";
        return;
    }

    switch (action._type)
    {
    case BotAction::ActionType::SoundPlay:
    {
        qDebug() << "Playing" << action.soundUrl().toString();

        _player.setVolume(_volume);
        _player.setMedia(QMediaContent(action.soundUrl()));
        _player.play();
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
        if (action.exclusiveInactivityPeriod())
        {
            if (action._inactivityPeriod > 0)
            {
                action._inactivityTimer.start(action._inactivityPeriod * 1000);
            }
            else
            {
                action._active = true;
            }
        }
        else
        {
            action._inactivityTimer.start(BotAction::DEFAULT_INACTIVITY_TIME * 1000);
        }
    }
}

QString ChatBot::commandsText() const
{
    QString text;

    text += "=================== " + tr("Custom commands") + " ===================\n";

    for (int i = 0; i < _actions.count(); ++i)
    {
        addLineToCommandsText(text, _actions.at(i));
    }

    text += "\n=================== " + tr("Built-in commands") + " ===================\n";

    for (int i = 0; i < _builtInActions.count(); ++i)
    {
        BotAction* action = _builtInActions.at(i);
        if (!action->soundUrl().toString().contains("2nd_channel"))
        {
            addLineToCommandsText(text, action);
        }
    }

    text += "\n=================== " + tr("2nd Channel Edition") + " ===================\n";

    for (int i = 0; i < _builtInActions.count(); ++i)
    {
        BotAction* action = _builtInActions.at(i);
        if (action->soundUrl().toString().contains("2nd_channel"))
        {
            addLineToCommandsText(text, action);
        }
    }

    return text;
}

void ChatBot::addLineToCommandsText(QString& text, const BotAction* action) const
{

    for (int j = 0; j < action->_keywords.count(); ++j)
    {
        const QString& keyword = action->_keywords.at(j);

        text += keyword;

        if (j < action->_keywords.count() - 1)
        {
            text += " = ";
        }
    }

    text += "\n";
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

void ChatBot::initBuiltinCommands()
{
    _builtInActions.append(BotAction::createSoundPlay({"!cj"}, QUrl("qrc:/resources/sound/commands/cj.wav")));
    _builtInActions.append(BotAction::createSoundPlay({"!фиаско", "!fiasko", "!fiasco"}, QUrl("qrc:/resources/sound/commands/fiasko.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!финал", "!final"}, QUrl("qrc:/resources/sound/commands/final.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!firstblood", "!перваякровь"}, QUrl("qrc:/resources/sound/commands/firstblood.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!гэндальф", "!гендальф", "!гэндалф", "!гендалф", "!gandalf", "!gendalf"}, QUrl("qrc:/resources/sound/commands/gandalf_shallnotpass.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!run", "!ран", "!беги"}, QUrl("qrc:/resources/sound/commands/run.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!грусть", "!грустно", "!скрипка", "!sad"}, QUrl("qrc:/resources/sound/commands/sad-violin.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!тромбон"}, QUrl("qrc:/resources/sound/commands/sad_trombone.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!вау", "!воу", "!wow"}, QUrl("qrc:/resources/sound/commands/wow.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!xp", "!хр"}, QUrl("qrc:/resources/sound/commands/xp.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!cforce", "!contraforce"}, QUrl("qrc:/resources/sound/commands/sfx_bill_v5.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!БенАплодисменты", "!БэнАплодисменты", "!БенФантом", "!БэнФантом", "!аплодисментыМужику"}, QUrl("qrc:/resources/sound/commands/ben_applause.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!lockAndLoad", "!chcLockAndLoad"}, QUrl("qrc:/resources/sound/commands/chc_lock_and_load.wav")));
    _builtInActions.append(BotAction::createSoundPlay({"!фура", "!fura", "!фураБольшая", "!fura", "!furaBig", "!bigFura"}, QUrl("qrc:/resources/sound/commands/fura_big.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!ps", "!ps1", "!playstation", "!playstation1", "!sony", "!плойка"}, QUrl("qrc:/resources/sound/commands/playstation.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!чкгВолчок", "!chgkVolchok"}, QUrl("qrc:/resources/sound/commands/chgk_volchok.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!пчМимо"}, QUrl("qrc:/resources/sound/commands/pch_mimo.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!аплодисменты"}, QUrl("qrc:/resources/sound/commands/applause.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!airhorn"}, QUrl("qrc:/resources/sound/commands/airhorn.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!badumts"}, QUrl("qrc:/resources/sound/commands/budum_tss.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!mchummer"}, QUrl("qrc:/resources/sound/commands/cant-touch-this.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!сверчки", "!неСмешно"}, QUrl("qrc:/resources/sound/commands/crickets.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!doIt", "!justDoIt"}, QUrl("qrc:/resources/sound/commands/do_it.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!неожиданно", "!неожидано"}, QUrl("qrc:/resources/sound/commands/dramatic.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!missionComplited"}, QUrl("qrc:/resources/sound/commands/gta-sa-done.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!хаха"}, QUrl("qrc:/resources/sound/commands/ha-ha.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!hadouken"}, QUrl("qrc:/resources/sound/commands/hadouken.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!monsterkill"}, QUrl("qrc:/resources/sound/commands/monsterkill.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!nooo"}, QUrl("qrc:/resources/sound/commands/nooo.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!omaewamou"}, QUrl("qrc:/resources/sound/commands/omaewamou.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!omaewamou2"}, QUrl("qrc:/resources/sound/commands/omaewamou2.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!грусть2"}, QUrl("qrc:/resources/sound/commands/sadaffleck.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!toBeContinued"}, QUrl("qrc:/resources/sound/commands/to_be_continued.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!tuturu"}, QUrl("qrc:/resources/sound/commands/tuturu.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!остынь"}, QUrl("qrc:/resources/sound/commands/wc_cool.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!неплохо"}, QUrl("qrc:/resources/sound/commands/wc_good.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!полежу", "!пойдуПолежу"}, QUrl("qrc:/resources/sound/commands/wc_lie_down.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!noTime"}, QUrl("qrc:/resources/sound/commands/wc_notime.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!чемТыЗанимаешься"}, QUrl("qrc:/resources/sound/commands/wc_what_are_you.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!xfiles", "!x-files"}, QUrl("qrc:/resources/sound/commands/x-files.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!ГГ", "!GG"}, QUrl("qrc:/resources/sound/commands/gg.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!кончено", "!ofCourse"}, QUrl("qrc:/resources/sound/commands/of_course.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!фшха", "!неСмешно2"}, QUrl("qrc:/resources/sound/commands/nuzhdiki.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!здоровоОтец", "!здаровоОтец", "!здороваОтец", "!здароваОтец"}, QUrl("qrc:/resources/sound/commands/great_father.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!пиво", "!pivo", "!beer"}, QUrl("qrc:/resources/sound/commands/pivo.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!вотЭтоПоворот"}, QUrl("qrc:/resources/sound/commands/vot_eto_povorot.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!накосячил", "!косяк"}, QUrl("qrc:/resources/sound/commands/накосячил.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!чтоЭто", "!чувакЧтоЭто", "!wat", "!what"}, QUrl("qrc:/resources/sound/commands/chto-eto.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!mkFatality", "!фаталити", "!мкФаталити", "!fatality"}, QUrl("qrc:/resources/sound/commands/fatality.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!mkFinishhim", "!finishhim"}, QUrl("qrc:/resources/sound/commands/finishhim.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!mkFight", "!fight", "!бой", "!вбой"}, QUrl("qrc:/resources/sound/commands/mk-fight.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!mkToasty", "!toasty"}, QUrl("qrc:/resources/sound/commands/mk-toasty.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!mkRound1", "!round1", "раунд1", "!мкРаунд1"}, QUrl("qrc:/resources/sound/commands/mk-round1.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!mkRound2", "!round2", "раунд2", "!мкРаунд2"}, QUrl("qrc:/resources/sound/commands/mk-round2.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!mkRound3", "!round3", "раунд3", "!мкРаунд3"}, QUrl("qrc:/resources/sound/commands/mk-round3.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!дружкоНеБуду", "!дружко"}, QUrl("qrc:/resources/sound/commands/drujko-ne-budu.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!неТочно", "!ноЭтоНеТочно", "!брб"}, QUrl("qrc:/resources/sound/commands/ne-tochno.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!astalavista", "!hastaLaVista", "!асталависта"}, QUrl("qrc:/resources/sound/commands/astalavista.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!initial1", "!initialD1", "!eurobeat1", "!евробит1", "!dejavu", "!deja-vu", "!дежавю", "!дежа-вю", "!дежаву", "!дежа-ву", "!дэжаву", "!дэжавю"}, QUrl("qrc:/resources/sound/commands/deja-vu.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!initial2", "!initialD2", "!eurobeat2", "!евробит2", "!газ", "!гас", "!gas", "!gaz"}, QUrl("qrc:/resources/sound/commands/gas.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!initial3", "!initialD3", "!eurobeat3", "!евробит3", "!90s", "!90е"}, QUrl("qrc:/resources/sound/commands/eurobeat.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!saxophone", "!saxophon", "!саксофон"}, QUrl("qrc:/resources/sound/commands/saxophone.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!fly", "!ICanFly"}, QUrl("qrc:/resources/sound/commands/fly.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!suspense1", "!саспенс1", "!суспенс1", "!suspens1", "!saspense1", "!saspens1", "!psycho"}, QUrl("qrc:/resources/sound/commands/psycho.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!suspense2", "!саспенс2", "!суспенс2", "!suspens2", "!saspense2", "!saspens2", "!барабаннаяДробь"}, QUrl("qrc:/resources/sound/commands/saspens.mp3")));

    // !МыГГ !gman (it's time to choose) !shaokan (you will never win)
    //ToDo: добавить: !ура,  !боль,  !позабавимся,  !обмыть,  !твари, !gameover, !lol, !пиво, !clear, !go, !negative, !победа, !rtrKakTak, !rtrGlavnoe, !дошик, !фиаско2, !zapab, !rtrZaruinil, !rtrZloyBoss, !rtrNadejda, !rtrЁКЛМН, !rtrNevezenie, !GoBen4, !jivi, !rtrPobedaBoss, !rtrBezuprechno, !rtrZlieBossi

    // by 2nd channel
    _builtInActions.append(BotAction::createSoundPlay({"!viknickLol", "!викникЛол", "!викникСмех", "!викник"}, QUrl("qrc:/resources/sound/commands/2nd_channel/2nd Channel-Викник смех.wav")));
    _builtInActions.append(BotAction::createSoundPlay({"!факт", "!этоФакт"}, QUrl("qrc:/resources/sound/commands/2nd_channel/2nd Channel-Это факт.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!гудлак", "!321GoodLuck"}, QUrl("qrc:/resources/sound/commands/2nd_channel/2nd Channel-3-2-1 - Good Luck-kissvk.com.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!гудлак2"}, QUrl("qrc:/resources/sound/commands/2nd_channel/2nd Channel-Gin Chin Good Luck.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!гудлак3"}, QUrl("qrc:/resources/sound/commands/2nd_channel/2nd Channel-Ни пуха ни Good Luck-kissvk.com.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!неалкоголики"}, QUrl("qrc:/resources/sound/commands/2nd_channel/2nd Channel-Анонимных неалкоголиков сборище-kissvk.com.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!вернутьВСемью"}, QUrl("qrc:/resources/sound/commands/2nd_channel/2nd Channel-Вернуть в семью Артура-kissvk.com.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!нетривиальный"}, QUrl("qrc:/resources/sound/commands/2nd_channel/2nd Channel-Вот таким нетривиальным способом-kissvk.com.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!доступЗапрещён", "!доступЗапрещен"}, QUrl("qrc:/resources/sound/commands/2nd_channel/2nd Channel-Доступ запрещён - Всем БАН. До Свидания-kissvk.com.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!измерениеХ", "!измерениеX"}, QUrl("qrc:/resources/sound/commands/2nd_channel/2nd Channel-Из измерения X возвращается-kissvk.com.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!тетрисВОкне"}, QUrl("qrc:/resources/sound/commands/2nd_channel/2nd Channel-Кто-то там играет в Тетрис-kissvk.com.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!МаксимПодтверди"}, QUrl("qrc:/resources/sound/commands/2nd_channel/2nd Channel-Максим подтверди - Подтверждаю-kissvk.com.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!чутьНеЗасосало"}, QUrl("qrc:/resources/sound/commands/2nd_channel/2nd Channel-Меня чуть не засосало-kissvk.com.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!разрабатываемТНУ4"}, QUrl("qrc:/resources/sound/commands/2nd_channel/2nd Channel-Мы оказывается разрабатываем TNU4-kissvk.com.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!удаляйтеИнтернет"}, QUrl("qrc:/resources/sound/commands/2nd_channel/2nd Channel-Не удаляйте мне интернет на компьютере-kissvk.com.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!РафаДостают"}, QUrl("qrc:/resources/sound/commands/2nd_channel/2nd Channel-Ну не надо меня доставать-kissvk.com.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!неСудьба"}, QUrl("qrc:/resources/sound/commands/2nd_channel/2nd Channel-Ну не судьба-kissvk.com.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!неБаньте"}, QUrl("qrc:/resources/sound/commands/2nd_channel/2nd Channel-Пожалуйста не баньте-kissvk.com.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!неХочуДумать"}, QUrl("qrc:/resources/sound/commands/2nd_channel/2nd Channel-Пожалуйста. Я не хочу думать об этом-kissvk.com.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!турболёт", "!турболет"}, QUrl("qrc:/resources/sound/commands/2nd_channel/2nd Channel-Полный турболёт-kissvk.com.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!понятноСТобой"}, QUrl("qrc:/resources/sound/commands/2nd_channel/2nd Channel-Понятно всё с тобой. Ничем ты не поможешь мне-kissvk.com.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!рисоватьИзврат"}, QUrl("qrc:/resources/sound/commands/2nd_channel/2nd Channel-Рисовать - это изврат-kissvk.com.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!электрод"}, QUrl("qrc:/resources/sound/commands/2nd_channel/2nd Channel-Сейчас так ХОП-kissvk.com.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!скринь", "!скинь", "!скрин", "!скинь", "!скин"}, QUrl("qrc:/resources/sound/commands/2nd_channel/2nd Channel-Скинь скринь скринъ-kissvk.com.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!потерялЖизнь"}, QUrl("qrc:/resources/sound/commands/2nd_channel/2nd Channel-Теперь я потерял жизнь-kissvk.com.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!терминаторЗабеременел"}, QUrl("qrc:/resources/sound/commands/2nd_channel/2nd Channel-Терминатор забеременел Робокопом-kissvk.com.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!чикса", "!бикса", "!чика"}, QUrl("qrc:/resources/sound/commands/2nd_channel/2nd Channel-Что надо чикса-бикса-kissvk.com.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!РафНеХочет"}, QUrl("qrc:/resources/sound/commands/2nd_channel/2nd Channel-Чёрт. Нет. Почему. Я не хочу-kissvk.com.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!вИнтернетеЗабаню"}, QUrl("qrc:/resources/sound/commands/2nd_channel/2nd Channel-Я тебя в интернете забаню-kissvk.com.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!баньСебяСам"}, QUrl("qrc:/resources/sound/commands/2nd_channel/2nd Channel- Бань себя сам-kissvk.com.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!терминаторДомогался"}, QUrl("qrc:/resources/sound/commands/2nd_channel/2nd Channel-Меня Терминатор домогался-kissvk.com.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!быстроТыПоел"}, QUrl("qrc:/resources/sound/commands/2nd_channel/2nd Channel-Да быстро ты поел-kissvk.com.mp3")));
    _builtInActions.append(BotAction::createSoundPlay({"!скрытаяРеклама"}, QUrl("qrc:/resources/sound/commands/2nd_channel/2nd Channel-Скрытая реклама.mp3")));
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
        _settings->setValue(_settingsGroupPath + "/" + _settingsGroupActions + QString("/%1").arg(i), action->toJson());
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

