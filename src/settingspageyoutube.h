#ifndef SETTINGSPAGEYOUTUBE_H
#define SETTINGSPAGEYOUTUBE_H

#include <QWidget>

namespace Ui {
class SettingsPageYouTube;
}

class SettingsPageYouTube : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsPageYouTube(QWidget *parent = nullptr);
    ~SettingsPageYouTube();

private:
    Ui::SettingsPageYouTube *ui;
};

#endif // SETTINGSPAGEYOUTUBE_H
