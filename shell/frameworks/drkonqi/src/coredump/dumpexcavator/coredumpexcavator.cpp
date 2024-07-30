// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "coredumpexcavator.h"

#include <QDebug>
#include <QFile>
#include <QProcess>

using namespace Qt::StringLiterals;

void CoredumpExcavator::excavateFromTo(const QString &coreFile, const std::shared_ptr<QFile> &coreFileTarget)
{
    auto proc = new QProcess(this);
    proc->setProcessChannelMode(QProcess::ForwardedErrorChannel); // stdout goes to file
    proc->setProgram("coredumpctl"_L1);
    proc->setArguments({"dump"_L1, "COREDUMP_FILENAME=%1"_L1.arg(coreFile)});
    qDebug() << "excavating" << proc->arguments();
    connect(proc, &QProcess::readyReadStandardOutput, coreFileTarget.get(), [proc, coreFileTarget] {
        auto data = proc->readAllStandardOutput();
        while (data.size() != 0) {
            auto written = coreFileTarget->write(data);
            data = data.mid(written);
        }
    });
    connect(proc, &QProcess::finished, this, [this, proc, coreFileTarget](int exitCode, QProcess::ExitStatus exitStatus) mutable {
        qDebug() << "Core dump excavation complete" << exitCode << exitStatus;
        proc->deleteLater();
        coreFileTarget->flush();
        Q_ASSERT(proc->readAllStandardOutput().size() == 0);
        Q_EMIT excavated(exitCode);
    });
    proc->start();
}
