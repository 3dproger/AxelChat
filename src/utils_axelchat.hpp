#ifndef UTILS_AXELCHAT_HPP
#define UTILS_AXELCHAT_HPP

#include <QByteArray>
#include <QString>
#include <QVector>

QByteArray convertANSIWithUtf8Numbers(const QString& string)
{
    const QVector<uint>& ucs4str = string.toUcs4();
    QByteArray ba;
    ba.reserve(ucs4str.count() * 4);

    for (const uint& c : ucs4str)
    {
        if (c >= 32u && c <= 126u && c!= 35u && c!= 60u && c!= 62u)
        {
            ba += (char)c;
        }
        else
        {
            ba += "<" + QByteArray::number(c) + ">";
        }
    }

    return ba;
}

#endif // UTILS_AXELCHAT_HPP
