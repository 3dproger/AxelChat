#include "settingspagecommon.h"
#include "ui_settingspagecommon.h"

SettingsPageCommon::SettingsPageCommon(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingsPageCommon)
{
    ui->setupUi(this);
}

SettingsPageCommon::~SettingsPageCommon()
{
    delete ui;
}
