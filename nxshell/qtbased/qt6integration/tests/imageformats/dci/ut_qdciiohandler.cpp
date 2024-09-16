/*
 * SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */
#include "qdciiohandler.h"

#include <gtest/gtest.h>

#include <DDciIcon>
#include <QColor>
#include <QFile>
#include <QImage>
#include <QRect>
#include <QVariant>
#include <QDebug>

DGUI_USE_NAMESPACE

class ut_QDciIOHandler : public testing::Test
{
protected:
    void SetUp();
    void TearDown();
};

void ut_QDciIOHandler::SetUp()
{
}

void ut_QDciIOHandler::TearDown()
{
}

TEST_F(ut_QDciIOHandler, testLoad)
{
    QFile file(":/radio_checked.dci");
    ASSERT_TRUE(file.open(QIODevice::ReadOnly));
    QDciIOHandler handler;
    handler.setDevice(&file);
    handler.setFormat("dci");
    ASSERT_TRUE(QDciIOHandler::canRead(&file));
    file.seek(0);
    ASSERT_TRUE(handler.canRead());
    ASSERT_TRUE(handler.option(QImageIOHandler::Size).toSize().isValid());

    QImage image;
    ASSERT_TRUE(handler.read(&image));
    ASSERT_FALSE(image.isNull());
}

static bool imagePixelEQ(const QImage &image1, const QImage &image2)
{
    if (image1.sizeInBytes() != image2.sizeInBytes())
        return false;
    for (auto i = image1.sizeInBytes() - 1; i >= 0; --i)
        if (image1.constBits()[i] != image2.constBits()[i])
            return false;
    return true;
}

TEST_F(ut_QDciIOHandler, testSetBackground)
{
    QFile file(":/radio_checked.dci");
    ASSERT_TRUE(file.open(QIODevice::ReadOnly));
    QDciIOHandler handler;
    handler.setDevice(&file);
    handler.setFormat("dci");

    QImage image;
    handler.setOption(QImageIOHandler::BackgroundColor, QColor(Qt::red));
    ASSERT_TRUE(handler.read(&image));
    ASSERT_FALSE(image.isNull());

    const QSize size = handler.option(QImageIOHandler::Size).toSize();

    DDciIcon dci(file.fileName());
    ASSERT_FALSE(dci.isNull());
    auto icon = dci.matchIcon(size.width(), DDciIcon::Light, DDciIcon::Normal, DDciIcon::DontFallbackMode);
    ASSERT_NE(icon, nullptr);
    ASSERT_TRUE(dci.isSupportedAttribute(icon, DDciIcon::HasPalette));
    DDciIconPalette pa(QColor::Invalid, QColor(Qt::red));
    QImage image2 = dci.pixmap(1.0, 0, icon, pa).toImage();
    ASSERT_FALSE(image2.isNull());
    ASSERT_TRUE(imagePixelEQ(image, image2));
}

TEST_F(ut_QDciIOHandler, testSetFormat)
{
    QFile file(":/radio_checked.dci");
    ASSERT_TRUE(file.open(QIODevice::ReadOnly));
    DDciIcon dci(file.fileName());
    ASSERT_FALSE(dci.isNull());

    {
        file.seek(0);
        QImage image;
        QDciIOHandler handler;
        handler.setDevice(&file);
        handler.setFormat("dci-light-hover");
        ASSERT_TRUE(handler.read(&image));
        ASSERT_FALSE(image.isNull());

        const QSize size = handler.option(QImageIOHandler::Size).toSize();
        auto icon = dci.matchIcon(size.width(), DDciIcon::Light, DDciIcon::Hover, DDciIcon::DontFallbackMode);
        ASSERT_NE(icon, nullptr);
        QImage image2 = dci.pixmap(1.0, 0, icon).toImage();
        ASSERT_FALSE(image2.isNull());
        ASSERT_TRUE(imagePixelEQ(image, image2));
    }

    {
        file.seek(0);
        QImage image;
        QDciIOHandler handler;
        handler.setDevice(&file);
        handler.setFormat("dci-dark-pressed");
        ASSERT_TRUE(handler.read(&image));
        ASSERT_FALSE(image.isNull());

        const QSize size = handler.option(QImageIOHandler::Size).toSize();
        auto icon = dci.matchIcon(size.width(), DDciIcon::Dark, DDciIcon::Pressed, DDciIcon::DontFallbackMode);
        ASSERT_NE(icon, nullptr);
        QImage image2 = dci.pixmap(1.0, 0, icon).toImage();
        ASSERT_FALSE(image2.isNull());
        ASSERT_TRUE(imagePixelEQ(image, image2));
    }
}
