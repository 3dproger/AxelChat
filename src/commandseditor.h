#ifndef COMMANDSEDITOR_H
#define COMMANDSEDITOR_H

#include <QDialog>
#include <QQmlEngine>
#include <QTableWidgetItem>
#include "chatbot.hpp"
#include "commandsingleeditor.h"

namespace Ui {
class CommandsEditor;
}

class CommandsEditor : public QDialog
{
    Q_OBJECT

public:
    explicit CommandsEditor(ChatBot& chatBot, QWidget *parent = nullptr);
    ~CommandsEditor();

    static void declareQml()
    {
        qmlRegisterUncreatableType<CommandsEditor>("CommandsEditor", 1, 0, "CommandsEditor", "Type cannot be created in QML");
    }

private slots:
    void on_pushButtonCreate_clicked();

    void on_tableWidgetCommands_doubleClicked(const QModelIndex &index);

    void on_pushButtonEdit_clicked();

    void on_tableWidgetCommands_itemSelectionChanged();

    void updateCommands();

    void on_pushButtonDelete_clicked();

    void on_pushButtonExecute_clicked();

private:
    void editCurrentCommand();
    Ui::CommandsEditor *ui;
    ChatBot& chatBot;
    CommandSingleEditor* _singleEditor = nullptr;
};

#endif // COMMANDSEDITOR_H
