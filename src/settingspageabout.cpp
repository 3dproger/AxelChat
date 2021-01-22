#include "settingspageabout.h"
#include "ui_settingspageabout.h"

SettingsPageAbout::SettingsPageAbout(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingsPageAbout)
{
    ui->setupUi(this);
}

SettingsPageAbout::~SettingsPageAbout()
{
    delete ui;
}
