#ifndef SETTINGSPAGECOMMANDS_H
#define SETTINGSPAGECOMMANDS_H

#include <QWidget>

namespace Ui {
class SettingsPageCommands;
}

class SettingsPageCommands : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsPageCommands(QWidget *parent = nullptr);
    ~SettingsPageCommands();

private:
    Ui::SettingsPageCommands *ui;
};

#endif // SETTINGSPAGECOMMANDS_H
