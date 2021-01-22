#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QMainWindow>
#include <QQmlEngine>

namespace Ui {
class SettingsWindow;
}

class SettingsWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SettingsWindow(QWidget *parent = nullptr);
    ~SettingsWindow();

    static void declareQml()
    {
        qmlRegisterUncreatableType<SettingsWindow>("SettingsWindow", 1, 0, "SettingsWindow", "Type cannot be created in QML");
    }

private slots:
    void on_listWidgetCategories_currentRowChanged(int currentRow);

private:
    Ui::SettingsWindow *ui;
};

#endif // SETTINGSWINDOW_H
