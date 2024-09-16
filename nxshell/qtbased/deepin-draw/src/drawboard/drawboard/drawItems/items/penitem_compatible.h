// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CGRAPHICSPENITEM_H
#define CGRAPHICSPENITEM_H

#include "vectoritem.h"
#include "csizehandlerect.h"

#include <QGraphicsItem>
#include <QPainterPath>
class QPainterPath;
class PenItem_Compatible : public VectorItem
{
public:
    explicit PenItem_Compatible(PageItem *parent = nullptr);
    explicit PenItem_Compatible(const QPointF &startPoint, PageItem *parent = nullptr);
    ~PenItem_Compatible() override;

    SAttrisList attributions() override;
    void  setAttributionVar(int attri, const QVariant &var, int phase) override;

    /**
     * @brief rect 重写实现画笔的矩形
     * @return
     */
    //QRectF itemRect() const override;

    /**
     * @brief rect 重写实现画笔的类型
     * @return
     */
    int type() const override;

    void doScaling(PageItemScalEvent *event) override;

    bool testScaling(PageItemScalEvent *event) override;

    /**
     * @brief getGraphicsUnit 获取图元的信息
     * @return
     */
    Unit getUnit(int reson) const override;

    /**
     * @brief updatePenPath 刷新画笔的路径信息（使用场景：toolCreatItemUpdate 创建该item时 刷新路径）
     * @return
     */
    void updatePenPath(const QPointF &endPoint, bool isShiftPress);

    /**
     * @brief drawComplete toolCreatItemFinished 绘制完成时调用
     * @return
     */
    void drawComplete(bool doBz = false);

    /**
     * @brief setPath 设置路径
     * @return
     */
    void setPath(const QPainterPath &path);

    /**
     * @brief getPath 获取路径
     * @return
     */
    QPainterPath getPath() const;

//    /**
//     * @brief setPenStartpath 设置起点样式
//     * @return
//     */
//    void setPenStartpath(const QPainterPath &path);

    /**
     * @brief getPenStartpath 返回起点样式
     * @return
     */
    QPainterPath getPenStartpath() const;

//    /**
//     * @brief setPenEndpath 设置终点样式
//     * @return
//     */
//    void setPenEndpath(const QPainterPath &path);

    /**
     * @brief getPenEndpath 获取终点样式
     * @return
     */
    QPainterPath getPenEndpath() const;

    /**
     * @brief setDrawFlag true表示正在绘制，false表示没有
     * @return
     */
    void setDrawFlag(bool flag);

    /**
     * @brief calcVertexes 根据路径计算绘制路径
     * @return
     */
    void calcVertexes();

    /**
     * @brief getHighLightPath 获取高亮path
     * @return
     */
    virtual QPainterPath highLightPath() const override;

    /**
     * @brief getPenStartType 得到起点的样式
     * @return
     */
    ELinePosStyle getPenStartType() const;

    /**
     * @brief setPenStartType 设置起点的样式
     * @return
     */
    void setPenStartType(const ELinePosStyle &penType);

    /**
     * @brief getPenEndType 得到终点的样式
     * @return
     */
    ELinePosStyle getPenEndType() const;

    /**
     * @brief setPenEndType 设置终点的样式
     * @return
     */
    void setPenEndType(const ELinePosStyle &penType);

// Cppcheck检测函数没有使用到
//    /**
//     * @brief straightLine 得到绘制过程中的可能存在的直线线条
//     * @return
//     */
//    QLineF straightLine();

    /**
     * @brief loadGraphicsUnit 加载图元数据
     * @return
     */
    void loadUnit(const Unit &data) override;

protected:
    /**
     * @brief inSideShape 重写实现画笔的图元原始形状（图元不包括边线）
     */
    QPainterPath getOrgShape() const override;

    /**
     * @brief updateHandlesGeometry 刷新孩子节点
     */
    //void updateHandlesGeometry() override;

    /**
     * @brief updateShape 刷新图元形状
     */
    void updateShape() override;

    /**
     * @brief paintCache 绘制换冲图(如果启动了缓冲图，记得重载该函数描述位图的样貌)
     */
    void paintSelf(QPainter *painter, const QStyleOptionGraphicsItem *option) override;

    /**
     * @brief isPosPenetrable  是否可以进行穿透
     */
    virtual bool isPosPenetrable(const QPointF &posLocal) override;

protected:
    QLineF m_straightLine;
    bool m_isShiftPress;

private:
    QPainterPath m_path;
    QVector<QPointF> m_smoothVector;
    QPixmap m_tmpPix;
    bool m_isDrawing;//是否正在绘图
    int m_drawIndex;
    QPointF m_point4;

    // 画笔类型
    ELinePosStyle m_penStartType; // 画笔起点样式
    ELinePosStyle m_penEndType; // 画笔终点样式
    QPainterPath m_startPath;
    QPainterPath m_endPath;
    bool m_isStartWithLine = false; // 用于判断是否是开始以画直线为起点
    bool m_isEndWithLine = false; // 用于判断是否是结束以画直线为起点

private:
    /**
     * @brief calcVertexes 计算箭头三角形的三个点
     * @param prePoint 前一个点
     * @param currentPoint 当前点
     */
    void calcVertexes(const QPointF &prePoint, const QPointF &currentPoint);

    qreal GetBezierValue(qreal p0, qreal p1, qreal p2, qreal p3, qreal p4, qreal p5, qreal t);
    QPointF GetBezierValue(QPainterPath::Element p0, QPainterPath::Element p1, QPainterPath::Element p2, QPainterPath::Element p3, QPainterPath::Element p4, QPainterPath::Element p5, qreal t);

    void updateStartPathStyle();
    void updateEndPathStyle();
};
#endif // CGRAPHICSPENITEM_H
