// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SITEMDATA_H
#define SITEMDATA_H
#include <QDataStream>
#include <QPen>
#include <QBrush>
#include <QPoint>
#include <QPainterPath>
#include <QTextDocument>
#include <QDebug>
#include <QBuffer>
#include <QTime>
#include <QCryptographicHash>
#include <QList>
#include <QThread>
#include "globaldefine.h"
#include "rastercommand.h"

#pragma pack(push, 1)

struct DRAWLIB_EXPORT SBlurInfo {
    QPainterPath blurPath;
    EBlurEffect  blurEfTp = UnknowEffect;
    bool isValid()const
    {
        return !blurPath.isEmpty();
    }
    void clear()
    {
        blurPath = QPainterPath();
    }
};
Q_DECLARE_METATYPE(SBlurInfo);

//图元头部
struct DRAWLIB_EXPORT UnitHead {
    qint8  headCheck[4];         //头部校验
    qint32 dataType;             //图元类型
    qint64 dataLength;           //数据长度

    QPen             pen;                //图元使用的画笔信息
    QBrush           brush;              //图元使用的画刷信息
    QPointF          pos;                //图元起始位置
    qreal            rotate;             //旋转角度(保存了trans的旋转角度值)
    qreal            zValue;             //Z值 用来保存图形层次
    QTransform       trans;              //图元的变换矩阵(自EDdf5_9_0_1_LATER添加)
    int              blurCount = 0;      //模糊的个数
    QList<SBlurInfo> blurInfos;          //所有模糊信息
    QRectF           rect = QRectF(0, 0, 0, 0);      //图元的矩形大小
};

//图元尾部
struct DRAWLIB_EXPORT UnitTail {
    qint8 tailCheck[4]; //尾部校验
};

//数据封装
struct DRAWLIB_EXPORT UnitData {
    QVariant data;
    int expendType = -1;
    template<class T>
    inline T value()const
    {
        return data.value<T>();
    }
    template<class T>
    inline void setValue(const T &v)
    {
        return data.setValue<T>(v);
    }
};
enum EUnitUsage {
    UnitUsage_NoUsed,
    UnitUsage_OnlySelf = 0x01,

    UnitUsage_All_DontNewChild = UnitUsage_OnlySelf << 1,
    UnitUsage_All_NewChild     = UnitUsage_OnlySelf << 2,
    UnitUsage_All              = UnitUsage_All_NewChild,

    UnitUsage_Copy = UnitUsage_All | (UnitUsage_OnlySelf << 3),
    UnitUsage_Save = UnitUsage_All | (UnitUsage_OnlySelf << 4),
    UnitUsage_Undo = UnitUsage_All_DontNewChild | (UnitUsage_OnlySelf << 5)
};

//单个图数据
struct DRAWLIB_EXPORT Unit {
    Unit(int usg = UnitUsage_NoUsed): usage(usg)
    {

    }
    UnitHead head;          //单个图元的头部信息及校验
    UnitData data;          //单个图元的数据部分
    UnitTail tail;          //单个图元的尾部校验
    QList<Unit> chidren;
    int usage = UnitUsage_NoUsed;
    bool isValid()const
    {
        return data.data.isValid();
    }
    template<class T>
    inline T value()const
    {
        return data.value<T>();
    }
    template<class T>
    inline void setValue(const T &v)
    {
        return data.setValue<T>(v);
    }
};
Q_DECLARE_METATYPE(Unit)
using UnitList = QList<Unit>;

struct DRAWLIB_EXPORT RasterUnitData {
    QList<QSharedPointer<RasterCommandBase>> commands;
    QImage baseImg;
    QRectF baseRect;
    bool   blocked = false;
    int    layerType = 0;
};
Q_DECLARE_METATYPE(RasterUnitData)
struct DRAWLIB_EXPORT LayerUnitData {
    RasterUnitData rasterData;
    QBrush         bgBrush;
};
Q_DECLARE_METATYPE(LayerUnitData)

//组合信息
struct DRAWLIB_EXPORT GroupUnitData {
    bool     isCancelAble = true;

    //组合的名字
    int       nameByteSz = 0;
    QString   name;

    //组合的类型
    int      groupType = 0;
};
Q_DECLARE_METATYPE(GroupUnitData)

//矩形
struct DRAWLIB_EXPORT RectUnitData {
    QPointF topLeft;
    QPointF bottomRight;
    int xRedius = 0;
    int yRedius = 0;

    bool bSameRadius = true;
    int  leftRadius = 0;
    int  rightRadius = 0;
    int  leftBottomRadius = 0;
    int  rightBottomRadius = 0;
};
Q_DECLARE_METATYPE(RectUnitData)

//圆
struct DRAWLIB_EXPORT EllipseUnitData {
    RectUnitData rect;
};
Q_DECLARE_METATYPE(EllipseUnitData)

//三角形
struct DRAWLIB_EXPORT TriangleUnitData {
    RectUnitData rect;
};
Q_DECLARE_METATYPE(TriangleUnitData)

//多边形
struct DRAWLIB_EXPORT PolygonUnitData {
    RectUnitData rect; //外接矩形
    qint32 pointNum = 0;
};
Q_DECLARE_METATYPE(PolygonUnitData)

//多角星
struct DRAWLIB_EXPORT StarUnit {
    RectUnitData rect; //外接矩形
    qint32 anchorNum = 0;
    qint32 radius = 0;      //内接半径
};
Q_DECLARE_METATYPE(StarUnit)

//线
enum ELinePosStyle {
    noneLine,     // 无
    normalRing,   // 空心圆
    soildRing,    // 实心圆
    normalArrow,  // 普通箭头
    soildArrow    // 实心箭头
};
struct DRAWLIB_EXPORT LineUnitData {
    QPointF point1;
    QPointF point2;
    ELinePosStyle start_type = noneLine; // 起点箭头样式
    ELinePosStyle end_type = noneLine;   // 终点箭头样式
};
Q_DECLARE_METATYPE(LineUnitData)

//文本
struct DRAWLIB_EXPORT TextUnitData {
    RectUnitData rect;
    QFont font;
    bool manResizeFlag = false;
    QString content;
    QColor color;  //不用保存到ddf
};
Q_DECLARE_METATYPE(TextUnitData)

//图片
struct ImageUnitData_Comp {
    RectUnitData rect;
    QImage image;
    QByteArray srcByteArry;
    bool flipHorizontal = false; // 水平翻转
    bool flipVertical = false;   // 垂直翻转
};
Q_DECLARE_METATYPE(ImageUnitData_Comp)

//画笔
struct PenUnitData_Comp {

    //version 2
    ELinePosStyle start_type = noneLine; // 起点箭头样式
    ELinePosStyle end_type = noneLine;   // 终点箭头样式
    QPainterPath path;

    //version 1
    QPolygonF arrow;
};
Q_DECLARE_METATYPE(PenUnitData_Comp)

struct BlurUnitData_Comp {
    PenUnitData_Comp data;
    qint8 effect = 0;
};
Q_DECLARE_METATYPE(BlurUnitData_Comp)

//class DRAWLIB_EXPORT RasterCmdBase
//{
//public:
//    virtual void doCommand() = 0;
//    virtual int  cmdType() = 0;
//    virtual void serialization(QDataStream &out) {}
//    virtual void deserialization(QDataStream &in) {}

//    static RasterCmdBase *creatCmd(int tp);
//};


//整个图元数据
struct DRAWLIB_EXPORT CGraphics {
    qint32        version = qint64(EDdfCurVersion);             //数据版本 默认给最新的版本枚举值
    qint64        unitCount;                                    //图元数量
    QRectF        rect;                                         // 画板大小和位置
    QVector<Unit> vecGraphicsUnit; //所有图元集合(不用保存到ddf)
};

struct DRAWLIB_EXPORT UnitTree_Comp {
    QList<Unit>      bzItems;
    QList<UnitTree_Comp>  childTrees;
    Unit   treeUnit;
    int    gpType = -1;

    int groupType() const
    {
        return this->gpType;
    }
    void setGroupType(int type)
    {
        gpType = type;
    }
};
Q_DECLARE_METATYPE(UnitTree_Comp)

struct DRAWLIB_EXPORT PageContextHead {
    quint32 ddfFlag = static_cast<quint32>(0xA0B0C0D0);
    qint32  version = qint64(EDdfCurVersion);             //数据版本 默认给最新的版本枚举值
    qint64  unitCount = 0;                                //图元数量
    QRectF  rect;                                         //画板大小和位置
};
struct DRAWLIB_EXPORT PageContextData {
    PageContextHead      head;
    UnitTree_Comp        units_comp;
    UnitList             units;
};
Q_DECLARE_METATYPE(PageContextData)

#pragma pack(pop)

#endif // SITEMDATA_H
