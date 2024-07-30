/*******************************************************************
 * debugpackageinstaller.cpp
 * SPDX-FileCopyrightText: 2009 Dario Andres Rodriguez <andresbajotierra@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 ******************************************************************/

#include <config-drkonqi.h>

#include "debugpackageinstaller.h"

#include <KLocalizedString>
#include <KProcess>
#include <QProgressDialog>
#include <QStandardPaths>

#include "crashedapplication.h"
#include "drkonqi.h"

DebugPackageInstaller::DebugPackageInstaller(QObject *parent)
    : QObject(parent)
{
    m_executablePath = QStandardPaths::findExecutable(QString::fromLatin1(DEBUG_PACKAGE_INSTALLER_NAME)); // defined from CMakeLists.txt
}

bool DebugPackageInstaller::canInstallDebugPackages() const
{
    return !m_executablePath.isEmpty();
}

void DebugPackageInstaller::setMissingLibraries(const QStringList &libraries)
{
    m_missingLibraries = libraries;
}

void DebugPackageInstaller::installDebugPackages()
{
    Q_ASSERT(canInstallDebugPackages());

    if (!m_installerProcess) {
        // Run process
        m_installerProcess = new KProcess(this);
        connect(m_installerProcess, QOverload<int, QProcess::ExitStatus>::of(&KProcess::finished), this, &DebugPackageInstaller::processFinished);

        *m_installerProcess << m_executablePath << DrKonqi::crashedApplication()->executable().absoluteFilePath() << m_missingLibraries;
        m_installerProcess->start();

        // Show dialog
        m_progressDialog = new QProgressDialog(i18nc("@info:progress",
                                                     "Requesting installation of missing "
                                                     "debug symbols packages..."),
                                               i18n("Cancel"),
                                               0,
                                               0,
                                               qobject_cast<QWidget *>(parent()));
        connect(m_progressDialog, &QProgressDialog::canceled, this, &DebugPackageInstaller::progressDialogCanceled);
        m_progressDialog->setWindowTitle(i18nc("@title:window", "Missing debug symbols"));
        m_progressDialog->show();
    }
}

void DebugPackageInstaller::progressDialogCanceled()
{
    m_progressDialog->deleteLater();
    m_progressDialog = nullptr;

    if (m_installerProcess) {
        if (m_installerProcess->state() == QProcess::Running) {
            disconnect(m_installerProcess, QOverload<int, QProcess::ExitStatus>::of(&KProcess::finished), this, &DebugPackageInstaller::processFinished);
            m_installerProcess->kill();
            disconnect(m_installerProcess, QOverload<int, QProcess::ExitStatus>::of(&KProcess::finished), m_installerProcess, &KProcess::deleteLater);
        }
        m_installerProcess = nullptr;
    }

    Q_EMIT canceled();
}

void DebugPackageInstaller::processFinished(int exitCode, QProcess::ExitStatus)
{
    switch (exitCode) {
    case ResultInstalled: {
        Q_EMIT packagesInstalled();
        break;
    }
    case ResultSymbolsNotFound: {
        Q_EMIT error(i18nc("@info", "Could not find debug symbol packages for this application."));
        break;
    }
    case ResultCanceled: {
        Q_EMIT canceled();
        break;
    }
    case ResultError:
    default: {
        Q_EMIT error(i18nc("@info",
                           "An error was encountered during the installation "
                           "of the debug symbol packages."));
        break;
    }
    }

    m_progressDialog->reject();

    delete m_progressDialog;
    m_progressDialog = nullptr;

    delete m_installerProcess;
    m_installerProcess = nullptr;
}

#include "moc_debugpackageinstaller.cpp"
