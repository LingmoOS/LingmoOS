/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2008 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <kiconloader.h>
#include <kpixmapsequenceloader.h>

#include <QDir>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTest>

#include <KPixmapSequence>

#include <KConfigGroup>
#include <KSharedConfig>

extern KICONTHEMES_EXPORT void uintToHex(uint32_t colorData, QChar *buffer);

class KPixmapSequenceLoader_UnitTest : public QObject
{
    Q_OBJECT
public:
    KPixmapSequenceLoader_UnitTest()
    {
    }

private:
    QDir testDataDir;
    QDir testIconsDir;

private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);

        KConfigGroup cg(KSharedConfig::openConfig(), "Icons");
        cg.writeEntry("Theme", "oxygen");
        cg.sync();

        testDataDir = QDir(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation));
        testIconsDir = QDir(testDataDir.absoluteFilePath(QStringLiteral("icons")));

        // we will be recursively deleting these, so a sanity check is in order
        QVERIFY(testIconsDir.absolutePath().contains(QLatin1String("qttest")));

        testIconsDir.removeRecursively();

        // set up a minimal Oxygen icon theme, in case it is not installed
        QVERIFY(testIconsDir.mkpath(QStringLiteral("oxygen/22x22/animations")));
        QVERIFY(QFile::copy(QStringLiteral(":/oxygen.theme"), testIconsDir.filePath(QStringLiteral("oxygen/index.theme"))));
        QVERIFY(QFile::copy(QStringLiteral(":/anim-22x22.png"), testIconsDir.filePath(QStringLiteral("oxygen/22x22/animations/process-working.png"))));
    }

    void cleanupTestCase()
    {
        if (testIconsDir != QDir()) {
            testIconsDir.removeRecursively();
        }
    }

    void init()
    {
        // Remove icon cache
        const QString cacheFile = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + QStringLiteral("/icon-cache.kcache");
        QFile::remove(cacheFile);

        // Clear SHM cache
        KIconLoader::global()->reconfigure(QString());
    }

    void testLoadPixmapSequence()
    {
        KPixmapSequence seq = KPixmapSequenceLoader::load(QStringLiteral("process-working"), 22);
        QVERIFY(seq.isValid());
    }
};

QTEST_MAIN(KPixmapSequenceLoader_UnitTest)

#include "kpixmapsequenceloadertest.moc"
