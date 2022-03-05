#ifndef CLIPBOARDQML_HPP
#define CLIPBOARDQML_HPP

#include <QObject>
#include <QQmlEngine>

class ClipboardQml : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
public:
    explicit ClipboardQml(QObject *parent = nullptr);
    static void declareQml()
    {
        qmlRegisterUncreatableType<ClipboardQml>("ClipboardQml.Clipboard", 1, 0, "Clipboard", "Type cannot be created in QML");
    }
    QString text() const;
    void setText(const QString& text) const;

signals:
    void textChanged();

private:

};

#endif // CLIPBOARDQML_HPP
