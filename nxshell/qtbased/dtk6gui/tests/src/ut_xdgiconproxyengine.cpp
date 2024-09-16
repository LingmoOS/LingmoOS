// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>

#include <DGuiApplicationHelper>
#include <QIcon>
#include <QPainter>
#include <QIODevice>
#include <QRandomGenerator>

#include <private/qicon_p.h>
#define private public
#include "xdgiconproxyengine_p.h"
#include <private/xdgiconloader/xdgiconloader_p.h>
#undef private

DGUI_USE_NAMESPACE

class ut_XdgIconProxyEngine : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    XdgIconProxyEngine *mIconEngine = nullptr;
    QSize s64 = QSize(64, 64);
};

void ut_XdgIconProxyEngine::SetUp()
{
    // :/icons/deepin/xxx.svg
    QIconLoader::instance()->setThemeName("deepin");
    mIconEngine = new XdgIconProxyEngine(new XdgIconLoaderEngine("cs_rect_64"));
}

void ut_XdgIconProxyEngine::TearDown()
{
    delete mIconEngine;
}

TEST_F(ut_XdgIconProxyEngine, entryCacheKey)
{
    ScalableEntry *entry = reinterpret_cast<ScalableEntry *>(QRandomGenerator::global()->generate());

    QSet<quint64> keySet;
    for (int i = QIcon::Normal; i <= QIcon::Selected; ++i) {
        for (int j = QIcon::On; j <= QIcon::Off; ++j) {
            quint64 key = XdgIconProxyEngine::entryCacheKey(entry, QIcon::Mode(i), QIcon::State(j));
            quint64 calKey = quint64(entry) ^ (quint64(i) << 56) ^ (quint64(j) << 48);
            EXPECT_EQ(key, calKey);
            // conflict check
            EXPECT_FALSE(keySet.contains(key));
            keySet.insert(key);
        }
    }
}

    /* cs_rect_64.svg
     * +-------------+-------------+
     * |             |             |
     * |  Highlight  |    Text     |
     * |             |             |
     * +-------------+-------------+
    */

void testHighlightColor(const QPalette &pa, const QImage &img)
{
    EXPECT_FALSE(img.isNull());
    // QColor(AHSL) && QColor(ARGB) ?
    EXPECT_EQ(pa.highlight().color().toRgb(), img.pixelColor(QPoint(16, 16)).name());
}

void testWindowTextColor(const QPalette &pa, const QImage &img)
{
    EXPECT_FALSE(img.isNull());
    EXPECT_EQ(pa.windowText().color(), img.pixelColor(QPoint(48, 16)));
}

void testHighlightedTextColor(const QPalette &pa, const QImage &img)
{
    EXPECT_FALSE(img.isNull());
    EXPECT_EQ(pa.highlightedText().color(), img.pixelColor(QPoint(48, 16)));
}

TEST_F(ut_XdgIconProxyEngine, pixmapByEntry)
{
    QPalette pa = qApp->palette();
    // ensureLoaded
    EXPECT_EQ(s64, mIconEngine->actualSize(s64, QIcon::Normal, QIcon::On));
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QIconLoaderEngineEntry *entry = mIconEngine->engine->entryForSize(s64);
#else
    QIconLoaderEngineEntry *entry = mIconEngine->engine->entryForSize(mIconEngine->engine->m_info, s64);
#endif
    ASSERT_TRUE(entry);

    QPixmap normalPix = mIconEngine->pixmapByEntry(entry, s64, QIcon::Normal, QIcon::On);

    testWindowTextColor(pa, normalPix.toImage());

    QPixmap selectPix = mIconEngine->pixmapByEntry(entry, s64, QIcon::Selected, QIcon::On);

    testHighlightedTextColor(pa, selectPix.toImage());

    testHighlightColor(pa, normalPix.toImage());
}

TEST_F(ut_XdgIconProxyEngine, paint)
{
    QPalette pa = qApp->palette();
    QImage img(s64, QImage::Format_ARGB32_Premultiplied);
    {
        img.fill(Qt::transparent);
        QPainter p(&img);
        mIconEngine->paint(&p, QRect(QPoint(0, 0), s64),QIcon::Normal, QIcon::On);

        testHighlightColor(pa, img);
        testWindowTextColor(pa, img);
    }
    {
        img.fill(Qt::transparent);
        QPainter p(&img);
        mIconEngine->paint(&p, QRect(QPoint(0, 0), s64),QIcon::Selected, QIcon::On);

        testHighlightColor(pa, img);
        testHighlightedTextColor(pa, img);
    }
}

TEST_F(ut_XdgIconProxyEngine, pixmap)
{
    QPalette pa = qApp->palette();

    QPixmap normalPix = mIconEngine->pixmap(s64, QIcon::Normal, QIcon::On);
    testWindowTextColor(pa, normalPix.toImage());

    QPixmap selectPix = mIconEngine->pixmap(s64, QIcon::Selected, QIcon::On);
    testHighlightColor(pa, selectPix.toImage());
    testHighlightedTextColor(pa, selectPix.toImage());
}

TEST_F(ut_XdgIconProxyEngine, addPixmap_addFile)
{
    QString fileName(":/icons/deepin/actions/64/cs_rect_64.svg");
    // XdgIconLoaderEngine did not implement these methods
    mIconEngine->addPixmap(QPixmap(fileName), QIcon::Normal, QIcon::On);
    mIconEngine->addFile(fileName, s64, QIcon::Normal, QIcon::On);
}

TEST_F(ut_XdgIconProxyEngine, key)
{
    ASSERT_EQ(mIconEngine->key(), QLatin1String("XdgIconProxyEngine"));
}

TEST_F(ut_XdgIconProxyEngine, actualSize)
{
    EXPECT_EQ(s64, mIconEngine->actualSize(s64, QIcon::Normal, QIcon::On));
}

TEST_F(ut_XdgIconProxyEngine, clone)
{
    QScopedPointer clone(mIconEngine->clone());
    ASSERT_EQ(clone->key(), mIconEngine->key());
    ASSERT_EQ(clone->iconName(), mIconEngine->iconName());
}

TEST_F(ut_XdgIconProxyEngine, read_write)
{
    QByteArray data;
    {
        QDataStream out(&data, QIODevice::WriteOnly);
        mIconEngine->write(out);
    }
    QString iconName;
    {
        QDataStream in(&data, QIODevice::ReadOnly);
        in >> iconName;
        ASSERT_EQ("cs_rect_64", iconName);
    }

    iconName = "test_icon_name";
    {
        QDataStream out(&data, QIODevice::WriteOnly);
        out << iconName;
    }
    {
        QDataStream in(&data, QIODevice::ReadOnly);
        mIconEngine->read(in);
        ASSERT_EQ(mIconEngine->engine->m_iconName, iconName);
        // iconName() not implement
        ASSERT_EQ(mIconEngine->iconName(), "");
        ASSERT_EQ(mIconEngine->engine->iconName(), "");
    }
}

TEST_F(ut_XdgIconProxyEngine, virtual_hook)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 9, 0)
    {
        QIconEngine::ScaledPixmapArgument scalePixmapArg = { s64, QIcon::Normal, QIcon::On, 1.25, QPixmap() };
        mIconEngine->virtual_hook(QIconEngine::ScaledPixmapHook, &scalePixmapArg);
        ASSERT_EQ(scalePixmapArg.pixmap.size(), s64);
    }
#endif
}

