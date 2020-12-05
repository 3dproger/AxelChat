#include "chatbot.hpp"

ChatBot::ChatBot(QSettings* settings, const QString& settingsGroup, QObject *parent) : QObject(parent)
{
    _settingsGroupPath = settingsGroup;
    _settings = settings;

    if (_settings)
    {
        setVolume(_settings->value(_settingsGroupPath + "/" + _settingsKeyVolume, 10).toInt());

        setEnabledSound(_settings->value(_settingsGroupPath + "/" + _settingsKeyEnabledSound, false).toBool());
    }

    connect(_mediaPlayer, SIGNAL(error(QMediaPlayer::Error)),                    this, SLOT(onMediaPlayerError(QMediaPlayer::Error)));
    connect(_mediaPlayer, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), this, SLOT(onMediaStatusChanged(QMediaPlayer::MediaStatus)));

    //ToDo: перенести в редактор
    //Встроенные голосовые команды
    _actions.append(BotAction::createSoundPlay({"!cj"}, QUrl("qrc:/resources/sound/cj.wav")));
    _actions.append(BotAction::createSoundPlay({"!фиаско", "!fiasko", "!fiasco"}, QUrl("qrc:/resources/sound/fiasko.mp3")));
    _actions.append(BotAction::createSoundPlay({"!финал", "!final"}, QUrl("qrc:/resources/sound/final.mp3")));
    _actions.append(BotAction::createSoundPlay({"!firstblood", "!перваякровь"}, QUrl("qrc:/resources/sound/firstblood.mp3")));
    _actions.append(BotAction::createSoundPlay({"!гэндальф", "!гендальф", "!гэндалф", "!гендалф", "!gandalf", "!gendalf"}, QUrl("qrc:/resources/sound/gandalf_shallnotpass.mp3")));
    _actions.append(BotAction::createSoundPlay({"!run", "!ран", "!беги"}, QUrl("qrc:/resources/sound/run.mp3")));
    _actions.append(BotAction::createSoundPlay({"!грусть", "!грустно", "!скрипка", "!sad"}, QUrl("qrc:/resources/sound/sad-violin.mp3")));
    _actions.append(BotAction::createSoundPlay({"!тромбон"}, QUrl("qrc:/resources/sound/sad_trombone.mp3")));
    _actions.append(BotAction::createSoundPlay({"!вау", "!воу", "!wow"}, QUrl("qrc:/resources/sound/wow.mp3")));
    _actions.append(BotAction::createSoundPlay({"!xp", "!хр"}, QUrl("qrc:/resources/sound/xp.mp3")));
    _actions.append(BotAction::createSoundPlay({"!cforce", "!contraforce"}, QUrl("qrc:/resources/sound/sfx_bill_v5.mp3")));
    _actions.append(BotAction::createSoundPlay({"!аплодисментыМужику"}, QUrl("qrc:/resources/sound/ben_applause.mp3")));
    _actions.append(BotAction::createSoundPlay({"!lockAndLoad", "!chcLockAndLoad"}, QUrl("qrc:/resources/sound/chc_lock_and_load.wav")));
    _actions.append(BotAction::createSoundPlay({"!фура", "!fura", "!фураБольшая", "!fura", "!furaBig", "!bigFura"}, QUrl("qrc:/resources/sound/fura_big.mp3")));
    _actions.append(BotAction::createSoundPlay({"!ps", "!ps1", "!playstation", "!playstation1", "!sony", "!плойка"}, QUrl("qrc:/resources/sound/playstation.mp3")));
    _actions.append(BotAction::createSoundPlay({"!viknickLol", "!викникЛол", "!викникСмех", "!викник"}, QUrl("qrc:/resources/sound/viknick_smeh.wav")));
    _actions.append(BotAction::createSoundPlay({"!чкгВолчок", "!chgkVolchok"}, QUrl("qrc:/resources/sound/chgk_volchok.mp3")));
    _actions.append(BotAction::createSoundPlay({"!чкгЧёрныйЯщик", "!чкгЧерныйЯщик", "!chgkBlackBox"}, QUrl("qrc:/resources/sound/chgk_yashik.mp3")));
    _actions.append(BotAction::createSoundPlay({"!пчПодумай"}, QUrl("qrc:/resources/sound/pch_60_seconds.mp3")));
    _actions.append(BotAction::createSoundPlay({"!пчАвтомобиль"}, QUrl("qrc:/resources/sound/pch_avtomobil.mp3")));
    _actions.append(BotAction::createSoundPlay({"!пчМимо"}, QUrl("qrc:/resources/sound/pch_mimo.mp3")));
    _actions.append(BotAction::createSoundPlay({"!пчПобеда"}, QUrl("qrc:/resources/sound/pch_winner.mp3")));
    _actions.append(BotAction::createSoundPlay({"!аплодисменты"}, QUrl("qrc:/resources/sound/applause.mp3")));
    _actions.append(BotAction::createSoundPlay({"!airhorn"}, QUrl("qrc:/resources/sound/airhorn.mp3")));
    _actions.append(BotAction::createSoundPlay({"!badumts"}, QUrl("qrc:/resources/sound/budum_tss.mp3")));
    _actions.append(BotAction::createSoundPlay({"!mchummer"}, QUrl("qrc:/resources/sound/cant-touch-this.mp3")));
    _actions.append(BotAction::createSoundPlay({"!неСмешно"}, QUrl("qrc:/resources/sound/crickets.mp3")));
    _actions.append(BotAction::createSoundPlay({"!doIt", "!justDoIt"}, QUrl("qrc:/resources/sound/do_it.mp3")));
    _actions.append(BotAction::createSoundPlay({"!неожиданно", "!неожидано"}, QUrl("qrc:/resources/sound/dramatic.mp3")));
    _actions.append(BotAction::createSoundPlay({"!missionComplited"}, QUrl("qrc:/resources/sound/gta-sa-done.mp3")));
    _actions.append(BotAction::createSoundPlay({"!хаха"}, QUrl("qrc:/resources/sound/ha-ha.mp3")));
    _actions.append(BotAction::createSoundPlay({"!hadouken"}, QUrl("qrc:/resources/sound/hadouken.mp3")));
    _actions.append(BotAction::createSoundPlay({"!monsterkill"}, QUrl("qrc:/resources/sound/monsterkill.mp3")));
    _actions.append(BotAction::createSoundPlay({"!nooo"}, QUrl("qrc:/resources/sound/nooo.mp3")));
    _actions.append(BotAction::createSoundPlay({"!omaewamou"}, QUrl("qrc:/resources/sound/omaewamou.mp3")));
    _actions.append(BotAction::createSoundPlay({"!omaewamou2"}, QUrl("qrc:/resources/sound/omaewamou2.mp3")));
    _actions.append(BotAction::createSoundPlay({"!грусть2"}, QUrl("qrc:/resources/sound/sadaffleck.mp3")));
    _actions.append(BotAction::createSoundPlay({"!toBeContinued"}, QUrl("qrc:/resources/sound/to_be_continued.mp3")));
    _actions.append(BotAction::createSoundPlay({"!tuturu"}, QUrl("qrc:/resources/sound/tuturu.mp3")));
    _actions.append(BotAction::createSoundPlay({"!остынь"}, QUrl("qrc:/resources/sound/wc_cool.mp3")));
    _actions.append(BotAction::createSoundPlay({"!неплохо"}, QUrl("qrc:/resources/sound/wc_good.mp3")));
    _actions.append(BotAction::createSoundPlay({"!полежу", "!пойдуПолежу"}, QUrl("qrc:/resources/sound/wc_lie_down.mp3")));
    _actions.append(BotAction::createSoundPlay({"!noTime"}, QUrl("qrc:/resources/sound/wc_notime.mp3")));
    _actions.append(BotAction::createSoundPlay({"!чемТыЗанимаешься"}, QUrl("qrc:/resources/sound/wc_what_are_you.mp3")));
    _actions.append(BotAction::createSoundPlay({"!чемТыЗанимаешься2"}, QUrl("qrc:/resources/sound/what-are-you-doing-in-my-swamp.mp3")));
    _actions.append(BotAction::createSoundPlay({"!xfiles", "!x-files"}, QUrl("qrc:/resources/sound/x-files.mp3")));
    _actions.append(BotAction::createSoundPlay({"!ГГ", "!GG"}, QUrl("qrc:/resources/sound/gg.mp3")));
    _actions.append(BotAction::createSoundPlay({"!кончено", "!ofCourse"}, QUrl("qrc:/resources/sound/of_course.mp3")));
    _actions.append(BotAction::createSoundPlay({"!фшха", "!неСмешно2"}, QUrl("qrc:/resources/sound/nuzhdiki.mp3")));
    _actions.append(BotAction::createSoundPlay({"!здоровоОтец", "!здаровоОтец", "!здороваОтец", "!здароваОтец"}, QUrl("qrc:/resources/sound/great_father.mp3")));
    _actions.append(BotAction::createSoundPlay({"!пиво", "!pivo", "!beer"}, QUrl("qrc:/resources/sound/pivo.mp3")));

    _actions.append(BotAction::createSoundPlay({"!факт", "!этоФакт"}, QUrl("qrc:/resources/sound/2nd_channel/2nd Channel-Это факт.mp3")));
    _actions.append(BotAction::createSoundPlay({"!гудлак", "!321GoodLuck"}, QUrl("qrc:/resources/sound/2nd_channel/2nd Channel-3-2-1 - Good Luck-kissvk.com.mp3")));
    _actions.append(BotAction::createSoundPlay({"!гудлак2"}, QUrl("qrc:/resources/sound/2nd_channel/2nd Channel-Gin Chin Good Luck.mp3")));
    _actions.append(BotAction::createSoundPlay({"!гудлак3"}, QUrl("qrc:/resources/sound/2nd_channel/2nd Channel-Ни пуха ни Good Luck-kissvk.com.mp3")));
    _actions.append(BotAction::createSoundPlay({"!неалкоголики"}, QUrl("qrc:/resources/sound/2nd_channel/2nd Channel-Анонимных неалкоголиков сборище-kissvk.com.mp3")));
    _actions.append(BotAction::createSoundPlay({"!вернутьВСемью"}, QUrl("qrc:/resources/sound/2nd_channel/2nd Channel-Вернуть в семью Артура-kissvk.com.mp3")));
    _actions.append(BotAction::createSoundPlay({"!нетривиальный"}, QUrl("qrc:/resources/sound/2nd_channel/2nd Channel-Вот таким нетривиальным способом-kissvk.com.mp3")));
    _actions.append(BotAction::createSoundPlay({"!доступЗапрещён", "!доступЗапрещен"}, QUrl("qrc:/resources/sound/2nd_channel/2nd Channel-Доступ запрещён - Всем БАН. До Свидания-kissvk.com.mp3")));
    _actions.append(BotAction::createSoundPlay({"!измерениеХ", "!измерениеX"}, QUrl("qrc:/resources/sound/2nd_channel/2nd Channel-Из измерения X возвращается-kissvk.com.mp3")));
    _actions.append(BotAction::createSoundPlay({"!тетрисВОкне"}, QUrl("qrc:/resources/sound/2nd_channel/2nd Channel-Кто-то там играет в Тетрис-kissvk.com.mp3")));
    _actions.append(BotAction::createSoundPlay({"!МаксимПодтверди"}, QUrl("qrc:/resources/sound/2nd_channel/2nd Channel-Максим подтверди - Подтверждаю-kissvk.com.mp3")));
    _actions.append(BotAction::createSoundPlay({"!чутьНеЗасосало"}, QUrl("qrc:/resources/sound/2nd_channel/2nd Channel-Меня чуть не засосало-kissvk.com.mp3")));
    _actions.append(BotAction::createSoundPlay({"!разрабатываемТНУ4"}, QUrl("qrc:/resources/sound/2nd_channel/2nd Channel-Мы оказывается разрабатываем TNU4-kissvk.com.mp3")));
    _actions.append(BotAction::createSoundPlay({"!удаляйтеИнтернет"}, QUrl("qrc:/resources/sound/2nd_channel/2nd Channel-Не удаляйте мне интернет на компьютере-kissvk.com.mp3")));
    _actions.append(BotAction::createSoundPlay({"!РафаДостают"}, QUrl("qrc:/resources/sound/2nd_channel/2nd Channel-Ну не надо меня доставать-kissvk.com.mp3")));
    _actions.append(BotAction::createSoundPlay({"!неСудьба"}, QUrl("qrc:/resources/sound/2nd_channel/2nd Channel-Ну не судьба-kissvk.com.mp3")));
    _actions.append(BotAction::createSoundPlay({"!неБаньте"}, QUrl("qrc:/resources/sound/2nd_channel/2nd Channel-Пожалуйста не баньте-kissvk.com.mp3")));
    _actions.append(BotAction::createSoundPlay({"!неХочуДумать"}, QUrl("qrc:/resources/sound/2nd_channel/2nd Channel-Пожалуйста. Я не хочу думать об этом-kissvk.com.mp3")));
    _actions.append(BotAction::createSoundPlay({"!турболёт", "!турболет"}, QUrl("qrc:/resources/sound/2nd_channel/2nd Channel-Полный турболёт-kissvk.com.mp3")));
    _actions.append(BotAction::createSoundPlay({"!понятноСТобой"}, QUrl("qrc:/resources/sound/2nd_channel/2nd Channel-Понятно всё с тобой. Ничем ты не поможешь мне-kissvk.com.mp3")));
    _actions.append(BotAction::createSoundPlay({"!рисоватьИзврат"}, QUrl("qrc:/resources/sound/2nd_channel/2nd Channel-Рисовать - это изврат-kissvk.com.mp3")));
    _actions.append(BotAction::createSoundPlay({"!электрод"}, QUrl("qrc:/resources/sound/2nd_channel/2nd Channel-Сейчас так ХОП-kissvk.com.mp3")));
    _actions.append(BotAction::createSoundPlay({"!скринь", "!скинь", "!скрин", "!скинь", "!скин"}, QUrl("qrc:/resources/sound/2nd_channel/2nd Channel-Скинь скринь скринъ-kissvk.com.mp3")));
    _actions.append(BotAction::createSoundPlay({"!потерялЖизнь"}, QUrl("qrc:/resources/sound/2nd_channel/2nd Channel-Теперь я потерял жизнь-kissvk.com.mp3")));
    _actions.append(BotAction::createSoundPlay({"!терминаторЗабеременел"}, QUrl("qrc:/resources/sound/2nd_channel/2nd Channel-Терминатор забеременел Робокопом-kissvk.com.mp3")));
    _actions.append(BotAction::createSoundPlay({"!чикса", "!бикса", "!чика"}, QUrl("qrc:/resources/sound/2nd_channel/2nd Channel-Что надо чикса-бикса-kissvk.com.mp3")));
    _actions.append(BotAction::createSoundPlay({"!РафНеХочет"}, QUrl("qrc:/resources/sound/2nd_channel/2nd Channel-Чёрт. Нет. Почему. Я не хочу-kissvk.com.mp3")));
    _actions.append(BotAction::createSoundPlay({"!вИнтернетеЗабаню"}, QUrl("qrc:/resources/sound/2nd_channel/2nd Channel-Я тебя в интернете забаню-kissvk.com.mp3")));
    _actions.append(BotAction::createSoundPlay({"!баньСебяСам"}, QUrl("qrc:/resources/sound/2nd_channel/2nd Channel- Бань себя сам-kissvk.com.mp3")));
    _actions.append(BotAction::createSoundPlay({"!терминаторДомогался"}, QUrl("qrc:/resources/sound/2nd_channel/2nd Channel-Меня Терминатор домогался-kissvk.com.mp3")));
    _actions.append(BotAction::createSoundPlay({"!быстроТыПоел"}, QUrl("qrc:/resources/sound/2nd_channel/2nd Channel-Да быстро ты поел-kissvk.com.mp3")));
    _actions.append(BotAction::createSoundPlay({"!скрытаяРеклама"}, QUrl("qrc:/resources/sound/2nd_channel/2nd Channel-Скрытая реклама.mp3")));

    _actions.append(BotAction::createSoundPlay({"!вотЭтоПоворот"}, QUrl("qrc:/resources/sound/vot_eto_povorot.mp3")));
    _actions.append(BotAction::createSoundPlay({"!накосячил", "!косяк"}, QUrl("qrc:/resources/sound/накосячил.mp3")));
    _actions.append(BotAction::createSoundPlay({"!чтоЭто", "!чувакЧтоЭто", "!wat", "!what"}, QUrl("qrc:/resources/sound/chto-eto.mp3")));

    _actions.append(BotAction::createSoundPlay({"!mkFatality", "!фаталити", "!мкФаталити", "!fatality"}, QUrl("qrc:/resources/sound/fatality.mp3")));
    _actions.append(BotAction::createSoundPlay({"!mkFinishhim", "!finishhim"}, QUrl("qrc:/resources/sound/finishhim.mp3")));
    _actions.append(BotAction::createSoundPlay({"!mkFight", "!fight", "!бой", "!вбой"}, QUrl("qrc:/resources/sound/mk-fight.mp3")));
    _actions.append(BotAction::createSoundPlay({"!mkToasty", "!toasty"}, QUrl("qrc:/resources/sound/mk-toasty.mp3")));
    _actions.append(BotAction::createSoundPlay({"!mkRound1", "!round1", "раунд1", "!мкРаунд1"}, QUrl("qrc:/resources/sound/mk-round1.mp3")));
    _actions.append(BotAction::createSoundPlay({"!mkRound2", "!round2", "раунд2", "!мкРаунд2"}, QUrl("qrc:/resources/sound/mk-round2.mp3")));
    _actions.append(BotAction::createSoundPlay({"!mkRound3", "!round3", "раунд3", "!мкРаунд3"}, QUrl("qrc:/resources/sound/mk-round3.mp3")));

    _actions.append(BotAction::createSoundPlay({"!дружкоНеБуду", "!дружко"}, QUrl("qrc:/resources/sound/drujko-ne-budu.mp3")));
    _actions.append(BotAction::createSoundPlay({"!неТочно", "!ноЭтоНеТочно", "!брб"}, QUrl("qrc:/resources/sound/ne-tochno.mp3")));
    _actions.append(BotAction::createSoundPlay({"!astalavista", "!hastaLaVista", "!асталависта"}, QUrl("qrc:/resources/sound/astalavista.mp3")));

    _actions.append(BotAction::createSoundPlay({"!initial1", "!initialD1", "!eurobeat1", "!евробит1", "!dejavu", "!deja-vu", "!дежавю", "!дежа-вю", "!дежаву", "!дежа-ву", "!дэжаву", "!дэжавю"}, QUrl("qrc:/resources/sound/deja-vu.mp3")));
    _actions.append(BotAction::createSoundPlay({"!initial2", "!initialD2", "!eurobeat2", "!евробит2", "!газ", "!гас", "!gas", "!gaz"}, QUrl("qrc:/resources/sound/gas.mp3")));
    _actions.append(BotAction::createSoundPlay({"!initial3", "!initialD3", "!eurobeat3", "!евробит3", "!90s", "!90е"}, QUrl("qrc:/resources/sound/eurobeat.mp3")));

    _actions.append(BotAction::createSoundPlay({"!saxophone", "!saxophon", "!саксофон"}, QUrl("qrc:/resources/sound/saxophone.mp3")));
    _actions.append(BotAction::createSoundPlay({"!fly", "!ICanFly"}, QUrl("qrc:/resources/sound/fly.mp3")));

    _actions.append(BotAction::createSoundPlay({"!suspense1", "!саспенс1", "!суспенс1", "!suspens1", "!saspense1", "!saspens1", "!psycho"}, QUrl("qrc:/resources/sound/psycho.mp3")));
    _actions.append(BotAction::createSoundPlay({"!suspense2", "!саспенс2", "!суспенс2", "!suspens2", "!saspense2", "!saspens2", "!барабаннаяДробь"}, QUrl("qrc:/resources/sound/saspens.mp3")));



    // !факт !этоФакт
    // !Гудлак !МыГГ !gman (it's time to choose) !shaokan (you will never win)

    //ToDo: добавить: !ура,  !боль,  !позабавимся,  !обмыть,  !твари, !отец, !Holyshit, !gameover, !lol, !пиво, !clear, !go, !negative, !победа, !rtrKakTak, !rtrGlavnoe, !дошик, !фиаско2, !zapab, !rtrZaruinil, !rtrZloyBoss, !rtrNadejda, !rtrЁКЛМН, !rtrNevezenie, !GoBen4, !jivi, !rtrPobedaBoss, !rtrBezuprechno, !rtrZlieBossi
}

int ChatBot::volume() const
{
    return _mediaPlayer->volume();
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

void ChatBot::setVolume(int volume)
{
    if (_mediaPlayer->volume() != volume)
    {
        _mediaPlayer->setVolume(volume);

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
            _mediaPlayer->setMedia(action._soundUrl);
            _mediaPlayer->play();
            qDebug() << "Playing" << action._soundUrl.toString();
        }
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

void ChatBot::onMediaPlayerError(QMediaPlayer::Error error)
{
    qDebug() << "QMediaPlayer error:" << error;
}

void ChatBot::onMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    qDebug() << "QMediaPlayer status:" << status;
}

