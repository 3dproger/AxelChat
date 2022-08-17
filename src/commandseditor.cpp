#include "commandseditor.h"
#include "ui_commandseditor.h"
#include <QDebug>
#include <QMessageBox>

CommandsEditor::CommandsEditor(ChatBot& chatBot_, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CommandsEditor),
    chatBot(chatBot_)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window);
    ui->pushButtonEnableDisable->setVisible(false); // ToDo:
    ui->pushButtonExecute->setVisible(false); // ToDo:
    ui->tableWidgetCommands->setColumnCount(2);
    ui->tableWidgetCommands->setColumnWidth(1, 130);
    ui->tableWidgetCommands->setColumnWidth(1, 300);
    _singleEditor = new CommandSingleEditor(chatBot, this);
    connect(_singleEditor, &CommandSingleEditor::commandsChanged, this, &CommandsEditor::updateCommands);
    updateCommands();
    on_tableWidgetCommands_itemSelectionChanged();
}

CommandsEditor::~CommandsEditor()
{
    delete ui;
}

void CommandsEditor::on_pushButtonCreate_clicked()
{
    _singleEditor->setCommand(nullptr, -1);
    _singleEditor->show();
}

void CommandsEditor::updateCommands()
{
    ui->tableWidgetCommands->clear();
    ui->tableWidgetCommands->setColumnCount(2);
    ui->tableWidgetCommands->setRowCount(0);
    ui->tableWidgetCommands->setHorizontalHeaderItem(0, new QTableWidgetItem(tr("Keywords")));
    ui->tableWidgetCommands->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("Action")));

    const QList<BotAction*>& actions = chatBot.actions();

    for (int i = 0; i < actions.count(); ++i)
    {
        const BotAction* action = actions.at(i);

        ui->tableWidgetCommands->insertRow(i);

        QString keywordString;
        const QStringList& keywords = action->keywords();
        for (int kw = 0; kw < keywords.count(); ++kw)
        {
            const QString& keyword = keywords.at(kw);
            keywordString += keyword.trimmed();
            if (kw < keywords.count() - 1)
            {
                keywordString += ", ";
            }
        }
        ui->tableWidgetCommands->setItem(i, 0, new QTableWidgetItem(keywordString));

        QString actionString;

        switch (action->type()) {
        case BotAction::ActionType::SoundPlay:
            actionString = tr("Sound: %1").arg(action->soundUrl().toLocalFile());
            break;
        case BotAction::ActionType::Unknown:
            actionString = tr("Error");
            break;
        }

        ui->tableWidgetCommands->setItem(i, 1, new QTableWidgetItem(actionString));
    }

    on_tableWidgetCommands_itemSelectionChanged();
}

void CommandsEditor::editCurrentCommand()
{
    const int row = ui->tableWidgetCommands->currentRow();
    const QList<BotAction*>& actions = chatBot.actions();
    if (row < actions.count() && row != -1)
    {
        _singleEditor->setCommand(actions.at(row), row);
        _singleEditor->show();
    }
}

void CommandsEditor::on_tableWidgetCommands_doubleClicked(const QModelIndex &index)
{
    Q_UNUSED(index);
    editCurrentCommand();
}

void CommandsEditor::on_pushButtonEdit_clicked()
{
    editCurrentCommand();
}

void CommandsEditor::on_tableWidgetCommands_itemSelectionChanged()
{
    const int row = ui->tableWidgetCommands->currentRow();
    const QList<BotAction*>& actions = chatBot.actions();
    if (row < actions.count() && row != -1)
    {
        ui->pushButtonEdit->setEnabled(true);
        ui->pushButtonExecute->setEnabled(true);
        ui->pushButtonEnableDisable->setEnabled(true);
        ui->pushButtonDelete->setEnabled(true);
    }
    else
    {
        ui->pushButtonEdit->setEnabled(false);
        ui->pushButtonExecute->setEnabled(false);
        ui->pushButtonEnableDisable->setEnabled(false);
        ui->pushButtonDelete->setEnabled(false);
    }
}

void CommandsEditor::on_pushButtonDelete_clicked()
{
    if (QMessageBox::StandardButton::Yes != QMessageBox::question(this,
                         tr("Deletion"),
                         tr("Are you sure you want to delete the command?"),
                         QMessageBox::StandardButton::No | QMessageBox::StandardButton::Yes))
    {
        return;
    }

    const int row = ui->tableWidgetCommands->currentRow();
    chatBot.deleteAction(row);
    updateCommands();
}

void CommandsEditor::on_pushButtonExecute_clicked()
{
    const int row = ui->tableWidgetCommands->currentRow();
    chatBot.executeAction(row);
}
