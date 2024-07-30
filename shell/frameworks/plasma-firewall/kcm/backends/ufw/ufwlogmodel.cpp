// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2020 Tomaz Canabrava <tcanabrava@kde.org>

#include "ufwlogmodel.h"
#include <QDebug>

UfwLogModel::UfwLogModel(QObject *parent)
    : LogListModel(parent)
{
}

// Regexp failed me, parsing it is.
std::map<QString, QString> parseString(const QString &line)
{
    // We can find a line we are not interested.:
    // "-- Journal begins at Sun 2020-09-20 11:37:15 BST, ends at Wed 2020-12-09 18:45:16 GMT. --"
    if (line.startsWith(QLatin1String("-- Journal begins at "))) {
        return {};
    }

    // indices
    // 0    1   2        3          4       5    6    7
    // Dec 06 17:42:45 tomatoland kernel: [UFW BLOCK] IN=wlan0 OUT= MAC= SRC=192.168.50.181
    // DST=224.0.0.252 LEN=56 TOS=0x00 PREC=0x00 TTL=255 ID=52151 PROTO=UDP SPT=5355 DPT=5355 LEN=36
    //
    // We are interested in the dates, (0, 1, 2), and then starting on 7.
    std::map<QString, QString> results;
    QStringList splited = line.split(' ');
    if (splited.size() < 7) {
        return {};
    }

    results[QStringLiteral("date")] = splited[0] + " " + splited[1];
    results[QStringLiteral("time")] = splited[3];

    // We can drop now everything up to 7.
    splited.erase(std::begin(splited), std::begin(splited) + 7);
    for (const QString &element : std::as_const(splited)) {
        for (const QString &key :
             {QStringLiteral("IN"), QStringLiteral("SRC"), QStringLiteral("DST"), QStringLiteral("PROTO"), QStringLiteral("STP"), QStringLiteral("DPT")}) {
            if (element.startsWith(key)) {
                results[key] = element.mid(element.indexOf('=') + 1);
            }
        }
    }

    return results;
}

void UfwLogModel::addRawLogs(const QStringList &rawLogsList)
{
    QList<LogData> newLogs;
    newLogs.reserve(rawLogsList.count());
    for (const QString &log : rawLogsList) {
        auto map = parseString(log);

        LogData logDetails{.sourceAddress = map["SRC"],
                           .sourcePort = map["SPT"],
                           .destinationAddress = map["DST"],
                           .destinationPort = map["DPT"],
                           .protocol = map["PROTO"],
                           .interface = map["IN"],
                           .action = "",
                           .time = map["time"],
                           .date = map["date"]};
        newLogs.append(logDetails);
    }
    qDebug() << "Number of logs" << newLogs.count();
    appendLogData(newLogs);
}
