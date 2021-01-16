#include "commandseditor.h"
#include "ui_commandseditor.h"

CommandsEditor::CommandsEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CommandsEditor)
{
    ui->setupUi(this);
}

CommandsEditor::~CommandsEditor()
{
    delete ui;
}
