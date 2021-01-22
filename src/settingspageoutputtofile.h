#ifndef SETTINGSPAGEOUTPUTTOFILE_H
#define SETTINGSPAGEOUTPUTTOFILE_H

#include <QWidget>

namespace Ui {
class SettingsPageOutputToFile;
}

class SettingsPageOutputToFile : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsPageOutputToFile(QWidget *parent = nullptr);
    ~SettingsPageOutputToFile();

private:
    Ui::SettingsPageOutputToFile *ui;
};

#endif // SETTINGSPAGEOUTPUTTOFILE_H
