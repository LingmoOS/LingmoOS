// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CGRAPHICSITEM_H
#define CGRAPHICSITEM_H

#include "csizehandlerect.h"
#include "globaldefine.h"
#include "sitemdata.h"
#include "cdrawparamsigleton.h"
#include "cdrawtoolmanagersigleton.h"
#include "idrawtool.h"
#include "dyncreatobject.h"

#include <QAbstractGraphicsShapeItem>
#include <QPainterPath>
#include <QCursor>
#include <QStyleOptionGraphicsItem>

class PageView;

class PageScene;

class CGraphicsItemGroup;
class CGraphicsLayer;

class CGraphItemEvent;
class CGraphItemMoveEvent;
class CGraphItemScalEvent;
class CGraphItemRotEvent;

class RegistItemHelper
{
public:
    RegistItemHelper(const QString &itemname, int type);
};

#define REGISTITEM_DECLARE(classname,type)\
    REGISTCLASS_DECLARE(classname)\
    class classname##RegistItemHelper1{static RegistItemHelper helper;};

#define REGISTITEM_DEFINE(classname,type)\
    REGISTCLASS_DEFINE(classname)\
    RegistItemHelper classname##RegistItemHelper1::helper = RegistItemHelper(#classname,type);

#define REGISTITEMCLASS(classname,type)\
    REGISTITEM_DECLARE(classname,type) \
    REGISTITEM_DEFINE(classname,type)

class CGraphicsItem : public QAbstractGraphicsShapeItem
{
public:
    enum EFilpDirect {EFilpHor, EFilpVer};
    /**
     * @brief CGraphicsItem 构造函数
     * @param parent 父图元
     */
    explicit CGraphicsItem(QGraphicsItem *parent);

    ~CGraphicsItem() override;

    static void registerItem(const QString &classname, int classType);
    static CGraphicsItem *creatItemInstance(int itemType, const CGraphicsUnit &data = CGraphicsUnit());

    /**
     * @brief setScene 设置图元所处的场景
     * @param scene 场景指针
     */
    void setScene(QGraphicsScene *scene, bool calZ = true);


    virtual DrawAttribution::SAttrisList attributions();
    virtual void  setAttributionVar(int attri, const QVariant &var, int phase);


    enum {Type = UserType};

    /**
     * @brief curView 返回当前所处的view
     * @return
     */
    PageView *curView()const;

    /**
     * @brief drawScene 返回当前所处的scene
     * @return
     */
    PageScene *drawScene() const ;


    Page *page()const;

    /**
     * @brief setPenColor 设置线颜色
     * @return
     */
    void setPenColor(const QColor &c, bool isPreview = false);

    /**
     * @brief setPenWidth 设置线颜色
     * @return
     */
    void setPenWidth(int w, bool isPreview = false);

    /**
     * @brief setBrushColor 设置线颜色
     * @return
     */
    void setBrushColor(const QColor &c, bool isPreview = false);

    /**
     * @brief paintBrush 填充信息
     * @return
     */
    QBrush paintBrush() const;

    /**
     * @brief setBrushColor 线条信息
     * @return
     */
    QPen paintPen(Qt::PenJoinStyle style = Qt::/*RoundJoin*/MiterJoin) const;


    QColor systemThemeColor()const;

    /**
     * @brief rect 基于一个矩形范围的图元，所以必须实现该虚函数
     */
    virtual QRectF rect() const = 0;

    /**
     * @brief boundingRect 自身坐标系的包围矩形
     * @return
     */
    QRectF boundingRect() const override;



    virtual QRectF selectedBoundingRect() const ;

    /**
     * @brief boundingRectTruly 自身坐标系的真实显示的包围矩形
     * @return
     */
    QRectF boundingRectTruly() const;

    /**
     * @brief shape 返回图元的形状
     */
    QPainterPath shape() const override;

    /**
     * @brief setCache 设置是否启动缓冲图加速绘制
     */
    void setCache(bool enable);

    /**
     * @brief isCached 是否当前是缓冲绘制激活的
     */
    bool isCached() const;

    /**
     * @brief setAutoCache 设置是否自动根据绘制耗时启动缓冲图加速绘制
     */
    void setAutoCache(bool autoCache, int autoCacheMs = 8);

// Cppcheck检测函数没有使用到
//    /**
//     * @brief isAutoCache 是否自动激活缓冲绘制
//     */
//    bool isAutoCache() const;

    /**
     * @brief shape 返回图元的原始形状
     */
    QPainterPath selfOrgShape() const;

    /**
     * @brief shape 返回图元的线条轮廓形状
     */
    QPainterPath penStrokerShape() const ;

    /**
     * @brief getCenter 返回图元的线条轮廓形状
     */
    virtual QPointF getCenter(CSizeHandleRect::EDirection dir);

    /**
     * @brief layer 返回图元所处的图层
     */
    CGraphicsLayer *layer();

    void setLayer(CGraphicsLayer *layer);

    /**
     * @brief isBzGroup 是否是一个组合图元
     * @param 如果返回值为true groupTp才有意义,返回具体的组合的类型
     */
    bool isBzGroup(int *groupTp = nullptr) const;

    /**
     * @brief bzGroup 返回当前所处的组合图元(onlyNormal为true不包括选择管理group)
     */
    CGraphicsItemGroup *bzGroup(bool onlyNormal = true) const;

    /**
     * @brief bzGroup 返回最顶层的组合图元(onlyNormal为true不包括选择管理group)
     */
    CGraphicsItemGroup *bzTopGroup(bool onlyNormal = true) const;

    /**
     * @brief thisBzProxyItem 图元的代管指针(当不处于组合时返回自身,当处于组合时返回组合图元)
     * @param topleve 为true表示返回顶层的组合指针,否则直接返回所处的组合
     */
    CGraphicsItem *thisBzProxyItem(bool topleve = true);

    /**
     * @brief setBzGroup 设置组合图元 (将自身添加到一个组合图元中去)
     */
    void setBzGroup(CGraphicsItemGroup *pGroup);

    /**
     * @brief drawZValue 图元的z值(一般等于zValue(),但组合图元的z值应该和组合中z值最小的图元相等)
     */
    virtual qreal drawZValue();

    /**
     * @brief contains 点是否在图元中（重载实现更好选中，增加用户体验）
     * @param point在图元本地坐标系的坐标值
     */
    bool contains(const QPointF &point) const override;

    /**
     * @brief loadGraphicsUnit 加载图元数据
     * @return
     */
    virtual void loadGraphicsUnit(const CGraphicsUnit &data);

    /**
     * @brief getGraphicsUnit 获取图元数据
     * @return
     */
    virtual CGraphicsUnit getGraphicsUnit(EDataReason reson) const;

    /**
     * @brief type 返回当前图元类型
     * @return
     */
    virtual int  type() const override;

    /**
     * @brief isBzItem 是否是业务图元(不包括多选图元)
     * @return
     */
    bool isBzItem() const;

    /**
     * @brief isSizeHandleExisted 是否自身存在resize节点
     * @return
     */
    bool isSizeHandleExisted();

    /**
     * @brief doFilp 在当前基础上翻转一下
     * @return
     */
    virtual void doFilp(EFilpDirect dir = EFilpHor);

    /**
     * @brief setFilpBaseOrg 设置在初始图像上是否翻转,否则和原图一致
     */
    void setFilpBaseOrg(EFilpDirect dir, bool b);

    /**
     * @brief isFilped 图像是否翻转过(相对原图)
     */
    bool isFilped(EFilpDirect dir) const;

    /**
     * @brief setRotation90
     */
    void setRotation90(bool leftOrRight);

    /**
     * @brief hitTest 碰撞检测，用来检测鼠标在图元的哪个点位上
     * @param point 鼠标指针
     * @return
     */
    virtual CSizeHandleRect::EDirection hitTest(const QPointF &point) const;

    /**
     * @brief isPosPenetrable 某一位置在图元上是否是可穿透的（透明的）
     * @param posLocal 该图元坐标系的坐标位置
     */
    virtual bool isPosPenetrable(const QPointF &posLocal);

    /**
     * @brief isPosPenetrable 某一矩形区域在图元上是否是可穿透的（透明的）
     * @param rectLocal 该图元坐标系的某一矩形区域
     */
    virtual bool isRectPenetrable(const QRectF &rectLocal);

    /**
     * @brief move  操作开始
     */
    virtual void operatingBegin(CGraphItemEvent *event);

    /**
     * @brief operating  执行操作
     */
    virtual void operating(CGraphItemEvent *event);

    /**
     * @brief testOpetating  测试操作是否可行
     */
    virtual bool testOpetating(CGraphItemEvent *event);

    /**
     * @brief move  操作结束
     */
    virtual void operatingEnd(CGraphItemEvent *event);

    /**
     * @brief  移动事件相关函数
     * @brief  doMoveBegin移动开始;testMoving移动测试;doMoving执行移动;doMoveEnd移动结束
     */
    virtual void doMoveBegin(CGraphItemMoveEvent *event);
    virtual void doMoving(CGraphItemMoveEvent *event);
    virtual bool testMoving(CGraphItemMoveEvent *event);
    virtual void doMoveEnd(CGraphItemMoveEvent *event);

    /**
     * @brief  缩放事件相关函数
     * @brief  doMoveBegin缩放开始;testMoving缩放测试;doMoving缩放移动;doMoveEnd缩放结束
     */
    virtual void doScalBegin(CGraphItemScalEvent *event);
    virtual void doScaling(CGraphItemScalEvent *event);
    virtual bool testScaling(CGraphItemScalEvent *event);
    virtual void doScalEnd(CGraphItemScalEvent *event);

    /**
     * @brief  旋转事件相关函数
     * @brief  doMoveBegin旋转开始;testMoving旋转测试;doMoving旋转移动;doMoveEnd旋转结束
     */
    virtual void doRotBegin(CGraphItemRotEvent *event);
    virtual void doRoting(CGraphItemRotEvent *event);
    virtual bool testRoting(CGraphItemRotEvent *event);
    virtual void doRotEnd(CGraphItemRotEvent *event);

    /**
     * @brief operatingType  操作类型
     */
    int operatingType();

    /**
     * @brief move  移动图元
     * @param beginPoint 移动起始点
     * @param movePoint 移动终点
     */
    virtual void move(QPointF beginPoint, QPointF movePoint);

    /**
      * @brief updateShape  刷新图元的形状(属性变化时调用重新计算图元的形状和样式)
      */
    virtual void updateShape();

    /**
     * @brief setSizeHandleRectFlag 设置边界各个方向上的矩形是否可见
     * @param dir 方向
     * @param flag true: 显示  false:不显示
     */
    //void setSizeHandleRectFlag(CSizeHandleRect::EDirection dir, bool flag);

    /**
     * @brief getGraphicsItemShapePathByOrg 根据画笔属性，把图元形状转为路径   此函数为Qt源码中自带的
     * @param path 形状路径
     * @param pen 画笔
     * @param penStrokerShape 为true表示返回orgPath的线条的填充路径，false表示返回orgPath的最外围路径
     * @param incW 线宽增量(可能的应用场景：虚拟提升线宽使更好选中)
     * @return  转换后的路径
     */
    static QPainterPath getGraphicsItemShapePathByOrg(const QPainterPath &orgPath,
                                                      const QPen &pen,
                                                      bool penStrokerShape = false,
                                                      const qreal incW = 0,
                                                      bool doSimplified = true);

    template<typename T = QGraphicsItem *, typename K = CGraphicsItem *>
    static QList<T> returnList(const QList<K> &list)
    {
        QList<T> result;
        for (auto p : list) {
            result.append(p);
        }
        return result;
    }


    /**
     * @brief zItem 返回z值最小或最低或等于某一z值的item
     * @param pBzItems 被查找的items
     * @param wantZitemTp 想要的类型(-1返回z值最小的item,-2返回z值最大的item,z>=0返回等于该值的item)
     * @return 返回的图元
     */
    static CGraphicsItem *zItem(const QList<CGraphicsItem *> &pBzItems, int wantZitemTp = -1);

    /**
     * @brief isSelected 图元是否处于被选择状态中
     * @return
     */
    bool isSelected() const;

    /**
     * @brief isMutiSelected 图元是否处于被选择多选状态中
     * @return
     */
    bool isMutiSelected() const;

    /**
     * @brief getHighLightPath 获取高亮path
     * @return
     */
    virtual QPainterPath getHighLightPath();

    /**
     * @brief handleNodes 返回控制节点
     */
    virtual QVector<CSizeHandleRect *> handleNodes();

    /**
     * @brief handleNode 返回控制节点
     */
    CSizeHandleRect *handleNode(CSizeHandleRect::EDirection direction = CSizeHandleRect::Rotation);

    /**
     * @brief resetCachePixmap 重置缓冲图
     */
    void resetCachePixmap();

    /**
     * @brief getFilpTransform 获取到翻转的转换矩阵
     */
    QTransform getFilpTransform() const;

// Cppcheck检测函数没有使用到
//    /**
//     * @brief drawItem 获取到翻转的转换矩阵
//     */
//    void drawItem(QPainter *painter, const QStyleOptionGraphicsItem *option);


    QPixmap rasterSelf();

    virtual void rasterToSelfLayer(bool deleteSelf = true);

    //QPointF mapFromDrawScene(const QPointF &posInDScene) const;
    //QPointF mapToDrawScene(const QPointF &posInThis) const;

    void updateViewport();

protected:
    /**
     * @brief loadHeadData 加载通用数据
     */
    void loadHeadData(const SGraphicsUnitHead &head);

    /**
     * @brief beginCheckIns 检查图元是否和场景就交集（必须和endCheckIns成對出现）
     * @param painter 绘制指针
     */
    void beginCheckIns(QPainter *painter);

    /**
     * @brief endCheckIns 结束检查图元是否和场景就交集（必须和beginCheckIns成對出现）
     * @param painter 绘制指针
     */
    void endCheckIns(QPainter *painter);

    /**
     * @brief paintMutBoundingLine 显示菜单
     * @param painter 绘制指针
     * @param option  绘制信息
     */
    void paintMutBoundingLine(QPainter *painter, const QStyleOptionGraphicsItem *option);

protected:
    /**
     * @brief selfOrgShape 图元的原始形状（rect类图元不包括边线）
     */
    virtual QPainterPath getSelfOrgShape() const;

    /**
     * @brief penStrokerShape 图元线条的形状（边线轮廓所组成的形状）
     */
    virtual QPainterPath getPenStrokerShape() const;

    /**
     * @brief shape 返回图元的外形状
     */
    virtual QPainterPath getShape() const;

    /**
     * @brief shape 返回真实显示的图元的外形状()
     */
    virtual QPainterPath getTrulyShape() const;

    /**
     * @brief setState 设置图元外接矩形状态
     * @param st 状态
     */
    virtual void setState(ESelectionHandleState st);

    /**
     * @brief contextMenuEvent 显示菜单
     * @param event 状态
     */
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

    /**
     * @brief itemChange 图元变更
     * @param change 变更属性
     * @param value 变更的值
     * @return
     */
    virtual QVariant itemChange(GraphicsItemChange doChange, const QVariant &value) override;

    /**
     * @brief initHandle 初始化控制节点item
     */
    virtual void initHandle();

    /**
     * @brief    updateHandlesGeometry 更新孩子节点们的位置
     */
    virtual void updateHandlesGeometry();

    /**
     * @brief clearHandle 清理所有控制节点item
     */
    virtual void clearHandle();

    /**
     * @brief incLength 虚拟的额外线宽宽度（解决选中困难的问题 提升用户体验）
     * @return 返回额外的线宽（一般与当前的放大值相关）
     */
    virtual qreal incLength() const;

    /**
     * @brief paint 绘制图元
     */
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;

    /**
     * @brief paintItemSelf paintSelf的马甲,同时在绘制前调用前做了一些图元的吃初始化操作
     * @param option 绘制信息
     * @param
     */
    enum EPaintReson {EPaintForDynamic, EPaintForStatic, EPaintForNoCache = EPaintForDynamic, EPaintForCache = EPaintForStatic};
    void paintItemSelf(QPainter *painter, const QStyleOptionGraphicsItem *option, EPaintReson paintReson);

    /**
     * @brief paintCache 绘制自身的样貌
     */
    virtual void paintSelf(QPainter *painter, const QStyleOptionGraphicsItem *option);

    /**
     * @brief 鼠标事件
     */
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

protected:
    /**
     * @brief shape 返回真实显示的图元的外形状()
     * @param baseOrg true表示基于原图进行模糊,false表示基于当前的模糊情况再模糊
     */
    QPixmap getCachePixmap(bool baseOrg = false);


    void  changeTransCenterTo(const QPointF &newCenter);
protected:
    typedef QVector<CSizeHandleRect *> Handles;

    /* 选中时出现的控制节点 */
    Handles m_handles;

    int m_operatingType = -1;

    QColor m_penPreviewColor;
    int m_penWidth = 1;
    QColor m_brPreviewColor;
    bool m_isPreviewCom[3] {0};

    QPainterPath m_selfOrgPathShape;
    QPainterPath m_penStroerPathShape;
    QPainterPath m_boundingShape;
    QRectF       m_boundingRect;

    QPainterPath m_boundingShapeTrue;
    QRectF       m_boundingRectTrue;

    QPixmap    *_cachePixmap    = nullptr;
    bool        _useCachePixmap = false;
    bool        _autoCache      = true;
    int         _autoEplMs      = 8;
    QStyleOptionGraphicsItem  _curStyleOption;

    CGraphicsItemGroup *_pGroup = nullptr;

public:

    static bool paintInteractBorderLine;

public:
    /**
     * @brief isBlurEnable 图元是否支持模糊
     */
    virtual bool isBlurEnable()const {return false;}
    /**
     * @brief isBlurEnable 图元是否支持模糊
     */
    virtual bool isEraserEnable()const {return false;}
    /**
     * @brief blurBegin 模糊操作开始
     * @param pos 当前点(图元自身坐标系)
     */
    virtual void blurBegin(const QPointF &pos);

    /**
     * @brief blurUpdate 模糊操作刷新
     * @param pos 当前点(图元自身坐标系)
     * @param optm 是否进行额外优化,如果是那么在模糊结束后,记得调用一下resetCachePixmap
     */
    virtual void blurUpdate(const QPointF &pos, bool optm = false);

    /**
     * @brief blurEnd 模糊操作结束
     */
    virtual void blurEnd();

    /**
     * @brief isBlurActived 模糊操作正在进行中
     */
    virtual bool isBlurActived();

    /**
     * @brief setDrawRotatin 设置图元的旋转角度(仅仅是设置旋转值,不会引起图元的旋转)
     */
    void  setDrawRotatin(qreal angle);

    /**
     * @brief setDrawRotatin 图元的旋转角度
     */
    qreal drawRotation()const {return _roteAgnel;}

    /**
     * @brief updateShapeRecursion 嵌套循环刷新形状及所处组合形状
     */
    void updateShapeRecursion();

protected:
    /**
     * @brief updateBlurPixmap 获取到新的模糊图像
     * @param onlyOrg 为true表示在原图的基础上进行模糊,false表示基于当前的模糊情况进行再模糊
     * @param effetTp 希望获取到的模糊图的类型
     */
    void updateBlurPixmap(bool baseOrg = false, EBlurEffect effetTp = UnknowEffect);

    /**
     * @brief updateBlurPixmapBySelfBlurInfo 根据当前模糊的需要获取到新的模糊图像
     */
    void updateBlurPixmapBySelfBlurInfo();

    /**
     * @brief addBlur 添加一个模糊路径
     */
    //void addBlur(const SBlurInfo &sblurInfo);

    /**
     * @brief paintAllBlur 绘制所有的模糊
     * @param translate 可设置绘制的路径偏移
     */
    void paintAllBlur(QPainter *painter, const QPointF &translate = QPointF(0, 0));

    /**
     * @brief paintBlur 绘制一个模糊
     * @param translate 可设置绘制的路径偏移
     */
    void paintBlur(QPainter *painter, const SBlurInfo &info, const QPointF &translate = QPointF(0, 0));

protected:
    /* blur infos */
    QList<SBlurInfo> _blurInfos;              //图元的所有模糊信息(路径,模糊样式等)
    SBlurInfo        _tempActiveBlurInfo;     //图元正在生成中的模糊信息
    QPainterPath     _tempActiveBlurPath;     //图元正在生成中的模糊路径
    QPixmap          _blurPix[UnknowEffect];  //图元的模糊图像

    /* 关于镜像翻转 */
    bool _flipHorizontal = false;   // 水平翻转
    bool _flipVertical   = false;   // 垂直翻转

    qreal            _roteAgnel = 0;         //图元的旋转角度


    CGraphicsLayer *_layer = nullptr;

    DECLAREPRIVATECLASS(CGraphicsItem)
};

#endif // CGRAPHICSITEM_H
