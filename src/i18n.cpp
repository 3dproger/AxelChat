#include "i18n.hpp"
#include <QApplication>
#include <QLocale>
#include <QQmlEngine>

I18n::I18n(QSettings& settings_, const QString& settingsGroup, QQmlApplicationEngine* qmlEngine, QObject *parent) :
    QObject(parent),
    settings(settings_),
    SettingsGroupPath(settingsGroup),
    _qmlEngine(qmlEngine)
{
    setLanguage(settings.value(SettingsGroupPath + "/" + SETTINGNAME_LANGUAGETAG, systemLanguage()).toString());
}

bool I18n::setLanguage(const QString &shortTag)
{
    QString normTag = shortTag.toLower().trimmed();
    if (normTag != _languageTag)
    {
        if (_appTranslator)
        {
            qApp->removeTranslator(_appTranslator);
            delete _appTranslator;
            _appTranslator = nullptr;
        }

        _languageTag = "C";

        if (normTag == "c" || normTag == "en") {
            settings.setValue(SettingsGroupPath + "/" + SETTINGNAME_LANGUAGETAG, normTag);

            if (_qmlEngine)
            {
                _qmlEngine->retranslate();
            }
            return true;
        }

        _appTranslator = new QTranslator(this);

        const QString fileName = ":/i18n/Translation_" + QLocale(normTag).name();
        if (!_appTranslator->load(fileName))
        {
            qDebug(QString("Failed to load translation \"%1\" with file \"%2\"")
                   .arg(shortTag).arg(fileName).toUtf8());
            delete _appTranslator;
            _appTranslator = nullptr;
            QLocale::setDefault(QLocale("C"));
            if (_qmlEngine)
            {
                _qmlEngine->retranslate();
            }
            emit languageChanged();
            return false;
        }

        if (!qApp->installTranslator(_appTranslator))
        {
            qDebug(QString("Failed to install translator for \"%1\" with file \"%2\"")
                   .arg(shortTag).arg(fileName).toUtf8());
            delete _appTranslator;
            _appTranslator = nullptr;
            QLocale::setDefault(QLocale("C"));
            if (_qmlEngine)
            {
                _qmlEngine->retranslate();
            }
            emit languageChanged();
            return false;
        }

        settings.setValue(SettingsGroupPath + "/" + SETTINGNAME_LANGUAGETAG, normTag);

        _languageTag = normTag;
        QLocale::setDefault(normTag);
        if (_qmlEngine)
        {
            _qmlEngine->retranslate();
        }
        emit languageChanged();
        return true;
    }

    qDebug(QString("Failed set language \"%1\"")
           .arg(shortTag).toUtf8());
    QLocale::setDefault(QLocale("C"));
    if (_qmlEngine)
    {
        _qmlEngine->retranslate();
    }
    emit languageChanged();
    return false;
}

QString I18n::systemLanguage() const
{
    return QLocale::system().bcp47Name();
}

I18n::~I18n()
{

}

void I18n::declareQml()
{
    qmlRegisterUncreatableType<I18n> ("AxelChat.I18n",
                                      1, 0, "I18n", "Type cannot be created in QML");
}

QString I18n::language() const
{
    return _languageTag;
}

void I18n::setQmlApplicationEngine(QQmlApplicationEngine *qmlEngine)
{
    _qmlEngine = qmlEngine;
    if (_qmlEngine)
    {
        _qmlEngine->retranslate();
    }
}
