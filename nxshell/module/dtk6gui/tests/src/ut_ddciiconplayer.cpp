// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test.h"
#include "ddciicon.h"
#include "ddciiconplayer.h"

#include <QGuiApplication>
#include <QImage>
#include <QImageReader>
#include <QPainter>
#include <QSignalSpy>
#include <QTimer>

DGUI_USE_NAMESPACE

static void image_dye(QImage &image, const QColor &color) {
    QPainter pa(&image);
    pa.setCompositionMode(QPainter::CompositionMode_SourceIn);
    pa.fillRect(image.rect(), color);
}

TEST(ut_DDciIconImage, render)
{
    DDciIcon icon(QStringLiteral(":/images/dci_heart.dci"));
    ASSERT_FALSE(icon.isNull());

    {
        // normal
        auto result = icon.matchIcon(-1, DDciIcon::Light, DDciIcon::Normal);
        QImageReader reader(":/images/dci_heart_dci_normal.webp");

        DDciIconImage image = icon.image(result, reader.size().width(), 1.0);
        ASSERT_FALSE(image.isNull());
        ASSERT_TRUE(image.hasPalette());

        QImage image1 = image.toImage(DDciIconPalette(Qt::red));
        QImage image2 = reader.read().convertToFormat(image1.format());
        image2.setDevicePixelRatio(1.0);
        image_dye(image2, Qt::red);

        ASSERT_EQ(image1, image2);
    }

    { // hover
        auto result = icon.matchIcon(-1, DDciIcon::Light, DDciIcon::Hover);
        QImageReader reader(":/images/dci_heart_dci_hover.webp");

        DDciIconImage image = icon.image(result, reader.size().width(), 1.0);
        ASSERT_FALSE(image.isNull());
        ASSERT_TRUE(reader.canRead());
        ASSERT_TRUE(image.supportsAnimation());
        ASSERT_TRUE(reader.supportsAnimation());
        ASSERT_TRUE(image.atBegin());

        ASSERT_EQ(image.loopCount(), reader.loopCount());
        ASSERT_EQ(image.maxImageCount(), reader.imageCount());

        while (!image.atEnd()) {
            QImage image1 = image.toImage();
            QImage image2 = reader.read().convertToFormat(image1.format());
            ASSERT_EQ(image1, image2);
            ASSERT_EQ(image.currentImageDuration(), reader.nextImageDelay());
            ASSERT_TRUE(image.jumpToNextImage());
        }
    }

    { // pressed
        auto result = icon.matchIcon(-1, DDciIcon::Light, DDciIcon::Pressed);
        QImageReader reader(":/images/dci_heart_dci_pressed.webp");

        DDciIconImage image = icon.image(result, reader.size().width(), 1.0);
        ASSERT_FALSE(image.isNull());
        ASSERT_TRUE(reader.canRead());
        ASSERT_TRUE(image.supportsAnimation());
        ASSERT_TRUE(reader.supportsAnimation());
        ASSERT_TRUE(image.atBegin());

        ASSERT_EQ(image.loopCount(), reader.loopCount());
        ASSERT_EQ(image.maxImageCount(), reader.imageCount());

        while (!image.atEnd()) {
            QImage image1 = image.toImage();
            QImage image2 = reader.read().convertToFormat(image1.format());
            ASSERT_EQ(image1, image2);
            ASSERT_EQ(image.currentImageDuration(), reader.nextImageDelay());
            ASSERT_TRUE(image.jumpToNextImage());
        }
    }
}

class ut_DDciIconPlayer : public DTest
{
protected:
    ut_DDciIconPlayer()
        : update_signal_spy(&player, &DDciIconPlayer::updated)
    {}

    void SetUp() override;
    void TearDown() override;

    DDciIconPlayer player;
    QSignalSpy update_signal_spy;
};

void ut_DDciIconPlayer::SetUp()
{
    DDciIcon icon(QStringLiteral(":/images/dci_heart.dci"));
    ASSERT_FALSE(icon.isNull());

    player.setIcon(icon);
    player.setTheme(DDciIcon::Light);
    player.setIconSize(200);
    player.setDevicePixelRatio(1.0);
    player.setPalette(DDciIconPalette(Qt::red));

    qputenv("D_DTK_DCI_PLAYER_IGNORE_ANIMATION_LOOP", "1");
}

void ut_DDciIconPlayer::TearDown()
{
    qunsetenv("D_DTK_DCI_PLAYER_IGNORE_ANIMATION_LOOP");
}

TEST_F(ut_DDciIconPlayer, play)
{
    QImageReader reader(":/images/dci_heart_dci_hover.webp");

    update_signal_spy.clear();
    player.play(DDciIcon::Hover);
    while (update_signal_spy.wait(100));

    ASSERT_EQ(update_signal_spy.count(), reader.imageCount());
}

TEST_F(ut_DDciIconPlayer, state)
{
    player.abort();
    ASSERT_EQ(player.state(), DDciIconPlayer::Idle);
    QSignalSpy state_changed_signal_spy(&player, &DDciIconPlayer::stateChanged);
    player.play(DDciIcon::Hover);
    ASSERT_EQ(player.state(), DDciIconPlayer::Busy);
    ASSERT_TRUE(state_changed_signal_spy.wait(2000));
    ASSERT_EQ(player.state(), DDciIconPlayer::Idle);
    ASSERT_EQ(state_changed_signal_spy.count(), 2);

    player.play(DDciIcon::Pressed);
    ASSERT_EQ(player.state(), DDciIconPlayer::Busy);
    player.stop();
    ASSERT_EQ(player.state(), DDciIconPlayer::Idle);
    ASSERT_EQ(state_changed_signal_spy.count(), 4);

    player.setMode(DDciIcon::Normal);
    player.setMode(DDciIcon::Pressed);
    ASSERT_TRUE(update_signal_spy.wait(100));
    ASSERT_EQ(player.state(), DDciIconPlayer::Busy);
    player.abort();
    ASSERT_FALSE(update_signal_spy.wait(100));
    ASSERT_EQ(player.state(), DDciIconPlayer::Idle);
    ASSERT_EQ(state_changed_signal_spy.count(), 6);

    player.setMode(DDciIcon::Normal);
    int idle_count = 0;
    while (update_signal_spy.wait(100))
        if (player.state() == DDciIconPlayer::Idle)
            ++idle_count;
    ASSERT_EQ(idle_count, 1);
    ASSERT_EQ(state_changed_signal_spy.count(), 8);
}

TEST_F(ut_DDciIconPlayer, setMode)
{
    QImageReader hover(":/images/dci_heart_dci_hover.webp");
    QImageReader pressed(":/images/dci_heart_dci_pressed.webp");

    player.setMode(DDciIcon::Normal);
    player.abort();
    ASSERT_EQ(player.state(), DDciIconPlayer::Idle);

    { // normal -> hover
        update_signal_spy.clear();
        player.setMode(DDciIcon::Hover);
        while (update_signal_spy.wait(100));
        ASSERT_EQ(update_signal_spy.count(), hover.imageCount());
    }

    { // hover -> pressed
        update_signal_spy.clear();
        player.setMode(DDciIcon::Pressed);
        while (update_signal_spy.wait(100));
        ASSERT_EQ(update_signal_spy.count(), pressed.imageCount());
    }

    { // pressed -> hover
        update_signal_spy.clear();
        player.setMode(DDciIcon::Hover);
        while (update_signal_spy.wait(100));
        // the animation is stop in the hover's last image, so needs +1
        ASSERT_EQ(update_signal_spy.count(), pressed.imageCount() + 1);
    }

    { // hover -> normal
        update_signal_spy.clear();
        player.setMode(DDciIcon::Normal);
        while (update_signal_spy.wait(100));
        // the animation is stop in the normal state, so needs +1
        ASSERT_EQ(update_signal_spy.count(), hover.imageCount() + 1);
    }

    { // normal -> pressed
        update_signal_spy.clear();
        player.setMode(DDciIcon::Pressed);
        while (update_signal_spy.wait(100));
        ASSERT_EQ(update_signal_spy.count(), hover.imageCount() + pressed.imageCount());
    }

    { // pressed -> normal
        update_signal_spy.clear();
        player.setMode(DDciIcon::Normal);
        while (update_signal_spy.wait(100));
        // the animation is stop in the normal state, so needs +1
        ASSERT_EQ(update_signal_spy.count(), hover.imageCount() + pressed.imageCount() + 1);
    }

    {
        player.setMode(DDciIcon::Normal);
        player.abort();

        // hover -> disabled
        update_signal_spy.clear();
        player.setMode(DDciIcon::Hover);
        while (update_signal_spy.wait(100)) {
            player.setMode(DDciIcon::Disabled);
        }
        ASSERT_EQ(update_signal_spy.count(), 2);

        QImageReader normal(":/images/dci_heart_dci_normal.webp");
        QImage image1 = player.currentImage();
        QImage image2 = normal.read().convertToFormat(image1.format());
        image2.setDevicePixelRatio(1.0);
        image_dye(image2, Qt::red);

        ASSERT_EQ(image1, image2);
    }

    {
        update_signal_spy.clear();
        // disable -> pressed
        player.setMode(DDciIcon::Pressed);
        update_signal_spy.wait(100);
        ASSERT_EQ(update_signal_spy.count(), 1);

        QImageReader normal(":/images/dci_heart_dci_normal.webp");
        QImage image1 = player.currentImage();
        QImage image2 = normal.read().convertToFormat(image1.format());
        image2.setDevicePixelRatio(1.0);
        image_dye(image2, Qt::red);
        ASSERT_EQ(image1, image2);
    }
}

TEST_F(ut_DDciIconPlayer, animationInvertedOrderAndContinue)
{
    QImageReader hover(":/images/dci_heart_dci_hover.webp");
    QImageReader pressed(":/images/dci_heart_dci_pressed.webp");

    player.setMode(DDciIcon::Normal);
    player.abort();
    ASSERT_EQ(player.state(), DDciIconPlayer::Idle);

    QVector<QPair<QImage, int>> images;
    while (hover.canRead()) {
        const QImage &img = hover.read();
        images.append({img, hover.nextImageDelay()});
    }
    while (pressed.canRead()) {
        const QImage &img = pressed.read();
        images.append({img, pressed.nextImageDelay()});
    }

    update_signal_spy.clear();
    player.setMode(DDciIcon::Hover);
    int lastImageIndex = -1;
    while (update_signal_spy.count() < hover.imageCount() / 2 && update_signal_spy.wait(100)) {
        QImage image1 = player.currentImage();
        auto i = images[++lastImageIndex];
        QImage image2 = i.first.convertToFormat(image1.format());
        image2.setDevicePixelRatio(1.0);
        image_dye(image2, Qt::red);
        ASSERT_EQ(image1, image2);
    }
    player.setMode(DDciIcon::Normal);
    ASSERT_TRUE(update_signal_spy.wait(0));

    do {
        ASSERT_TRUE(lastImageIndex > -2); // the last image is normal
        if (lastImageIndex < 0) {
            QImageReader normal(":/images/dci_heart_dci_normal.webp");
            QImage image1 = player.currentImage();
            QImage image2 = normal.read().convertToFormat(image1.format());
            image2.setDevicePixelRatio(1.0);
            image_dye(image2, Qt::red);
            ASSERT_EQ(image1, image2);
            break;
        }

        ASSERT_TRUE(lastImageIndex < images.count());

        QImage image1 = player.currentImage();
        auto i = images[lastImageIndex];
        QImage image2 = i.first.convertToFormat(image1.format());
        image2.setDevicePixelRatio(1.0);
        image_dye(image2, Qt::red);
        ASSERT_EQ(image1, image2);
    } while (update_signal_spy.wait(images[lastImageIndex--].second + 1));

    ASSERT_EQ(lastImageIndex, -1);
}

TEST_F(ut_DDciIconPlayer, animationSequentially)
{
    QImageReader hover(":/images/dci_heart_dci_hover.webp");
    QImageReader pressed(":/images/dci_heart_dci_pressed.webp");

    player.setMode(DDciIcon::Normal);
    player.abort();
    ASSERT_EQ(player.state(), DDciIconPlayer::Idle);

    QVector<QPair<QImage, int>> images;
    while (hover.canRead()) {
        const QImage &img = hover.read();
        images.append({img, hover.nextImageDelay()});
    }
    while (pressed.canRead()) {
        const QImage &img = pressed.read();
        images.append({img, pressed.nextImageDelay()});
    }

    update_signal_spy.clear();
    player.setMode(DDciIcon::Hover);
    player.setMode(DDciIcon::Pressed);

    int lastImageIndex = 0;
    while (update_signal_spy.wait(100)) {
        ASSERT_TRUE(lastImageIndex < images.count());

        QImage image1 = player.currentImage();
        auto i = images[lastImageIndex++];
        QImage image2 = i.first.convertToFormat(image1.format());
        image2.setDevicePixelRatio(1.0);

        if (lastImageIndex <= hover.imageCount()) // the pressed image is not have palette
            image_dye(image2, Qt::red);
        ASSERT_EQ(image1, image2);
    }

    ASSERT_EQ(lastImageIndex, images.count());
}

TEST_F(ut_DDciIconPlayer, nonAnimationImage)
{
    DDciIcon icon(QStringLiteral(":/images/3depict.dci"));
    ASSERT_FALSE(icon.isNull());

    DDciIconPlayer player;
    update_signal_spy.clear();
    player.setIcon(icon);
    update_signal_spy.wait(1000);
    ASSERT_EQ(update_signal_spy.count(), 1);
    ASSERT_FALSE(player.currentImage().isNull());
}
