// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>

#include <QPainter>
#include <QIODevice>
#include <private/qicon_p.h>

#include <DGuiApplicationHelper>
#include <DPlatformTheme>
#define private public
#include "diconproxyengine_p.h"
#undef private

DGUI_USE_NAMESPACE
/*
    bool read(QDataStream &in) override;
    bool write(QDataStream &out) const override;

private:
    void virtual_hook(int id, void *data) override;
*/

class ut_DIconProxyEngine : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    DIconProxyEngine *mIconEngine = nullptr;
    QSize s64 = QSize(64, 64);
};

void ut_DIconProxyEngine::SetUp()
{
    mIconEngine = new DIconProxyEngine("selected_indicator" ,DIconTheme::DontFallbackToQIconFromTheme);
}

void ut_DIconProxyEngine::TearDown()
{
    delete mIconEngine;
}

TEST_F(ut_DIconProxyEngine, key_proxyKey)
{
    EXPECT_EQ(mIconEngine->key(), QLatin1String("DIconProxyEngine"));
    EXPECT_EQ(mIconEngine->proxyKey(), QLatin1String("DBuiltinIconEngine"));
}

TEST_F(ut_DIconProxyEngine, themeName)
{
    QString themeName = DGuiApplicationHelper::instance()->applicationTheme()->iconThemeName();

    EXPECT_EQ(mIconEngine->themeName(), themeName);
}

TEST_F(ut_DIconProxyEngine, iconName)
{
    EXPECT_EQ(mIconEngine->iconName(), "selected_indicator");
}

TEST_F(ut_DIconProxyEngine, paint)
{
    QImage img(s64, QImage::Format_ARGB32_Premultiplied);
    {
        img.fill(Qt::transparent);
        QPainter p(&img);
        mIconEngine->paint(&p, QRect({0, 0}, s64), QIcon::Normal, QIcon::On);
    }
    QImage img1(s64, QImage::Format_ARGB32_Premultiplied);
    {
        img1.fill(Qt::transparent);
        QPainter p(&img1);
        ASSERT_TRUE(mIconEngine->m_iconEngine);
        mIconEngine->m_iconEngine->paint(&p, QRect({0, 0}, s64), QIcon::Normal, QIcon::On);
    }
    ASSERT_TRUE(img == img1);
}

TEST_F(ut_DIconProxyEngine, actualSize)
{
    ASSERT_TRUE(mIconEngine->m_iconEngine);
    QSize s1 = mIconEngine->actualSize(s64, QIcon::Normal, QIcon::On);
    QSize s2 = mIconEngine->m_iconEngine->actualSize(s64, QIcon::Normal, QIcon::On);
    ASSERT_TRUE(s1 == s2);
}

TEST_F(ut_DIconProxyEngine, pixmap)
{
    ASSERT_TRUE(mIconEngine->m_iconEngine);
    ASSERT_EQ(mIconEngine->pixmap(s64, QIcon::Normal, QIcon::On).toImage(),
              mIconEngine->m_iconEngine->pixmap(s64, QIcon::Normal, QIcon::On).toImage());
}

TEST_F(ut_DIconProxyEngine, clone)
{
    QScopedPointer clone(mIconEngine->clone());
    ASSERT_EQ(clone->key(), mIconEngine->key());
    ASSERT_EQ(clone->iconName(), mIconEngine->iconName());
}

TEST_F(ut_DIconProxyEngine, read_write)
{
    QByteArray data;
    {
        QDataStream out(&data, QIODevice::WriteOnly);
        mIconEngine->write(out);
    }
    QString iconName, themeName;
    {
        QDataStream in(&data, QIODevice::ReadOnly);
        in >> iconName >> themeName;
        ASSERT_EQ(themeName, mIconEngine->themeName());
        ASSERT_EQ(iconName, mIconEngine->iconName());
    }

    {
        auto engine = new DIconProxyEngine("selected_indicator" ,DIconTheme::DontFallbackToQIconFromTheme);
        QIcon icon_write(engine);
        QDataStream out(&data, QIODevice::WriteOnly);
        out << icon_write;

        QIcon icon_read;
        QDataStream in(&data, QIODevice::ReadOnly);
        in >> icon_read;

        ASSERT_TRUE(icon_read.data_ptr()->engine);
        EXPECT_EQ(icon_read.data_ptr()->engine->key(),
                  engine->key());
        EXPECT_EQ(icon_read.data_ptr()->engine->iconName(),
                    engine->iconName());
    }
}

TEST_F(ut_DIconProxyEngine, virtual_hook)
{
    ASSERT_TRUE(mIconEngine->m_iconEngine);
    bool data1 = false, data2 = false;
    mIconEngine->virtual_hook(QIconEngine::IsNullHook, &data1);
    mIconEngine->m_iconEngine->virtual_hook(QIconEngine::IsNullHook, &data2);

    ASSERT_EQ(data1, data2);
}
