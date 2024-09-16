// Copyright (C) 2020 ~ 2022 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BORDERPROCESSINTERFACE_H
#define BORDERPROCESSINTERFACE_H

#include <QObject>
#include <QPixmap>
#include <QtSvg>
class BorderProcessInterface : public QObject
{
    Q_OBJECT
public:
    explicit BorderProcessInterface(QObject *parent = nullptr);
    virtual ~BorderProcessInterface();

    virtual void initBorderInfo(const int borderConfig) = 0;

    virtual QPixmap getPixmapAddBorder(const QPixmap &pix) = 0;

    virtual void calculateBorderImageInfo(const QSize shotImageSize);
protected:
    QSizeF m_svgImageSize; // svg 边框svg资源图片大小
    QSizeF m_svgImageSizeByshot;  // svg 根据截图缩放后的大小
    QSizeF m_svgCenterSize; // 边框svg图片资源，中心透明矩形大小，（透明矩形，用于透视截图）
    QSizeF m_svgCenterSizeByshot;
    QPointF m_svgCenterPoint; // 边框svg图片资源，中心透明矩形坐标。
    QPointF m_svgCenterPointByshot;
    QSvgRenderer m_svgRenderer;
};

// 外边框
// 根据截图的宽高比，等比放大边框，然后将边框资源进行裁剪
// 边框资源裁剪，裁剪中间区域（边框的中间区域都是一致的）
class ExternalBorderProcess : public BorderProcessInterface
{
    enum BorderStyle {
        Nothing = 0,
        BorderStyle_1,
        BorderStyle_2,
        BorderStyle_3,
        BorderStyle_4,
        BorderStyle_5,
        BorderStyle_6,
        BorderStyle_7,
        BorderStyle_8,
        BorderStyle_9,
    };
public:
    explicit ExternalBorderProcess(QObject *parent = nullptr);
    virtual ~ExternalBorderProcess() override;

    virtual void initBorderInfo(const int borderConfig) override;

    virtual QPixmap getPixmapAddBorder(const QPixmap &pix) override;

    virtual void calculateBorderImageInfo(const QSize shotImageSize) override;
private:
    QPixmap getBorderImage(const QSizeF shotImageSize);

    void drawDateText(QPainter &painter) const;

    QPixmap cropShotImage(QPixmap shotImage) const;

    QPixmap cropShotImageEx(QPixmap shotImage) const;
private:
    double m_ratioCenter = 0; // 缩放比
    bool m_isHorizontalCrop = false; // 标志纵向还是横向扩展
    QPixmap m_svgImage;
    int m_borderType = 0; // 边框类型，特殊边框需要绘制日期
};


// 样机
// 根据样机边框中，透明区域的宽高比，进行缩放。
// 截图左右，或上下侧填充黑色来达到边框中的宽高比
class PrototypeBorderProcess : public BorderProcessInterface
{
    enum BorderEffects {
        Nothing = 0,
        BorderEffects_1,
        BorderEffects_2,
        BorderEffects_3,
        BorderEffects_4,
    };
public:
    explicit PrototypeBorderProcess(QObject *parent = nullptr);
    virtual ~PrototypeBorderProcess() override;

    virtual void initBorderInfo(const int borderConfig) override;

    virtual QPixmap getPixmapAddBorder(const QPixmap &pix) override;

    virtual void calculateBorderImageInfo(const QSize shotImageSize) override;

    void cropShotImage();
private:
    QSizeF m_centerSize; // 等比放大后中心区域大小，添加黑色背景。
    QPointF m_centerPoint;
    QPixmap m_svgImage;
    int m_borderType = 0; // 边框类型
};



// 投影
class ShadowBorderProcess : public BorderProcessInterface
{
public:
    explicit ShadowBorderProcess(QObject *parent = nullptr);
    virtual ~ShadowBorderProcess() override;

    virtual void initBorderInfo(const int borderConfig) override;

    virtual QPixmap getPixmapAddBorder(const QPixmap &pix) override;

private:
    void drawShadow(QPainter *p, const QRect &rect, const QColor &color, int radius);
};

#endif // BORDERPROCESSINTERFACE_H


















