// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CPICTUREITEM_H
#define CPICTUREITEM_H

#include <QGraphicsPixmapItem>
#include <QPainterPath>

#include "rectitem.h"

class ImageItem_Compatible : public RectBaseItem
{
public:

    explicit ImageItem_Compatible(const QPixmap &pixmap = QPixmap(),
                                  PageItem *parent = nullptr,
                                  const QByteArray &fileSrcData = QByteArray());

    explicit ImageItem_Compatible(const QRectF &rect,
                                  const QPixmap &pixmap,
                                  PageItem *parent = nullptr,
                                  const QByteArray &fileSrcData = QByteArray());
    ~ImageItem_Compatible() override;

    SAttrisList attributions() override;
    void  setAttributionVar(int attri, const QVariant &var, int phase) override;

    /**
     * @brief type 图元的类型(图片)
     */
    int  type() const override;

    /**
     * @brief 加载图元的信息
     */
    void loadUnit(const Unit &data) override;

    /**
     * @brief 获取图元的信息
     */
    Unit getUnit(int reson) const override;

    /**
     * @brief 设置图片
     */
    void setPixmap(const QPixmap &pixmap);

    /**
     * @brief pixmap
     */
    QPixmap &pixmap();

    /**
     * @brief 设置旋转角度
     */
    void setAngle(const qreal &angle);

    /**
     * @brief isPosPenetrable 某一位置在图元上是否是可穿透的（透明的）
     * @param posLocal 该图元坐标系的坐标位置
     */
    bool isPosPenetrable(const QPointF &posLocal) override;

    /**
     * @brief isPosPenetrable 某一矩形区域在图元上是否是可穿透的（透明的）
     * @param rectLocal 该图元坐标系的某一矩形区域
     */
    bool isRectPenetrable(const QRectF &rectLocal) override;

    /**
     * @brief getSelfOrgShape 获取到自身的原始形状
     */
    QPainterPath calOrgShapeBaseRect(const QRectF &rect) const override;

    /**
     * @brief move  操作结束
     */
    void operatingEnd(PageItemEvent *event) override;

    /**
     * @brief move  操作结束
     */
    void operatingBegin(PageItemEvent *event) override;

    /**
      * @brief updateShape  刷新图元的形状(属性变化时调用重新计算图元的形状和样式)
      */
    //void updateShape() override;

protected:

    /**
     * @brief paintCache 绘制自身的样貌
     */
    void paintSelf(QPainter *painter, const QStyleOptionGraphicsItem *option) override;

protected:
    QPixmap m_pixmap;
    qreal   m_angle;
    QByteArray _srcByteArry;

    QTransform _trans;
};
#endif // CPICTUREITEM_H
