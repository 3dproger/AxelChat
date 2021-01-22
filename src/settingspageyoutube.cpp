#include "settingspageyoutube.h"
#include "ui_settingspageyoutube.h"

SettingsPageYouTube::SettingsPageYouTube(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingsPageYouTube)
{
    ui->setupUi(this);
}

SettingsPageYouTube::~SettingsPageYouTube()
{
    delete ui;
}
