// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CGRAPHICSRECTITEM_H
#define CGRAPHICSRECTITEM_H
#include "csizehandlerect.h"
#include "cgraphicsitem.h"
#include "cdrawparamsigleton.h"
#include <QtGlobal>
#include <QPainterPath>
#include <QVector>
#include <QRectF>
#include <QGraphicsBlurEffect>

class CSizeHandleRect;
class CGraphicsRectItem : public CGraphicsItem
{
public:
    explicit CGraphicsRectItem(CGraphicsItem *parent = nullptr);
    explicit CGraphicsRectItem(const QRectF &rect, CGraphicsItem *parent = nullptr);
    explicit CGraphicsRectItem(qreal x, qreal y, qreal w, qreal h, CGraphicsItem *parent = nullptr);
    ~CGraphicsRectItem()  override;

    DrawAttribution::SAttrisList attributions() override;
    void  setAttributionVar(int attri, const QVariant &var, int phase) override;

    /**
     * @brief type 图元的类型
     * @return
     */
    int type() const override;

    /**
     * @brief setRect 创建矩形时，用于设置矩形大小
     * @param rect
     */
    virtual void setRect(const QRectF &rect);

    /**
     * @brief rect 矩形的大小
     * @param rect
     */
    QRectF rect() const override;

    /**
     * @brief operating  执行操作
     */
    //void operating(CGraphItemEvent *event) override;

    void doScaling(CGraphItemScalEvent *event) override;

    bool testScaling(CGraphItemScalEvent *event) override;

    /**
     * @brief loadGraphicsUnit 加载图元数据
     * @return
     */
    void loadGraphicsUnit(const CGraphicsUnit &data) override;

    /**
     * @brief getGraphicsUnit 得到图元的数据
     * @return
     */
    CGraphicsUnit getGraphicsUnit(EDataReason reson) const override;

    /**
     * @brief setXYRedius 设置矩形圆角半径
     * @param xRedius x方向半径
     * @param yRedius y方向半径
     */
    void setXYRedius(int xRedius, int yRedius, bool preview = false);

    /**
     * @brief getXYRedius 默认返回一个，返回圆角半径
     * @return
     */
    int getXRedius();

    /**
     * @brief getXYRedius 默认返回一个，返回圆角半径
     * @return
     */
//    int getPaintRedius();



protected:
    void loadGraphicsRectUnit(const SGraphicsRectUnitData &rectData);

protected:
    /**
     * @brief paintCache 绘制自身的样貌
     */
    void paintSelf(QPainter *painter, const QStyleOptionGraphicsItem *option) override;

    /**
     * @brief inSideShape 图元内部形状（rect类图元不包括边线）
     */
    QPainterPath getSelfOrgShape() const override;

    /**
     * @brief incLength 虚拟的额外线宽宽度（解决选中困难的问题 提升用户体验,但闭合形状很容易选中，所以返回0）
     * @return 返回额外的线宽（0）
     */
    qreal incLength() const override;

private:
    QPointF m_topLeftPoint; //左上角的点
    QPointF m_bottomRightPoint; //右下角的点
    int m_xRedius = 0;
    int m_yRedius = 0;

    int m_rediusForPreview = 5;
    bool m_isPreviewRedius = false;


};

#endif // CGRAPHICSRECTITEM_H
