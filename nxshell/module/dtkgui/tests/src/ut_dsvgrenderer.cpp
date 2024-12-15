// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dsvgrenderer.h"
#include "test.h"
#include <QDebug>
#include <QFile>
#include <QLibrary>
#include <QPainter>
#include <QPixmap>

DGUI_USE_NAMESPACE

TEST(ut_DSvgRenderer, testInit)
{
    DSvgRenderer svg_1;
    DSvgRenderer svg_2(QStringLiteral(":/images/logo_icon.svg"));

    ASSERT_FALSE(svg_1.isValid());
    ASSERT_TRUE(svg_2.isValid());

    QFile file(":/images/logo_icon.svg");
    if (!file.open(QFile::ReadOnly))
        return;

    QByteArray data = file.readAll();
    DSvgRenderer svg_3(data);
    ASSERT_TRUE(svg_3.isValid());
}

class TDSvgRenderer : public DTest
{
protected:
    void SetUp();
    void TearDown();

    DSvgRenderer *renderer;
    bool canLoad;
};

void TDSvgRenderer::SetUp()
{
    QLibrary rsvg("rsvg-2", "2");

    if (!rsvg.isLoaded()) {
        canLoad = rsvg.load();
        if (canLoad) {
            rsvg.unload();
        }
    }

    renderer = new DSvgRenderer;
}

void TDSvgRenderer::TearDown()
{
    delete renderer;
}

TEST_F(TDSvgRenderer, testLoad)
{
    if (!canLoad)
        return;

    ASSERT_TRUE(renderer->load(QStringLiteral(":/images/logo_icon.svg")));
    ASSERT_TRUE(renderer->isValid());
    ASSERT_FALSE(renderer->defaultSize().isEmpty());
    ASSERT_FALSE(renderer->viewBox().isEmpty());
    ASSERT_FALSE(renderer->viewBoxF().isEmpty());
}

static bool testPixmapHasData(const QPixmap &pixmap)
{
    QImage image = pixmap.toImage();

    image.reinterpretAsFormat(QImage::Format_RGB32);
    const QRgb *bits = reinterpret_cast<const QRgb *>(image.constBits());
    const QRgb *end = bits + qulonglong(image.sizeInBytes()) / sizeof(QRgb);
    return !std::all_of(bits, end, [](QRgb r) { return r == QColor(Qt::green).rgb(); });
}

#define TestRenderID QStringLiteral("#tittlebar")
#define TestRenderID_NotExist QStringLiteral("#TestRsvg_notexist")

TEST_F(TDSvgRenderer, testRender)
{
    if (!canLoad)
        return;

    enum { TestPixmapSize = 16 };

    ASSERT_TRUE(renderer->load(QString(":/images/logo_icon.svg")));
    QPixmap tPixmap(QSize(TestPixmapSize, TestPixmapSize));
    tPixmap.fill(Qt::green);
    QPainter tPainter(&tPixmap);

    renderer->render(&tPainter);
    ASSERT_TRUE(testPixmapHasData(tPixmap));
    tPixmap.fill(Qt::green);
    ASSERT_FALSE(testPixmapHasData(tPixmap));

    renderer->render(&tPainter, TestRenderID);
    ASSERT_TRUE(testPixmapHasData(tPixmap));
    tPixmap.fill(Qt::green);
    ASSERT_FALSE(testPixmapHasData(tPixmap));
    ASSERT_TRUE(renderer->elementExists(TestRenderID));
    ASSERT_FALSE(renderer->elementExists(TestRenderID_NotExist));
    ASSERT_FALSE(renderer->boundsOnElement(TestRenderID).isEmpty());
    ASSERT_TRUE(renderer->boundsOnElement(TestRenderID_NotExist).isEmpty());

    renderer->render(&tPainter, {0, 0, TestPixmapSize, TestPixmapSize});
    ASSERT_TRUE(testPixmapHasData(tPixmap));
    tPixmap.fill(Qt::green);
    ASSERT_FALSE(testPixmapHasData(tPixmap));

    ASSERT_FALSE(renderer->toImage({TestPixmapSize, TestPixmapSize}).isNull());
    ASSERT_FALSE(renderer->toImage({TestPixmapSize, TestPixmapSize}, TestRenderID).isNull());
}
