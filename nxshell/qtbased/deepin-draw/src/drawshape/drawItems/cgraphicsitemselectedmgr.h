// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CGRAPHICSITEMGROUP_H
#define CGRAPHICSITEMGROUP_H

#include "csizehandlerect.h"
#include "cgraphicsitem.h"
#include <QGraphicsItemGroup>
#include <QPainterPath>

class CGraphicsPenItem;

/**
 * @brief The CGraphicsItemGroup class 组合图元管理类
 * 所有的图元操作都通过该类执行。
 */
class CGraphicsItemGroup : public QObject, public CGraphicsItem
{
    Q_OBJECT

public:
    /**
     * @brief EGroupType 描述组合类型的枚举值
     * ENormalGroup 普通的组合; ESelectGroup 用于管理选择情况的组合; EVirRootGroup用于临时作为组合树的根组合(如场景内以树结构表示其组合情况);
     */
    enum EGroupType {ENormalGroup, ESelectGroup, EVirRootGroup, EGroupTypeCount};

    ~CGraphicsItemGroup();

    /**
     * @brief CGraphicsItemGroup 构造函数
     */
    explicit CGraphicsItemGroup(EGroupType tp = ENormalGroup, const QString &nam = "");

    DrawAttribution::SAttrisList attributions() override;
    void  setAttributionVar(int attri, const QVariant &var, int phase) override;

    /**
     * @brief name 组合的名字
     */
    QString name() const;

    /**
     * @brief setName 设置组合的名字
     */
    void setName(const QString &name);

    /**
     * @brief type 描述组合的类型
     */
    EGroupType groupType() const;

    /**
     * @brief type 设置组合的类型
     */
    void  setGroupType(EGroupType tp);

    /**
     * @brief isTopBzGroup 是否是顶层的一个组合
     */
    bool  isTopBzGroup() const;

    /**
     * @brief type 设置组合是否可取消的
     */
    void setCancelable(bool enable);

    /**
     * @brief isCancelable 组合是否可取消的
     */
    bool isCancelable() const;

    /**
     * @brief add 添加图元到多选
     * @param item
     */
    void add(CGraphicsItem *item, bool updateAttri = true, bool updateRect = true);

    /**
     * @brief removeFromGroup 从多选列表中删除图元
     * @param item
     */
    void remove(CGraphicsItem *item, bool updateAttri = true, bool updateRect = true);

    /**
     * @brief clear 清除组合
     */
    void clear();

    /**
     * @brief updateAttributes 刷新属性显示
     */
    void updateAttributes(bool showTitle = false);

    /**
     * @brief setHandleVisible 刷新属性显示
     */
    void setHandleVisible(bool visble, CSizeHandleRect::EDirection dirHandle = CSizeHandleRect::InRect);

    /**
     * @brief boundingRect 边界矩形
     * @return
     */
    //QRectF boundingRect() const override;

    /**
     * @brief updateShape 刷新形状
     * @return
     */
    void updateShape() override;

    /**
     * @brief updateBoundingRect 刷新大小矩形
     * @return
     */
    void updateBoundingRect(bool force = false);

    /**
     * @brief shape 图元形状
     * @return
     */
    QPainterPath getSelfOrgShape() const override;

    /**
     * @brief incLength 虚拟的额外线宽宽度（解决选中困难的问题 提升用户体验）
     * @return 返回额外的线宽（一般与当前的放大值相关）
     */
    qreal incLength() const override;

    /**
     * @brief count 组合的业务图元个数
     * @return
     */
    int count();

    int allCount()const;

    /**
     * @brief items 获取组合中的图元
     * @param recursiveFind 是否递归循环子组合找到所有的图元,为true表示递归返回所有图元,false返回子图元
     * @return 返回找到的图元
     */
    QList<CGraphicsItem *> items(bool recursiveFind = false) const;

    /**
     * @brief getItems 获取组合中的基本业务图元
     * @param recursiveFind 是否递归循环子组合找到所有的基本业务图元,为true表示递归返回所有的基本业务图元,false返回子基本业务图元
     * @return 返回找到的图元
     */
    QList<CGraphicsItem *> getBzItems(bool recursiveFind = false) const;

    /**
     * @brief getGroups 获取组合中的组合
     * @param recursiveFind 是否递归循环找到所有的组合,为true表示递归返回所有的组合,false只返回子组合
     * @return 返回找到的组合
     */
    QList<CGraphicsItemGroup *> getGroups(bool recursiveFind = false) const;

    /**
     * @brief drawZValue 图元的z值(一般等于zValue(),但组合图元的z值应该和组合中z值最小的图元相等)
     */
    qreal drawZValue() override;

    /**
     * @brief markZDirty 使当前z值脏,以便之后刷新
     */
    inline void markZDirty() {_zIsDirty = true;}

    /**
     * @brief updateZValue 刷新图元的z值
     */
    void updateZValue();

    /**
     * @brief move  移动图元
     * @param beginPoint 移动起始点
     * @param movePoint 移动终点
     */
    void move(QPointF beginPoint, QPointF movePoint) override;

    /**
     * @brief type 返回图元类型
     * @return
     */
    int type() const override;

    /**
     * @brief getCenter 获取中心点
     */
    QPointF getCenter(CSizeHandleRect::EDirection dir) override;

    /**
     * @brief move  操作开始
     */
    void operatingBegin(CGraphItemEvent *event) override;

    /**
     * @brief operating  执行操作
     */
    void operating(CGraphItemEvent *event) override;

    /**
     * @brief testOpetating  测试该操作
     */
    bool testOpetating(CGraphItemEvent *event) override;

    /**
     * @brief operatingEnd  操作结束
     */
    void operatingEnd(CGraphItemEvent *event) override;

    /**
     * @brief testScaling  测试缩放操作
     */
    bool testScaling(CGraphItemScalEvent *event) override;

    /**
     * @brief doScaling  执行缩放操作
     */
    void doScaling(CGraphItemScalEvent *event) override;

    /**
     * @brief doScaling  执行缩放操作结束
     */
    void doScalEnd(CGraphItemScalEvent *event) override;

    /**
     * @brief rect 图元的基础矩形大小
     * @return
     */
    QRectF rect() const override;

    /**
     * @brief loadGraphicsUnit 加载图元数据
     * @return
     */
    void loadGraphicsUnit(const CGraphicsUnit &data) override;

    /**
     * @brief getGraphicsUnit 获取图元数据
     */
    CGraphicsUnit getGraphicsUnit(EDataReason reson) const override;

    /**
     * @brief setNoContent  设置是否有内容
     */
    void setNoContent(bool b, bool children = true);

    /**
     * @brief setRecursiveScene  将组合内的所有图元的场景设置为scene
     */
    void setRecursiveScene(PageScene *scene);

    /**
     * @brief getMinZ 图元的Z值
     */
    qreal getMinZ() const;

    /**
     * @brief minZItem Z值最小的图元
     */
    CGraphicsItem *minZItem() const;

    /**
     * @brief getLogicFirst 获取逻辑上第一个添加到组合中的图元
     */
    CGraphicsItem *getLogicFirst() const;

    /**
     * @brief EAddType 描述了图元是通过什么样的方式添加到组合的
     */
    enum EAddType {EOneByOne, ERectSelect};

    /**
     * @brief setAddType 自定义设置图元是通过什么样的方式添加到组合的
     */
    void setAddType(EAddType tp);

    /**
     * @brief addType 描述图元是通过什么样的方式添加到组合的
     */
    EAddType addType() const;

    /**
     * @brief setRect 设置矩形的大小
     */
    virtual void setRect(const QRectF rct);


    void rasterToSelfLayer(bool deleteSelf = true) override;
signals:
    /**
     * @brief childrenChanged 组合内孩子图元发生了变化的信号
     */
    void childrenChanged(const QList<CGraphicsItem * > &children);

protected:
    /**
     * @brief paint 绘制函数
     * @param painter
     * @param option
     * @param widget
     */
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void paintSelf(QPainter *painter, const QStyleOptionGraphicsItem *option) override;


    virtual void itemAdded(CGraphicsItem *item) {Q_UNUSED(item)}
    virtual void itemRemoved(CGraphicsItem *item) {Q_UNUSED(item)}

private:
    /**
     * @brief initRect 初始化矩形的属性和边框小方块
     */

    void initHandle() override;

    /**
     * @brief updateHandlesGeometry 刷新节点位置
     */
    void updateHandlesGeometry() override;

private:
    EGroupType _type = ENormalGroup;

    QList<CGraphicsItem * > m_listItems;

    bool   _isCancelable = true;

    int    _indexForTest = 0;

    EAddType _addTp = EOneByOne;

    QString _name;

    bool  _zIsDirty = true;
protected:
    qreal _maxZ;
};


#endif // CGRAPHICSITEMGROUP_H
