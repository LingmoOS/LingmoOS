/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2013 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kdeplatformtheme_config.h"
#include "kstyle.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QTest>
#include <QToolBar>
#include <QToolButton>

#include <QDebug>

static void prepareEnvironment()
{
    QStandardPaths::setTestModeEnabled(true);

    QString configPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);

    if (!QDir(configPath).mkpath(QStringLiteral("."))) {
        qFatal("Failed to create test configuration directory.");
    }

    configPath.append("/kdeglobals");

    QFile::remove(configPath);
    if (!QFile::copy(CONFIGFILE, configPath)) {
        qFatal("Failed to copy kdeglobals required for tests.");
    }
}

Q_COREAPP_STARTUP_FUNCTION(prepareEnvironment)

class KStyle_UnitTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        qApp->setStyle(new KStyle);
    }
    void cleanupTestCase()
    {
        QString configPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
        configPath.append("/kdeglobals");
        QFile::remove(configPath);
    }

    void testToolButtonStyleHint()
    {
        QToolBar *toolbar = new QToolBar();
        QToolButton *btn = new QToolButton(toolbar);

        QCOMPARE(qApp->style()->styleHint(QStyle::SH_ToolButtonStyle, nullptr, btn), (int)Qt::ToolButtonTextOnly);

        toolbar->setProperty("otherToolbar", true);
        QCOMPARE(qApp->style()->styleHint(QStyle::SH_ToolButtonStyle, nullptr, btn), (int)Qt::ToolButtonTextUnderIcon);
    }
};

QTEST_MAIN(KStyle_UnitTest)

#include "kstyle_unittest.moc"
