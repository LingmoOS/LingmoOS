/*
 * SPDX-FileCopyrightText: 2019 Mikhail Zolotukhin <zomial@protonmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include <QProcessEnvironment>
#include <QStandardPaths>

#include "config.h"
#include "themepreviewer.h"

const QString ThemePreviewer::gtk3PreviewerExecutablePath = QStandardPaths::findExecutable(QStringLiteral("gtk3_preview"), {CMAKE_INSTALL_FULL_LIBEXECDIR});

ThemePreviewer::ThemePreviewer(QObject *parent)
    : QObject(parent)
    , gtk3PreviewerProccess()
{
}

void ThemePreviewer::showGtk3App(const QString &themeName)
{
    if (gtk3PreviewerProccess.state() == QProcess::ProcessState::NotRunning) {
        QProcessEnvironment gtk3PreviewEnvironment = QProcessEnvironment::systemEnvironment();
        gtk3PreviewEnvironment.insert(QStringLiteral("GTK_THEME"), themeName);
        gtk3PreviewerProccess.setProcessEnvironment(gtk3PreviewEnvironment);

        gtk3PreviewerProccess.start(gtk3PreviewerExecutablePath, QStringList());
    } else {
        gtk3PreviewerProccess.close();
    }
}

#include "moc_themepreviewer.cpp"
