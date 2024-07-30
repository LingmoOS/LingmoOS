/*  This file is part of the KI18N Framework
    SPDX-FileCopyrightText: 2014 Kevin Krammer <krammer@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testhelpers.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QTest>

static const QString targetFileName = QStringLiteral("ktranscript.ini");

bool deployTestConfig()
{
    QStandardPaths::setTestModeEnabled(true);

    QDir configDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    const QFileInfo targetFile = QFileInfo(configDir, targetFileName);

    if (!configDir.exists()) {
        if (!QDir::current().mkpath(configDir.absolutePath())) {
            qWarning() << "Failed to create config dir" << configDir.absolutePath();
            return false;
        }
    } else if (targetFile.exists()) {
        if (!configDir.remove(targetFileName)) {
            qWarning() << "Failed to remove existing test config file" << targetFile.absoluteFilePath();
            return false;
        }
    }

    QFile sourceFile(QFINDTESTDATA("ktranscript-test.ini"));
    if (!sourceFile.exists()) {
        qWarning() << "Could not locate test data file" << sourceFile.fileName();
        return false;
    }

    if (!sourceFile.copy(targetFile.absoluteFilePath())) {
        qWarning() << "Failed to copy test config file" << sourceFile.fileName() << "to target location" << targetFile.absoluteFilePath();
        return false;
    }

    return true;
}

bool removeTestConfig()
{
    Q_ASSERT(QStandardPaths::isTestModeEnabled());

    QDir configDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    if (!configDir.exists()) {
        return true;
    }

    return configDir.remove(targetFileName);
}
