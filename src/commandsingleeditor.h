#ifndef COMMANDSINGLEEDITOR_H
#define COMMANDSINGLEEDITOR_H

#include <QDialog>
#include "chatbot.hpp"
#include <QSound>

namespace Ui {
class CommandSingleEditor;
}

class CommandSingleEditor : public QDialog
{
    Q_OBJECT

public:
    explicit CommandSingleEditor(ChatBot& chatBot, QWidget *parent = nullptr);
    void setCommand(BotAction* action, int pos);
    ~CommandSingleEditor();

signals:
    void commandsChanged();

private slots:
    void on_pushButtonCancel_clicked();

    void on_pushButtonDone_clicked();

    void on_comboBoxActionType_currentIndexChanged(int index);

    void on_toolButtonFindSoundFile_clicked();

    void on_toolButtonSoundPlay_clicked();

    void on_checkBoxChangeInactivePeriod_stateChanged(int arg1);

private:
    Ui::CommandSingleEditor *ui;
    ChatBot& chatBot;
    bool _createNew = true;
    int _editingActionNum = -1;
};

#endif // COMMANDSINGLEEDITOR_H
