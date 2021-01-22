#include "settingspagecommands.h"
#include "ui_settingspagecommands.h"

SettingsPageCommands::SettingsPageCommands(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingsPageCommands)
{
    ui->setupUi(this);
}

SettingsPageCommands::~SettingsPageCommands()
{
    delete ui;
}
