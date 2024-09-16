// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include <DGuiApplicationHelper>
#include <QIcon>
#include <QPainter>
#include <QIODevice>

#define private public
#include "dbuiltiniconengine_p.h"
#undef private

DGUI_USE_NAMESPACE

#define ICONNAME "icon_Layout"
#define ICONSIZE 16

class ut_DBuiltinIconEngine : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    DBuiltinIconEngine *mIconEngine;
};

void ut_DBuiltinIconEngine::SetUp()
{
    mIconEngine = new DBuiltinIconEngine(ICONNAME);
}

void ut_DBuiltinIconEngine::TearDown()
{
    delete mIconEngine;
}

QIconLoaderEngineEntry *firstEntry(const QThemeIconEntries &entries)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
    QIconLoaderEngineEntry *entry = entries.first();
#else
    QIconLoaderEngineEntry *entry = entries.begin()->get();
#endif
    return entry;
}


TEST_F(ut_DBuiltinIconEngine, loadIcon)
{
    const QThemeIconInfo &themeInfo = mIconEngine->loadIcon(ICONNAME, DGuiApplicationHelper::instance()->themeType());

    ASSERT_EQ(themeInfo.iconName, ICONNAME);

    QIconLoaderEngineEntry *entry = firstEntry(themeInfo.entries);
    QString builtinActionPath = ":/icons/deepin/builtin/actions";

    ASSERT_TRUE(entry->filename.contains(ICONNAME));
    ASSERT_EQ(entry->dir.path, builtinActionPath);
    ASSERT_EQ(entry->dir.size, ICONSIZE);
    ASSERT_EQ(entry->dir.type, QIconDirInfo::Scalable);
    ASSERT_FALSE(entry->pixmap(QSize(ICONSIZE, ICONSIZE), QIcon::Normal, QIcon::On).isNull());

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
    for (auto item : themeInfo.entries) {
        delete item;
    }
#endif
}

TEST_F(ut_DBuiltinIconEngine, actualSize)
{
    QSize size(ICONSIZE, ICONSIZE);

    QSize iconSize = mIconEngine->actualSize(size, QIcon::Normal, QIcon::On);
    ASSERT_FALSE(iconSize.isEmpty());
    QIconLoaderEngineEntry *entry = firstEntry(mIconEngine->m_info.entries);
    if (entry->dir.type == QIconDirInfo::Scalable) {
        ASSERT_EQ(iconSize, size);
    } else {
        bool isTrue = (iconSize.width() <= qMin(iconSize.width(), iconSize.height())) && (iconSize.height() <= qMin(iconSize.width(), iconSize.height()));
        ASSERT_TRUE(isTrue);
    }
}

TEST_F(ut_DBuiltinIconEngine, pixmap)
{
    QPixmap iconenginePixmap = mIconEngine->pixmap(QSize(ICONSIZE, ICONSIZE), QIcon::Normal, QIcon::On);

    ASSERT_FALSE(iconenginePixmap.isNull());
}

TEST_F(ut_DBuiltinIconEngine, paint)
{
    QImage iconImage(QSize(ICONSIZE, ICONSIZE), QImage::Format_ARGB32);
    QPainter iconPainter(&iconImage);

    mIconEngine->paint(&iconPainter, QRect({0, 0}, iconImage.size()), QIcon::Normal, QIcon::On);
    ASSERT_FALSE(iconImage.isNull());
}

TEST_F(ut_DBuiltinIconEngine, key)
{
    ASSERT_EQ(mIconEngine->key(), "DBuiltinIconEngine");
}

TEST_F(ut_DBuiltinIconEngine, clone)
{
    auto clone = mIconEngine->clone();
    ASSERT_FALSE(!clone);
    delete clone;
}

TEST_F(ut_DBuiltinIconEngine, read)
{
    QByteArray data;
    uint key = 1;
    bool followSystemTheme = true;
    QString iconName = ICONNAME;
    QDataStream out(&data, QIODevice::WriteOnly);
    out << iconName << key << followSystemTheme;

    QDataStream in(&data, QIODevice::ReadOnly);
    mIconEngine->read(in);

    ASSERT_EQ(mIconEngine->m_iconName, iconName);
    ASSERT_EQ(mIconEngine->m_key, key);
    ASSERT_EQ(mIconEngine->m_followSystemTheme, followSystemTheme);
}

TEST_F(ut_DBuiltinIconEngine, write)
{
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    mIconEngine->write(out);

    uint key = 0;
    bool followSystemTheme = false;
    QString iconName;

    QDataStream in(&data, QIODevice::ReadOnly);
    in >> iconName >> key >> followSystemTheme;

    ASSERT_EQ(mIconEngine->m_iconName, iconName);
    ASSERT_EQ(mIconEngine->m_key, key);
    ASSERT_EQ(mIconEngine->m_followSystemTheme, followSystemTheme);
}

TEST_F(ut_DBuiltinIconEngine, iconName)
{
    ASSERT_EQ(mIconEngine->iconName(), ICONNAME);
}

TEST_F(ut_DBuiltinIconEngine, ensureLoaded)
{
    mIconEngine->ensureLoaded();

    ASSERT_EQ(mIconEngine->m_key, DGuiApplicationHelper::instance()->themeType());
    ASSERT_TRUE(mIconEngine->m_initialized);

    ASSERT_EQ(mIconEngine->m_info.iconName, ICONNAME);
    QIconLoaderEngineEntry *entry = firstEntry(mIconEngine->m_info.entries);
    QString builtinActionPath = ":/icons/deepin/builtin/actions";

    ASSERT_TRUE(entry->filename.contains(ICONNAME));
    ASSERT_EQ(entry->dir.path, builtinActionPath);
    ASSERT_EQ(entry->dir.size, ICONSIZE);
    ASSERT_EQ(entry->dir.type, QIconDirInfo::Scalable);
}

TEST_F(ut_DBuiltinIconEngine, hasIcon)
{
    mIconEngine->ensureLoaded();
    ASSERT_TRUE(mIconEngine->hasIcon());
}

TEST_F(ut_DBuiltinIconEngine, virtual_hook)
{
    void *arg;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QIconEngine::AvailableSizesArgument data;
    arg = reinterpret_cast<void *>(&data);
    mIconEngine->virtual_hook(QIconEngine::AvailableSizesHook, arg);
    ASSERT_FALSE(data.sizes.isEmpty());

    QString icon_name;
    arg = reinterpret_cast<void *>(&icon_name);
    mIconEngine->virtual_hook(QIconEngine::IconNameHook, arg);
    ASSERT_EQ(icon_name, ICONNAME);
#endif

    bool isNull = true;
    arg = reinterpret_cast<void *>(&isNull);
    mIconEngine->virtual_hook(QIconEngine::IsNullHook, arg);
    ASSERT_FALSE(isNull);

    QIconEngine::ScaledPixmapArgument pixmapData;
    // 参数值会在函数内部进行判断，需要初始化后传入 并判断函数是否执行成功
    pixmapData.size = QSize(ICONSIZE, ICONSIZE);
    pixmapData.scale = 1;
    arg = reinterpret_cast<void *>(&pixmapData);
    mIconEngine->virtual_hook(QIconEngine::ScaledPixmapHook, arg);
    ASSERT_FALSE(pixmapData.pixmap.isNull());
}
