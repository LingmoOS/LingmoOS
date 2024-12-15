// Copyright (C) 2020 ~ 2022 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "borderprocessinterface.h"
#include "utils.h"
#include "configsettings.h"

#include <DFontSizeManager>
#include <dstyle.h>

#include <QBrush>

QT_BEGIN_NAMESPACE
// 参考DTK blurEffect 的实现
// qtbase-opensource-src/src/widgets/effects/qpixmapfilter.cpp:863
Q_WIDGETS_EXPORT extern void
qt_blurImage(QPainter *p, QImage &blurImage, qreal radius, bool quality, bool alphaOnly, int transposed = 0);
QT_END_NAMESPACE

BorderProcessInterface::BorderProcessInterface(QObject *parent)
    : QObject(parent)
{
}

BorderProcessInterface::~BorderProcessInterface() {}

void BorderProcessInterface::calculateBorderImageInfo(const QSize shotImageSize)
{
    m_svgImageSizeByshot = QSizeF(shotImageSize.width(), shotImageSize.height());
    m_svgCenterPoint = QPointF(0, 0);
    return;
}

ExternalBorderProcess::ExternalBorderProcess(QObject *parent)
    : BorderProcessInterface(parent)
{
}

ExternalBorderProcess::~ExternalBorderProcess() {}

void ExternalBorderProcess::initBorderInfo(const int borderConfig)
{
    m_borderType = borderConfig;

    QString svg = QString("imageBorder/border/externalBorder%1.svg").arg((borderConfig & 0xFF));
    m_svgRenderer.load(Utils::getQrcPath(svg));
    m_svgImageSize = m_svgRenderer.defaultSize();

    switch (borderConfig) {
        case BorderStyle_1: {
            m_svgCenterSize = QSizeF(143, 82);
            m_svgCenterPoint = QPointF(9, 10);
            break;
        }
        case BorderStyle_2: {
            m_svgCenterSize = QSizeF(144, 84);
            m_svgCenterPoint = QPointF(8, 8);
            break;
        }
        case BorderStyle_3: {
            m_svgCenterSize = QSizeF(145, 85);
            m_svgCenterPoint = QPointF(6, 6);
            break;
        }
        case BorderStyle_4: {
            m_svgCenterSize = m_svgImageSize;
            m_svgCenterPoint = QPointF(0, 0);
            break;
        }
        case BorderStyle_5: {
            m_svgCenterSize = QSizeF(157, 97);
            m_svgCenterPoint = QPointF(1.5, 1.5);
            break;
        }
        case BorderStyle_6: {
            m_svgCenterSize = QSizeF(156, 96);
            m_svgCenterPoint = QPointF(2, 2);
            break;
        }
        case BorderStyle_7: {
            m_svgCenterSize = QSizeF(147, 87);
            m_svgCenterPoint = QPointF(6.5, 6.5);
            break;
        }
        case BorderStyle_8: {
            m_svgCenterSize = m_svgImageSize;
            m_svgCenterPoint = QPointF(0, 0);
            break;
        }
    }
}

QPixmap ExternalBorderProcess::getPixmapAddBorder(const QPixmap &pix)
{
    QPixmap shotImage = pix;
    calculateBorderImageInfo(pix.size());
    getBorderImage(pix.size());
    if (m_borderType == BorderStyle_8 || m_borderType == BorderStyle_4) {
        shotImage = cropShotImage(pix);
    }
    QPixmap image(m_svgImage.width(), m_svgImage.height());
    image.fill(Qt::transparent);
    QPainter painter;
    painter.begin(&image);
    painter.drawPixmap(static_cast<int>(m_svgCenterPointByshot.x()),
                       static_cast<int>(m_svgCenterPointByshot.y()),
                       shotImage.width(),
                       shotImage.height(),
                       shotImage);

    painter.drawPixmap(0, 0, m_svgImage);
    // 添加绘制日期
    if (m_borderType == BorderStyle_5) {
        drawDateText(painter);
    }
    painter.end();

    if (m_borderType == BorderStyle_2) {
        return cropShotImageEx(image);
    }

    return image;
}

void ExternalBorderProcess::calculateBorderImageInfo(const QSize shotImageSize)
{
    qDebug() << __FUNCTION__ << __LINE__ << shotImageSize << m_svgImageSize << m_svgCenterSize << m_svgCenterPoint;
    // 截图原始宽高
    double shotImageW = shotImageSize.width();
    double shotImageH = shotImageSize.height();
    // shotImageW / shotImageH = m_svgCenterSize.width() / m_svgCenterSize.height();

    // 截图，透明区域 的宽高比
    double shotImageRatioWH = shotImageW / shotImageH;
    double centerRatioWH = m_svgCenterSize.width() / m_svgCenterSize.height();
    qDebug() << __FUNCTION__ << __LINE__ << shotImageRatioWH << centerRatioWH;

    double centerW = shotImageW;
    double centerH = shotImageH;
    if (shotImageRatioWH > centerRatioWH) {
        centerH = centerW / centerRatioWH;
        m_isHorizontalCrop = false;
    } else {
        centerW = centerH * centerRatioWH;
        m_isHorizontalCrop = true;
    }

    qDebug() << __FUNCTION__ << __LINE__ << centerW << centerH;
    // outImageSizeW / m_svgImageSize.width() = centerW / m_svgCenterSize.width();

    // 计算边框的缩放系数，理论上下面两个值相等才正确
    double ratioCenterw = centerW / m_svgCenterSize.width();
    double ratioCenterh = centerH / m_svgCenterSize.height();

    // 计算边框资源放大后尺寸（边框资源尺寸，与透明区域不一致）
    qDebug() << __FUNCTION__ << __LINE__ << ratioCenterw << ratioCenterh;
    double outImageSizeW = m_svgImageSize.width() * ratioCenterw;
    double outImageSizeH = m_svgImageSize.height() * ratioCenterh;

    m_svgImageSizeByshot = QSizeF(outImageSizeW, outImageSizeH);
    m_svgCenterPointByshot = QPointF(m_svgCenterPoint.x() * ratioCenterw, m_svgCenterPoint.y() * ratioCenterh);
    m_svgCenterSizeByshot = QSizeF(m_svgCenterSize.width() * ratioCenterw, m_svgCenterSize.height() * ratioCenterh);
    m_ratioCenter = ratioCenterh;

    return;
}

QPixmap ExternalBorderProcess::getBorderImage(const QSizeF shotImageSize)
{
    qDebug() << __FUNCTION__ << shotImageSize;
    QPixmap rimage(static_cast<int>(m_svgImageSizeByshot.width()), static_cast<int>(m_svgImageSizeByshot.height()));
    rimage.fill(Qt::transparent);
    QPainter painter(&rimage);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(rimage.rect(), Qt::transparent);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    m_svgRenderer.render(&painter, QRectF(QPointF(0, 0), m_svgImageSizeByshot));
    painter.end();

    if (m_isHorizontalCrop) {
        double wDiff = m_svgCenterSizeByshot.width() - shotImageSize.width();
        m_svgImage =
            QPixmap(static_cast<int>(m_svgImageSizeByshot.width() - wDiff), static_cast<int>(m_svgImageSizeByshot.height()));
        m_svgImage.fill(Qt::transparent);
        QPainter rp(&m_svgImage);
        rp.drawPixmap(0, 0, rimage.copy(0, 0, m_svgImage.width() / 2, rimage.height()));
        rp.drawPixmap(
            m_svgImage.width() / 2,
            0,
            rimage.copy(static_cast<int>((rimage.width() + wDiff) / 2), 0, m_svgImage.width() / 2, m_svgImage.height()));
        rp.end();
        return m_svgImage;
    } else {
        double hDiff = m_svgCenterSizeByshot.height() - shotImageSize.height();
        m_svgImage =
            QPixmap(static_cast<int>(m_svgImageSizeByshot.width()), static_cast<int>(m_svgImageSizeByshot.height() - hDiff));
        m_svgImage.fill(Qt::transparent);
        QPainter rp(&m_svgImage);
        rp.drawPixmap(0, 0, rimage.copy(0, 0, rimage.width(), m_svgImage.height() / 2));
        rp.drawPixmap(
            0,
            m_svgImage.height() / 2,
            rimage.copy(0, static_cast<int>((rimage.height() + hDiff) / 2), m_svgImage.width(), m_svgImage.height() / 2));
        rp.end();
        return m_svgImage;
    }
}

void ExternalBorderProcess::drawDateText(QPainter &painter) const
{
    QFont posFont(DFontSizeManager::instance()->t8());
    // 字体大小与缩放比正相关
    posFont.setPixelSize(static_cast<int>(5 * m_ratioCenter));
    painter.setFont(posFont);
    painter.setPen(QColor(Qt::white));

    QTextOption posTextOption;
    posTextOption.setAlignment(Qt::AlignLeft | Qt::AlignBottom);

    QString dateTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm");
    QFontMetricsF fm(posFont);
    QSizeF dateSize = fm.boundingRect(dateTime).size();

    // 绘制区域
    QRectF drawRect(m_svgImage.width() - dateSize.width() - 3.5 * m_ratioCenter,
                    m_svgImage.height() - dateSize.height() - 3 * m_ratioCenter,
                    dateSize.width() + 10,
                    dateSize.height());

    painter.drawText(drawRect, dateTime, posTextOption);
    return;
}
// 根据边框像素，裁剪截图
QPixmap ExternalBorderProcess::cropShotImage(QPixmap shotImage) const
{
    qDebug() << m_svgImage.size() << shotImage.size();
    uchar colorReset = 255;  // jpg，bmp格式不支持透明，边框用白色赋值
    int imageFormat = ConfigSettings::instance()->getValue("shot", "format").toInt();
    if (imageFormat == 0) {  // png 格式支持透明，边框像素值用 0 赋值。
        colorReset = 0;
    }

    QImage svg = m_svgImage.toImage();
    QImage shot = shotImage.toImage();
    uchar *svgData = svg.bits();
    uchar *shotData = shot.bits();
    for (int i = 0; i < m_svgImage.height(); ++i) {
        for (int j = 0; j < m_svgImage.width(); ++j) {
            uint32_t index = static_cast<uint32_t>(i * m_svgImage.width() * 4 + j * 4);
            uint8_t a = svgData[index + 0];
            uint8_t r = svgData[index + 1];
            uint8_t g = svgData[index + 2];
            uint8_t b = svgData[index + 3];
            shotData[index + 0] = shotData[index + 1] = shotData[index + 2] = shotData[index + 3] = colorReset;
            if ((a | r | g | b) != 0) {
                break;
            }
        }

        for (int j = m_svgImage.width() - 1; j >= 0; --j) {
            uint32_t index = static_cast<uint32_t>(i * m_svgImage.width() * 4 + j * 4);
            uint8_t a = svgData[index + 0];
            uint8_t r = svgData[index + 1];
            uint8_t g = svgData[index + 2];
            uint8_t b = svgData[index + 3];
            shotData[index + 0] = shotData[index + 1] = shotData[index + 2] = shotData[index + 3] = colorReset;
            if ((a | r | g | b) != 0) {
                break;
            }
        }
    }
    return QPixmap::fromImage(shot);
}

QPixmap ExternalBorderProcess::cropShotImageEx(QPixmap shotImage) const
{
    int imageFormat = ConfigSettings::instance()->getValue("shot", "format").toInt();
    if (imageFormat == 0) {  // png 格式支持透明，边框像素值用 0 赋值。
        return shotImage;
    }

    QImage shot = shotImage.toImage();
    uchar *shotData = shot.bits();
    uchar colorReset = 255;  // jpg，bmp格式不支持透明，边框用白色赋值
    for (int i = 0; i < m_svgImage.height(); ++i) {
        for (int j = 0; j < m_svgImage.width(); ++j) {
            uint32_t index = static_cast<uint32_t>(i * m_svgImage.width() * 4 + j * 4);
            uint8_t a = shotData[index + 0];
            uint8_t r = shotData[index + 1];
            uint8_t g = shotData[index + 2];
            uint8_t b = shotData[index + 3];
            shotData[index + 0] = shotData[index + 1] = shotData[index + 2] = shotData[index + 3] = colorReset;
            if ((a | r | g | b) != 0) {
                break;
            }
        }

        for (int j = m_svgImage.width() - 1; j >= 0; --j) {
            uint32_t index = static_cast<uint32_t>(i * m_svgImage.width() * 4 + j * 4);
            uint8_t a = shotData[index + 0];
            uint8_t r = shotData[index + 1];
            uint8_t g = shotData[index + 2];
            uint8_t b = shotData[index + 3];
            shotData[index + 0] = shotData[index + 1] = shotData[index + 2] = shotData[index + 3] = colorReset;
            if ((a | r | g | b) != 0) {
                break;
            }
        }
    }
    return QPixmap::fromImage(shot);
}

// 样机边框
PrototypeBorderProcess::PrototypeBorderProcess(QObject *parent)
    : BorderProcessInterface(parent)
{
}

PrototypeBorderProcess::~PrototypeBorderProcess() {}

void PrototypeBorderProcess::initBorderInfo(const int borderConfig)
{
    m_borderType = borderConfig;
    // pc notebook ipad cellphone
    if (borderConfig == BorderEffects_2) {
        QString svg = "imageBorder/border/notebook.svg";
        m_svgRenderer.load(Utils::getQrcPath(svg));
        m_svgImageSize = m_svgRenderer.defaultSize();
        m_svgCenterSize = QSizeF(111.997884, 71.5);
        m_svgCenterPoint = QPointF(23.7521159, 9.25);
    } else if (borderConfig == BorderEffects_4) {
        QString svg = "imageBorder/border/cellphone.svg";
        m_svgRenderer.load(Utils::getQrcPath(svg));
        m_svgImageSize = m_svgRenderer.defaultSize();
        m_svgCenterSize = QSizeF(47.33, 93.16);
        m_svgCenterPoint = QPointF(1.15, 1.58);
    } else if (borderConfig == BorderEffects_1) {
        QString svg = "imageBorder/border/pc.svg";
        m_svgRenderer.load(Utils::getQrcPath(svg));
        m_svgImageSize = m_svgRenderer.defaultSize();
        m_svgCenterSize = QSizeF(127, 70.82);
        m_svgCenterPoint = QPointF(16.5, 7.68);
    } else if (borderConfig == BorderEffects_3) {
        QString svg = "imageBorder/border/ipad.svg";
        m_svgRenderer.load(Utils::getQrcPath(svg));
        m_svgImageSize = m_svgRenderer.defaultSize();
        m_svgCenterSize = QSizeF(115.41, 84.84);
        m_svgCenterPoint = QPointF(22.49, 8.11);
    }
}

QPixmap PrototypeBorderProcess::getPixmapAddBorder(const QPixmap &pix)
{
    calculateBorderImageInfo(pix.size());
    // 整图
    QPixmap image(static_cast<int>(m_svgImageSizeByshot.width()), static_cast<int>(m_svgImageSizeByshot.height()));
    image.fill(Qt::transparent);
    // 样式边框，图片背景黑色填充
    QPixmap bImage(static_cast<int>(m_centerSize.width()), static_cast<int>(m_centerSize.height()));
    bImage.fill(Qt::black);

    QPainter painter;
    painter.begin(&image);
    // 绘制黑色背景背景
    painter.drawPixmap(static_cast<int>(m_centerPoint.x()), static_cast<int>(m_centerPoint.y()), bImage);
    // 绘制截图
    painter.drawPixmap(static_cast<int>(m_svgCenterPointByshot.x()),
                       static_cast<int>(m_svgCenterPointByshot.y()),
                       pix.width(),
                       pix.height(),
                       pix);
    // 绘制svg边框
    // m_svgRenderer.render(&painter, QRectF(QPointF(0, 0), m_svgImageSizeByshot));
    cropShotImage();  // 根据保存图片格式，jpg，bmp不支持透明度，将边框透明区域填充成白色。
    painter.drawPixmap(0, 0, m_svgImage);
    painter.end();

    return image;
}

void PrototypeBorderProcess::calculateBorderImageInfo(const QSize shotImageSize)
{
    //
    qDebug() << __FUNCTION__ << __LINE__ << shotImageSize << m_svgImageSize << m_svgCenterSize << m_svgCenterPoint;
    double shotImageW = shotImageSize.width();
    double shotImageH = shotImageSize.height();
    // shotImageW / shotImageH = m_svgCenterSize.width() / m_svgCenterSize.height();

    double shotImageRatioWH = shotImageW / shotImageH;
    double centerRatioWH = m_svgCenterSize.width() / m_svgCenterSize.height();
    qDebug() << __FUNCTION__ << __LINE__ << shotImageRatioWH << centerRatioWH;

    double centerW = shotImageW;
    double centerH = shotImageH;

    double offsetX = 0;
    double offsetY = 0;
    if (shotImageRatioWH > centerRatioWH) {
        centerH = centerW / centerRatioWH;
        offsetY = (centerH - shotImageH) / 2;
    } else {
        centerW = centerH * centerRatioWH;
        offsetX = (centerW - shotImageW) / 2;
    }

    qDebug() << __FUNCTION__ << __LINE__ << centerW << centerH;
    // outImageSizeW / m_svgImageSize.width() = centerW / m_svgCenterSize.width();
    double ratioCenterw = centerW / m_svgCenterSize.width();
    double ratioCenterh = centerH / m_svgCenterSize.height();

    qDebug() << __FUNCTION__ << __LINE__ << ratioCenterw << ratioCenterh;
    double outImageSizeW = m_svgImageSize.width() * ratioCenterw;
    double outImageSizeH = m_svgImageSize.height() * ratioCenterh;

    m_svgImageSizeByshot = QSizeF(outImageSizeW, outImageSizeH);
    m_svgCenterPointByshot =
        QPointF(m_svgCenterPoint.x() * ratioCenterw + offsetX, m_svgCenterPoint.y() * ratioCenterh + offsetY);

    m_centerSize.setWidth(m_svgCenterSize.width() * ratioCenterw);
    m_centerSize.setHeight(m_svgCenterSize.height() * ratioCenterh);
    m_centerPoint.setX(m_svgCenterPoint.x() * ratioCenterw);
    m_centerPoint.setY(m_svgCenterPoint.y() * ratioCenterh);
    return;
}

void PrototypeBorderProcess::cropShotImage()
{
    QPixmap rimage(static_cast<int>(m_svgImageSizeByshot.width()), static_cast<int>(m_svgImageSizeByshot.height()));
    rimage.fill(Qt::transparent);
    QPainter painter(&rimage);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(rimage.rect(), Qt::transparent);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    m_svgRenderer.render(&painter, QRectF(QPointF(0, 0), m_svgImageSizeByshot));
    painter.end();

    m_svgImage = rimage;
    int imageFormat = ConfigSettings::instance()->getValue("shot", "format").toInt();
    if (imageFormat == 0) {  // png 格式支持透明，边框像素值用 0 赋值。
        return;
    }

    uchar colorReset = 255;  // jpg，bmp格式不支持透明，边框用白色赋值
    QImage svg = m_svgImage.toImage();
    uchar *svgData = svg.bits();
    for (int i = 0; i < m_svgImage.height(); ++i) {
        for (int j = 0; j < m_svgImage.width(); ++j) {
            uint32_t index = static_cast<uint32_t>(i * m_svgImage.width() * 4 + j * 4);
            uint8_t a = svgData[index + 0];
            uint8_t r = svgData[index + 1];
            uint8_t g = svgData[index + 2];
            uint8_t b = svgData[index + 3];
            if ((a | r | g | b) != 0) {
                break;
            }
            svgData[index + 0] = svgData[index + 1] = svgData[index + 2] = svgData[index + 3] = colorReset;
        }

        for (int j = m_svgImage.width() - 1; j >= 0; --j) {
            uint32_t index = static_cast<uint32_t>(i * m_svgImage.width() * 4 + j * 4);
            uint8_t a = svgData[index + 0];
            uint8_t r = svgData[index + 1];
            uint8_t g = svgData[index + 2];
            uint8_t b = svgData[index + 3];
            if ((a | r | g | b) != 0) {
                break;
            }
            svgData[index + 0] = svgData[index + 1] = svgData[index + 2] = svgData[index + 3] = colorReset;
        }
    }

    // 样机边框1，纵向处理
    if (m_borderType != BorderEffects_1) {
        m_svgImage = QPixmap::fromImage(svg);
        return;
    }

    for (int i = 0; i < m_svgImage.width(); ++i) {
        for (int j = m_svgImage.height() - 1; j >= 0; --j) {
            uint32_t index = static_cast<uint32_t>(j * m_svgImage.width() * 4 + i * 4);
            uint8_t a = svgData[index + 0];
            uint8_t r = svgData[index + 1];
            uint8_t g = svgData[index + 2];
            uint8_t b = svgData[index + 3];
            if (((a | r | g | b) != 0) && (a != colorReset && r != colorReset && g != colorReset && b != colorReset)) {
                break;
            }
            svgData[index + 0] = svgData[index + 1] = svgData[index + 2] = svgData[index + 3] = colorReset;
        }
    }

    m_svgImage = QPixmap::fromImage(svg);
}

// 阴影边框
ShadowBorderProcess::ShadowBorderProcess(QObject *parent)
    : BorderProcessInterface(parent)
{
}

ShadowBorderProcess::~ShadowBorderProcess() {}

void ShadowBorderProcess::initBorderInfo(const int borderConfig)
{
    Q_UNUSED(borderConfig);
    return;
}

QPixmap ShadowBorderProcess::getPixmapAddBorder(const QPixmap &pix)
{
    // 调整阴影策略，阴影：内边框 border: 1px solid rgba(0, 0, 0, 0.20);
    //            压在图像边缘 box-shadow: 0 4px 12px 0 rgba(0, 0, 0, 0.50);

    // 模糊强度，和投影偏移量
    const qreal shadowRadius = 36;
    const int xOffset = 0;
    const int yOffset = 6;
    const QColor borderColor(0, 0, 0, 50);
    const QColor shadowColor(0, 0, 0, 70);

    // 获取当前缩放比例
    qreal radius = shadowRadius * pix.devicePixelRatioF();
    QRect generateRect = pix.rect().adjusted(0, 0, radius * 2, radius * 2);
    QRect imageRect = pix.rect();

    // 调整绘制区域
    if (xOffset < radius && yOffset < radius) {
        imageRect.moveTo(radius - xOffset, radius - yOffset);
    } else {
        if (xOffset > radius) {
            imageRect.setX(0);
            generateRect.setWidth(generateRect.width() + xOffset - radius);
        }

        if (yOffset > radius) {
            imageRect.setY(0);
            generateRect.setHeight(generateRect.height() + yOffset - radius);
        }
    }

    QPixmap newPixmap(generateRect.size());
    newPixmap.fill(Qt::transparent);

    QPainter painter;
    painter.setRenderHint(QPainter::Antialiasing);
    painter.begin(&newPixmap);

    painter.save();
    // 绘制投影 DTK提供的接口调整了绘制的透明度，不便直接使用,参考 QPixmapDropShadowFilter 实现
    drawShadow(&painter, generateRect, shadowColor, radius);
    painter.restore();

    // 绘制图片
    painter.drawPixmap(imageRect, pix);

    // 绘制边框 设置 0, 不同缩放率下均绘制1px
    painter.setPen(QPen(borderColor));
    painter.drawRect(imageRect.adjusted(0, 0, -1, -1));

    painter.end();
    return newPixmap;
}

/**
   @brief 根据传入的矩形 \a rect 绘制居中阴影，以模糊强度 \a radius 为边距(像素计算范围)居中，阴影基础颜色以 \a color 为准
 */
void ShadowBorderProcess::drawShadow(QPainter *p, const QRect &rect, const QColor &color, int radius)
{
    // 绘制区域必须大于像素模糊半径
    if (rect.width() < radius * 2 || rect.height() < radius * 2) {
        return;
    }

    // 用于计算的蒙蔽图片
    QImage tmp(rect.size(), QImage::Format_ARGB32_Premultiplied);
    tmp.fill(0);
    QPainter tmpPainter(&tmp);
    tmpPainter.setCompositionMode(QPainter::CompositionMode_Source);
    tmpPainter.setBrush(color);
    // 调整蒙版图片，居中
    QRect tmpRect = rect.adjusted(radius, radius, -radius, -radius);
    tmpPainter.drawRect(tmpRect);
    tmpPainter.end();

    // 在区域中绘制模糊背景
    p->save();
    qt_blurImage(p, tmp, radius, false, true);
    p->restore();
}
