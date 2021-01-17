#include "commandsingleeditor.h"
#include "ui_commandsingleeditor.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include "botaction.hpp"

CommandSingleEditor::CommandSingleEditor(ChatBot* chatBot, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CommandSingleEditor),
    _chatBot(chatBot)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    ui->splitter->setSizes({100, 200});
    setModal(true);
    setCommand(nullptr, -1);
}

void CommandSingleEditor::setCommand(BotAction *action, int pos)
{
    _createNew = true;
    _editingActionNum = -1;
    setWindowTitle(tr("Command Creating"));
    ui->plainTextEditKeywords->clear();
    ui->comboBoxActionType->setCurrentIndex(0);
    ui->checkBoxCaseSensitivity->setChecked(false);
    ui->lineEditSoundFile->setText("");

    if (!action)
    {
        return;
    }

    _createNew = false;
    _editingActionNum = pos;
    setWindowTitle(tr("Command Editing"));

    QString keywordsString;
    const QStringList& keywords = action->keywords();
    for (int i = 0; i < keywords.count(); ++i)
    {
        const QString& keyword = keywords.at(i);
        keywordsString.append(keyword.trimmed());
        if (i < keywords.count() - 1)
        {
            keywordsString += "\n";
        }
    }
    ui->plainTextEditKeywords->setPlainText(keywordsString);

    ui->checkBoxCaseSensitivity->setChecked(action->caseSensitive());

    switch (action->type()) {
    case BotAction::ActionType::SoundPlay:
        ui->lineEditSoundFile->setText(action->soundUrl().toString());
        ui->comboBoxActionType->setCurrentIndex(0);
        break;
    }
}

CommandSingleEditor::~CommandSingleEditor()
{
    delete ui;
}

void CommandSingleEditor::on_pushButtonCancel_clicked()
{
    close();
}

void CommandSingleEditor::on_pushButtonDone_clicked()
{
    QString rawText = ui->plainTextEditKeywords->toPlainText();
    rawText.remove('\r');
    rawText.remove('\t');
    rawText.remove('\v');
    QStringList keywords;
    QString rawKeyword;
    for (int i = 0; i < rawText.count(); ++i)
    {
        const QChar& c = rawText.at(i);
        if (c == '\n')
        {
            rawKeyword = rawKeyword.trimmed();
            if (!rawKeyword.isEmpty())
            {
                keywords.append(rawKeyword);
            }
            rawKeyword.clear();
            continue;
        }

        rawKeyword += c;
    }

    rawKeyword = rawKeyword.trimmed();
    if (!rawKeyword.isEmpty())
    {
        keywords.append(rawKeyword);
    }

    if (keywords.isEmpty())
    {
        QMessageBox::critical(this, tr("Error"), tr("No keywords specified"));
        return;
    }

    const QString fileName = ui->lineEditSoundFile->text();
    if (fileName.isEmpty())
    {
        QMessageBox::critical(this, tr("Error"), tr("Sound file not specified"));
        return;
    }

    QFile file(fileName);
    if (!file.exists())
    {
        if (QMessageBox::StandardButton::Yes != QMessageBox::warning(this,
                             tr("Warning"),
                             tr("File \"%1\" not found. Save command anyway?").arg(fileName),
                             QMessageBox::StandardButton::No | QMessageBox::StandardButton::Yes))
        {
            return;
        }
    }

    if (_createNew)
    {
        if (_chatBot)
        {
            BotAction *newAction = BotAction::createSoundPlay(
                        keywords,
                        QUrl(fileName),
                        ui->checkBoxCaseSensitivity->isChecked());

            _chatBot->addAction(newAction);
        }
        else
        {
            qDebug() << "!_chatBot";
        }
    }
    else
    {
        if (_chatBot)
        {
            BotAction *newAction = BotAction::createSoundPlay(
                        keywords,
                        QUrl(fileName),
                        ui->checkBoxCaseSensitivity->isChecked());

            _chatBot->rewriteAction(_editingActionNum, newAction);
        }
        else
        {
            qDebug() << "!_chatBot";
        }
    }

    emit commandsChanged();
    close();
}

void CommandSingleEditor::on_comboBoxActionType_currentIndexChanged(int index)
{
    ui->stackedWidgetActionEdit->setCurrentIndex(index);
}

void CommandSingleEditor::on_toolButtonFindSoundFile_clicked()
{
    const QString fileName = QFileDialog::getOpenFileName(
                this,
                tr("Sound File Openning..."),
                "",
                tr("WAV File") + " (*.wav);;" + tr("All Files") + " (*)");
    if (fileName.isEmpty())
    {
        return;
    }

    ui->lineEditSoundFile->setText(fileName);
}

void CommandSingleEditor::on_toolButtonSoundPlay_clicked()
{
    const QString fileName = ui->lineEditSoundFile->text().trimmed();

    if (fileName.isEmpty())
    {
        QMessageBox::critical(this,
                             tr("Error"),
                             tr("Sound file not specified"));
        return;
    }

    QFile file(fileName);
    if (!file.exists())
    {
        QMessageBox::warning(this,
                             tr("Warning"),
                             tr("File \"%1\" not found").arg(fileName));
        return;
    }

    QSound::play(fileName);
}
