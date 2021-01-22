#ifndef SETTINGSPAGECOMMON_H
#define SETTINGSPAGECOMMON_H

#include <QWidget>

namespace Ui {
class SettingsPageCommon;
}

class SettingsPageCommon : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsPageCommon(QWidget *parent = nullptr);
    ~SettingsPageCommon();

private:
    Ui::SettingsPageCommon *ui;
};

#endif // SETTINGSPAGECOMMON_H
