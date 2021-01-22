#include "settingspageoutputtofile.h"
#include "ui_settingspageoutputtofile.h"

SettingsPageOutputToFile::SettingsPageOutputToFile(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingsPageOutputToFile)
{
    ui->setupUi(this);
}

SettingsPageOutputToFile::~SettingsPageOutputToFile()
{
    delete ui;
}
