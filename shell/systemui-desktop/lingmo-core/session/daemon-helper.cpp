/**
 * @name daemon-helper.cpp
 * @author Elysia <c.elysia@foxmail.com>
 **/
#include "daemon-helper.h"

#include <QCoreApplication>
#include <QProcess>
#include <QTimer>
#include <QDebug>
#include <QPair>
#include <QString>
#include <QPointer>

namespace LINGMO_SESSION {
Daemon::Daemon(const QList<QPair<QString, QStringList>> &processList, bool _enableAutoStart, QObject *parent)
    : QObject(parent), m_processList(processList), m_enableAutoRestart(_enableAutoStart) {
  for (const auto &processInfo : m_processList) {
    startProcess(processInfo);
  }
}

void Daemon::onProcessError(QProcess::ProcessError error) {
  const QPointer process = qobject_cast<QProcess *>(sender());

  if (!process)
    return;

  QString program = process->program();
  qDebug() << "Process error:" << program << "Error:" << error;

  for (const auto &processInfo : m_processList) {
    if (processInfo.first == program) {
      qDebug() << "Restarting process due to error:" << program;
      QTimer::singleShot(1, this, [this, processInfo]() {
        startProcess(processInfo);
      }); // Restart after 1 second
      return;
    }
  }
}

void Daemon::startProcess(const QPair<QString, QStringList> &processInfo) {
  const QPointer process = new QProcess(this);

  if (this->m_enableAutoRestart)
    connect(process, &QProcess::errorOccurred,
            this, &Daemon::onProcessError);

  process->start(processInfo.first, processInfo.second);
  if (process->waitForStarted()) {
    qDebug() << "Process started:" << processInfo.first << "PID:" << process->processId();
  } else {
    qDebug() << "Failed to start process:" << processInfo.first << process->errorString();
  }
}
} // namespace LINGMO_SESSION
