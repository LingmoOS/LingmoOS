/*
    SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#pragma once

#include <QQuickStyle>
#include <QStandardPaths>
#include <QTest>

using namespace Qt::StringLiterals;

namespace Common
{
void initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);

    if (qEnvironmentVariableIsSet("KDECI_BUILD")) {
        const QString libraryPath = QLatin1String(qgetenv("CMAKE_BINARY_DIR")) + QDir::separator() + "bin"_L1 + QDir::separator() + "org.kde.desktop"_L1
            + QLatin1String(qgetenv("CMAKE_SHARED_LIBRARY_SUFFIX"));
        QVERIFY2(QFileInfo::exists(libraryPath), qUtf8Printable(libraryPath));
        QDir libraryDir = QFileInfo(libraryPath).dir();
        const QString kirigamiFolder(libraryDir.absolutePath() + QDir::separator() + u"kf6"_s + QDir::separator() + u"kirigami" + QDir::separator()
                                     + u"platform"_s);
        QVERIFY2(libraryDir.mkpath(kirigamiFolder), qUtf8Printable(kirigamiFolder));
        const QString targetFilePath = kirigamiFolder + QDir::separator() + QFileInfo(libraryPath).fileName();
        QFile(targetFilePath).remove();
        QVERIFY(QFile(libraryPath).copy(targetFilePath));
        QCoreApplication::addLibraryPath(QString::fromLatin1(qgetenv("CMAKE_BINARY_DIR")));
    }

    QQuickStyle::setStyle(u"org.kde.desktop"_s);
}
}