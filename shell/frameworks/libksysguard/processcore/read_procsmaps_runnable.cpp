/*
    SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "read_procsmaps_runnable.h"

#include <QFile>

using namespace KSysGuard;

ReadProcSmapsRunnable::ReadProcSmapsRunnable(const QString &dir)
    : QObject()
    , m_dir(dir)
{
    setAutoDelete(true);
}

void ReadProcSmapsRunnable::run()
{
    QFile file{m_dir + QStringLiteral("smaps_rollup")};
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    qulonglong pss = 0LL;
    auto buffer = QByteArray{1024, '\0'};
    while (file.readLine(buffer.data(), buffer.size()) > 0) {
        if (buffer.startsWith("Pss:")) {
            pss += std::stoll(buffer.mid(sizeof("Pss:")).toStdString());
            break;
        }
    }

    file.close();

    Q_EMIT finished(pss);
}
