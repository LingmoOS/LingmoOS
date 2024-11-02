#ifndef COMMON_H
#define COMMON_H

#include <QString>
#include <QStringList>
#include <QMap>
#include <map>

#define QTC(x) (qstringTochar(x))

const char *qstringTochar(const QString &qstr);

#endif // COMMON_H