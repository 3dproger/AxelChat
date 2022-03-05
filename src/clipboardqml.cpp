#include "clipboardqml.hpp"
#include <QGuiApplication>
#include <QClipboard>

ClipboardQml::ClipboardQml(QObject *parent) : QObject(parent)
{
    connect(QGuiApplication::clipboard(), &QClipboard::dataChanged, this, &ClipboardQml::textChanged);
}

QString ClipboardQml::text() const
{
    return QGuiApplication::clipboard()->text();
}

void ClipboardQml::setText(const QString &text) const
{
    QGuiApplication::clipboard()->setText(text);
}
