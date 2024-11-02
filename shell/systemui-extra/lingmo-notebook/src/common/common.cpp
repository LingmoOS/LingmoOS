#include <QString>
#include <QByteArray>

#include "common.h"


const char *qstringTochar(const QString &qstr)
{
    if (qstr.isEmpty())
        return nullptr;
    QByteArray ba = qstr.toLatin1();
    return ba.data();
}