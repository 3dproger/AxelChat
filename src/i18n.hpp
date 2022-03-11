#pragma once

#include <QObject>
#include <QTranslator>
#include <QSettings>
#include <QQmlApplicationEngine>

class I18n : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString language READ language NOTIFY languageChanged)

public:
    explicit I18n(QSettings& settings, const QString& settingsGroup, QQmlApplicationEngine* qmlEngine = nullptr, QObject *parent = nullptr);
    Q_INVOKABLE bool setLanguage(const QString& shortTag);
    Q_INVOKABLE QString systemLanguage() const;
    ~I18n();
    static void declareQml();
    QString language() const;

    void setQmlApplicationEngine(QQmlApplicationEngine* qmlEngine);

signals:
    void languageChanged();

private:
    QTranslator* _appTranslator = nullptr;
    QSettings& settings;
    const QString SettingsGroupPath = "i18n";

    QQmlApplicationEngine* _qmlEngine = nullptr;

    const QString SETTINGNAME_LANGUAGETAG = "language_tag";
    QString _languageTag;
};

