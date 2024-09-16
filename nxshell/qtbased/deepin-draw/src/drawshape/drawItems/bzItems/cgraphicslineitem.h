// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CGRAPHICSLINEITEM_H
#define CGRAPHICSLINEITEM_H
#include "cgraphicsitem.h"
#include "csizehandlerect.h"
#include <QGraphicsItem>
#include <QPainterPath>
#include <QLineF>
class CGraphicsLineItem : public CGraphicsItem
{
public:
    explicit CGraphicsLineItem(QGraphicsItem *parent = nullptr);
    explicit CGraphicsLineItem(const QLineF &line, QGraphicsItem *parent = nullptr);
    explicit CGraphicsLineItem(const QPointF &p1, const QPointF &p2, QGraphicsItem *parent = nullptr);
    explicit CGraphicsLineItem(qreal x1, qreal y1, qreal x2, qreal y2, QGraphicsItem *parent = nullptr);
    ~CGraphicsLineItem() override;

    DrawAttribution::SAttrisList attributions() override;
    void  setAttributionVar(int attri, const QVariant &var, int phase) override;

    /**
     * @brief rect 基于一个矩形范围的图元，重写实现该图元的矩形范围
     */
    QRectF rect() const override;

    /**
     * @brief type 图元类型
     */
    int type() const override;

    /**
     * @brief doChange 根据图元事件对自身进行相应的变化
     */
    void doScaling(CGraphItemScalEvent *event) override;

    bool testScaling(CGraphItemScalEvent *event) override;

    /**
     * @brief line  返回线条
     */
    QLineF line() const;

    /**
     * @brief setLine  设置直线条
     */
    void setLine(const QLineF &line, bool init = false);
    /**
     * @brief setLine  设置直线条
     */
    void setLine(const QPointF &p1, const QPointF &p2, bool init = false);

    /**
     * @brief setLine  设置直线条
     */
    inline void setLine(qreal x1, qreal y1, qreal x2, qreal y2, bool init = false);

    /**
     * @brief getGraphicsUnit  返回图元的信息
     */
    CGraphicsUnit getGraphicsUnit(EDataReason reson) const override;
    void loadGraphicsUnit(const CGraphicsUnit &data) override;

    /**
     * @brief setLineStartType  设置线开始点的样式
     */
    void setLineStartType(ELineType type);

    /**
     * @brief getLineStartType  返回当前线开始点的样式
     */
    ELineType getLineStartType() const;

    /**
     * @brief setLineEndType  设置线结束点的样式
     */
    void setLineEndType(ELineType type);

    /**
     * @brief getLineEndType  返回当前线结束点的样式
     */
    ELineType getLineEndType() const;

    /**
     * @brief calcVertexes  计算箭头
     */
    void calcVertexes();

    /**
     * @brief getHighLightPath 获取高亮path
     * @return
     */
    QPainterPath getHighLightPath() override;

protected:
    /**
     * @brief getSelfOrgShape 重写实现画笔的图元内部形状（rect类图元不包括边线）
     */
    QPainterPath getSelfOrgShape() const override;

    /**
     * @brief updateHandlesGeometry 刷新子节点位置
     * @return
     */
    void updateHandlesGeometry() override;

    /**
     * @brief updateShape 刷新图元形状
     * @return
     */
    void updateShape() override;

    /**
     * @brief paint 绘制图元
     * @return
     */
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;

    /**
     * @brief itemChange 图元变更
     * @param change 变更属性
     * @param value 变更的值
     * @return
     */
    QVariant itemChange(GraphicsItemChange doChange, const QVariant &value) override;

    /**
     * @brief isPosPenetrable  是否可以进行穿透
     */
    virtual bool isPosPenetrable(const QPointF &posLocal) override;

private:
    void initLine();

    void initHandle() override;

    void drawStart();

    void drawEnd();

private:
    QLineF m_line; // 中间直线的位置信息
    QLineF m_dRectline; // 矩形直线的位置信息
    ELineType m_startType; // 起始点样式
    ELineType m_endType; // 终点样式

    QPainterPath m_startPath; // 绘制起点路径
    QPainterPath m_endPath; // 绘制终点路径
    CSizeHandleRect     *leftTop;
    CSizeHandleRect     *rightBottom;
};

#endif // CGRAPHICSLINEITEM_H
