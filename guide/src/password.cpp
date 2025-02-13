#include "password.h"

#include <QRandomGenerator>
#include <crypt.h>
#include <unistd.h>

Password::Password(QObject *parent)
    : QObject(parent)
{
}

QString Password::cryptPassword(const QString &password) const
{
    QByteArray alpha = "0123456789ABCDEFGHIJKLMNOPQRSTUVXYZ"
                       "abcdefghijklmnopqrstuvxyz./";
    QByteArray salt("$6$");
    int len = alpha.count();
    for (int i = 0; i < 16; i++)
        salt.append(alpha.at(QRandomGenerator::global()->bounded(len)));

    return QString::fromLatin1(::crypt(password.toUtf8().constData(), salt.constData()));
}
