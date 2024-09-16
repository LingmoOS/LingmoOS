// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "IconProvider.h"

#include <DGuiApplicationHelper>

#include <QImage>
#include <QPainter>

DGUI_USE_NAMESPACE

static const QSize DEFAULT_SIZE{ 20, 20 };

IconProvider::IconProvider()
    : QQuickImageProvider(QQuickImageProvider::Pixmap)
    , penColor_(getPenColor())
{
    connect(DGuiApplicationHelper::instance(),
            &DGuiApplicationHelper::themeTypeChanged,
            this,
            [this] {
                penColor_ = getPenColor();
            });
}

IconProvider::~IconProvider() = default;

QPixmap IconProvider::requestPixmap(const QString &icon, QSize *size, const QSize &requestedSize)
{
    auto s = requestedSize.width() > 0 && requestedSize.height() > 0 ? requestedSize : DEFAULT_SIZE;

    QImage image(s, QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    QPen pen = painter.pen();

    pen.setColor(penColor_);

    QFont font = painter.font();
    font.setBold(true);
    font.setPixelSize(18);

    painter.setPen(pen);
    painter.setFont(font);
    painter.drawText(image.rect(), Qt::AlignCenter, icon);

    QPixmap pixmap = QPixmap::fromImage(image);

    return pixmap;
}

QColor IconProvider::getPenColor()
{
    return DGuiApplicationHelper::instance()->themeType()
            == DGuiApplicationHelper::ColorType::LightType
        ? Qt::darkGray
        : Qt::lightGray;
}
