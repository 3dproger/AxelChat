#ifndef COMMANDSEDITOR_H
#define COMMANDSEDITOR_H

#include <QDialog>
#include <QQmlEngine>

namespace Ui {
class CommandsEditor;
}

class CommandsEditor : public QDialog
{
    Q_OBJECT

public:
    explicit CommandsEditor(QWidget *parent = nullptr);
    ~CommandsEditor();

    static void declareQml()
    {
        qmlRegisterUncreatableType<CommandsEditor>("CommandsEditor", 1, 0, "CommandsEditor", "Type cannot be created in QML");
    }

private:
    Ui::CommandsEditor *ui;
};

#endif // COMMANDSEDITOR_H
