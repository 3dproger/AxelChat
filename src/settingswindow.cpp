#include "settingswindow.h"
#include "ui_settingswindow.h"
#include "settingspagecommon.h"
#include "settingspageyoutube.h"
#include "settingspagecommands.h"
#include "settingspageoutputtofile.h"
#include "settingspageabout.h"

SettingsWindow::SettingsWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SettingsWindow)
{
    ui->setupUi(this);

    ui->stackedWidgetCategories->addWidget(new SettingsPageYouTube(this));
    ui->stackedWidgetCategories->addWidget(new SettingsPageCommon(this));
    ui->stackedWidgetCategories->addWidget(new SettingsPageCommands(this));
    ui->stackedWidgetCategories->addWidget(new SettingsPageOutputToFile(this));
    ui->stackedWidgetCategories->addWidget(new SettingsPageAbout(this));

    ui->splitter->setSizes({1, 8});
}

SettingsWindow::~SettingsWindow()
{
    delete ui;
}

void SettingsWindow::on_listWidgetCategories_currentRowChanged(int currentRow)
{
    if (currentRow >= 0 && currentRow < ui->listWidgetCategories->count())
    {
        ui->stackedWidgetCategories->setCurrentIndex(currentRow);
    }
    else
    {
        qDebug() << "invalid row";
    }
}
