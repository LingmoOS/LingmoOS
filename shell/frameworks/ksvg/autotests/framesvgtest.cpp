/*
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "framesvgtest.h"
#include <QDirIterator>
#include <QStandardPaths>

void copyDirectory(const QString &srcDir, const QString &dstDir)
{
    QDir targetDir(dstDir);
    QDirIterator it(srcDir, QDir::Filters(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Name), QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        QString path = it.filePath();
        QString relDestPath = path.last(it.filePath().length() - srcDir.length() - 1);
        if (it.fileInfo().isDir()) {
            QVERIFY(targetDir.mkpath(relDestPath));
        } else {
            QVERIFY(QFile::copy(path, dstDir % '/' % relDestPath));
        }
    }
}

void FrameSvgTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);

    m_themeDir = QDir(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) % '/' % "plasma");
    m_themeDir.removeRecursively();

    copyDirectory(QFINDTESTDATA("data/plasma"), m_themeDir.absolutePath());

    m_cacheDir = QDir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
    m_cacheDir.removeRecursively();

    m_frameSvg = new KSvg::FrameSvg;
    m_frameSvg->setImagePath(QFINDTESTDATA("data/background.svgz"));
    QVERIFY(m_frameSvg->isValid());
}

void FrameSvgTest::cleanupTestCase()
{
    delete m_frameSvg;

    m_themeDir.removeRecursively();
    m_cacheDir.removeRecursively();
}

void FrameSvgTest::margins()
{
    QCOMPARE(m_frameSvg->marginSize(KSvg::FrameSvg::LeftMargin), (qreal)26);
    QCOMPARE(m_frameSvg->marginSize(KSvg::FrameSvg::TopMargin), (qreal)26);
    QCOMPARE(m_frameSvg->marginSize(KSvg::FrameSvg::RightMargin), (qreal)26);
    QCOMPARE(m_frameSvg->marginSize(KSvg::FrameSvg::BottomMargin), (qreal)26);
}

void FrameSvgTest::contentsRect()
{
    m_frameSvg->resizeFrame(QSize(100, 100));
    QCOMPARE(m_frameSvg->contentsRect(), QRectF(26, 26, 48, 48));
}

void FrameSvgTest::repaintBlocked()
{
    // check the properties to be correct even if set during a repaint blocked transaction
    m_frameSvg->setRepaintBlocked(true);
    QVERIFY(m_frameSvg->isRepaintBlocked());

    m_frameSvg->setElementPrefix("prefix");
    m_frameSvg->setEnabledBorders(KSvg::FrameSvg::TopBorder | KSvg::FrameSvg::LeftBorder);
    m_frameSvg->resizeFrame(QSizeF(100, 100));

    m_frameSvg->setRepaintBlocked(false);

    QCOMPARE(m_frameSvg->prefix(), QString("prefix"));
    QCOMPARE(m_frameSvg->enabledBorders(), KSvg::FrameSvg::TopBorder | KSvg::FrameSvg::LeftBorder);
    QCOMPARE(m_frameSvg->frameSize(), QSizeF(100, 100));
}

void FrameSvgTest::setImageSet()
{
    // Should not crash

    KSvg::FrameSvg *frameSvg = new KSvg::FrameSvg;
    frameSvg->setImagePath("widgets/background");
    frameSvg->setImageSet(new KSvg::ImageSet("breeze-light", {}, this));
    frameSvg->framePixmap();
    frameSvg->setImageSet(new KSvg::ImageSet("breeze-dark", {}, this));
    frameSvg->framePixmap();
    delete frameSvg;

    frameSvg = new KSvg::FrameSvg;
    frameSvg->setImagePath("widgets/background");
    frameSvg->setImageSet(new KSvg::ImageSet("breeze-light", {}, this));
    frameSvg->framePixmap();
    frameSvg->setImageSet(new KSvg::ImageSet("breeze-dark", {}, this));
    frameSvg->framePixmap();

    frameSvg->setImageSet(new KSvg::ImageSet("testtheme", "plasma/desktoptheme", this));
    QCOMPARE(frameSvg->color(KSvg::Svg::Text), QColor(255, 54, 59));

    delete frameSvg;
}

void FrameSvgTest::resizeMask()
{
    m_frameSvg->resizeFrame(QSize(100, 100));
    QCOMPARE(m_frameSvg->alphaMask().size(), QSize(100, 100));
    m_frameSvg->resizeFrame(QSize(50, 50));
    QCOMPARE(m_frameSvg->alphaMask().size(), QSize(50, 50));
    m_frameSvg->resizeFrame(QSize(100, 100));
    QCOMPARE(m_frameSvg->alphaMask().size(), QSize(100, 100));
}

void FrameSvgTest::loadQrc()
{
    KSvg::FrameSvg *frameSvg = new KSvg::FrameSvg;
    frameSvg->setImageSet(new KSvg::ImageSet("testtheme", "plasma/desktoptheme", this));
    frameSvg->setImagePath(QStringLiteral("qrc:/data/background.svgz"));
    QVERIFY(frameSvg->isValid());
    // An external image is colored as well
    QCOMPARE(frameSvg->color(KSvg::Svg::Text), QColor(255, 54, 59));
    delete frameSvg;
}

QTEST_MAIN(FrameSvgTest)

#include "moc_framesvgtest.cpp"
