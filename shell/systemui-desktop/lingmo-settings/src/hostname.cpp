#include <QProcess>
#include <QDebug>

#include "hostname.h"

void changeHostName(const QString &newHostName) {
    QProcess process;
    QStringList arguments;
    arguments << "general" << "hostname" << newHostName;
    process.start("nmcli", arguments);
}
