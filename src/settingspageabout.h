#ifndef SETTINGSPAGEABOUT_H
#define SETTINGSPAGEABOUT_H

#include <QWidget>

namespace Ui {
class SettingsPageAbout;
}

class SettingsPageAbout : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsPageAbout(QWidget *parent = nullptr);
    ~SettingsPageAbout();

private:
    Ui::SettingsPageAbout *ui;
};

#endif // SETTINGSPAGEABOUT_H
