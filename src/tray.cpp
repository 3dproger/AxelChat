#include "tray.h"
#include <QTranslator>
#include <QApplication>

Tray::Tray(QObject *parent) : QObject(parent)
{
    tray = new QSystemTrayIcon(QIcon(":/resources/images/axelchat-16x16.png"), this);

    QMenu* menu = new QMenu();
    QAction* action;

    action = new QAction(QIcon(":/resources/images/applications-system.png"), QTranslator::tr("Settings"), menu);
    connect(action, &QAction::triggered, this, [&](){
        emit triggered("settings");
    });
    menu->addAction(action);

    _ignoreMouseAction = new QAction(QIcon(":/resources/images/input-mouse-disable.png"), QTranslator::tr("Ignore Mouse"), menu);
    connect(_ignoreMouseAction, &QAction::triggered, this, [&](){
        emit triggered("input_transparent_toggle");
    });
    menu->addAction(_ignoreMouseAction);

    action = new QAction(QIcon(""), QTranslator::tr("Clear Messages"), menu);
    connect(action, &QAction::triggered, this, [&](){
        emit triggered("clear_messages");
    });
    menu->addAction(action);

    action = new QAction(QIcon(":/resources/images/emblem-unreadable.png"), QTranslator::tr("Close"), menu);
    connect(action, &QAction::triggered, this, [&](){
        emit triggered("close_application");
    });
    menu->addAction(action);

    tray->setContextMenu(menu);

    tray->show();
}

void Tray::setIgnoredMouse(bool ignored)
{
    _ignoreMouse = ignored;

    if (_ignoreMouseAction)
    {
        if (ignored)
        {
            _ignoreMouseAction->setText(QTranslator::tr("Don't Ignore Mouse"));
            _ignoreMouseAction->setIcon(QIcon(":/resources/images/input-mouse.png"));
        }
        else
        {
            _ignoreMouseAction->setText(QTranslator::tr("Ignore Mouse"));
            _ignoreMouseAction->setIcon(QIcon(":/resources/images/input-mouse-disable.png"));
        }
    }
}
