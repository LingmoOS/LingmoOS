/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2014 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kmimetypeeditor.h"
#include "kmessagedialog.h"

#include <QObject>
#include <QProcess>
#include <QStandardPaths>

static const char s_keditfiletypeExecutable[] = "keditfiletype";

void KMimeTypeEditor::editMimeType(const QString &mimeType, QWidget *widget)
{
    QStringList args;
#ifndef Q_OS_WIN
    args << QStringLiteral("--parent") << QString::number(widget->window()->winId());
#endif
    args << mimeType;

    const QString exec = QStandardPaths::findExecutable(QLatin1String(s_keditfiletypeExecutable));
    if (exec.isEmpty()) {
        auto *dlg = new KMessageDialog(KMessageDialog::Error, QObject::tr("Could not find the \"keditfiletype\" executable in PATH."), widget);
        dlg->setAttribute(Qt::WA_DeleteOnClose);
        dlg->setModal(true);
        dlg->show();
        return;
    }

    const bool result = QProcess::startDetached(exec, args);
    if (!result) {
        auto *dlg =
            new KMessageDialog(KMessageDialog::Error, QObject::tr("Could not start the \"keditfiletype\" executable, please check your installation."), widget);
        dlg->setAttribute(Qt::WA_DeleteOnClose);
        dlg->setModal(true);
        dlg->show();
    }
}
