// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
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
#include "drawshape/globaldefine.h"

#pragma pack(push, 1)

enum EDdfVersion {
    EDdfUnknowed = -1,       //未知的版本(ddf被修改过)

    EDdf5_8_0_Base = 0,      //最原始的ddf版本
    EDdf5_8_0_9_1,           //添加了矩形属性的ddf版本
    EDdf5_8_0_10_1,          //添加了直线起点终点的版本（但这个版本在保存时漏掉保存了pen图元的保存 所以这个版本保存的ddf被加载后pen图元显示会问题）
    EDdf5_8_0_10_2,          //修复了EDdf5_8_0_10_1版本未保存画笔图元path的问题
    EDdf5_8_0_16_1,          //为了优化图片的保存速度修改了保存图片时由原先的直接保存QImage该成保存QByteArry

    EDdf5_8_0_20,            //添加功能md5校验，以检查ddf文件是否被修改过(ddf破坏过,变脏了那么不再加载或其他处理方式?)

    EDdf5_8_0_48_LATER,      //1030专业版5_8_0_48版本后将多保存图片图元的翻转信息,以解决翻转信息丢失的问题

    EDdf5_8_0_84_LATER,      //1040 sprint-1引入模糊,仅模糊图片,需要记录图元的模糊信息(做成了一种属性,而非模糊图元了)(实际发布班为5.9.0.1)

    EDdf5_9_0_1_LATER,       //5.9.0.1之后添加了图元的transform保存

    EDdf5_9_0_3_LATER,       //5.9.0.3之后在默认head中添加了QRecF类型的数据(对所有图元都应该有一个矩形),同时开始支持组合信息的保存

    EDdf5_9_9_0_LATER,       //5.9.9.0之后(include) dylayer 这个版本将画笔绘制成了一个位图,同时添加了基于位图的模糊，之前的针对图片采取的矢量模糊，现在需要调整为位图模糊

    EDdf5_10_2_7_LATER,      //5_10_2_7之后 dylayer添加一个layerType用来表示图层的类型，如画笔图层或者图像图层

    EDdfVersionCount,

    EDdfCurVersion = EDdfVersionCount - 1  //最新的版本号(添加新的枚举必须添加到EDdfUnknowed和EDdfVersionCount之间)
};

static EDdfVersion getVersion(QDataStream &stream)
{
    EDdfVersion     version = EDdfUnknowed;


    if (stream.device() != nullptr) {
        quint32         headCheckFlag;
        qint64 pos = stream.device()->pos();

        stream.device()->seek(0);

        stream >> headCheckFlag;

        if (headCheckFlag == static_cast<quint32>(0xA0B0C0D0)) {
            int verVar;
            stream >> verVar;
            version = EDdfVersion(verVar & 0xff);

        } else {
            version = EDdf5_8_0_Base;
            //version = EDdfUnknowed;
        }
        //还原
        stream.device()->seek(pos);
    }

    return version;
}

struct SBlurInfo {
    //QPointF      startPos;
    QPainterPath blurPath;
    EBlurEffect  blurEfTp = UnknowEffect;
    bool isValid()const
    {
        return !blurPath.isEmpty();
    }
    void clear()
    {
        blurPath = QPainterPath();
        //startPos = QPoint(0, 0);
    }

    friend QDataStream &operator<<(QDataStream &out, const SBlurInfo &blurInfo)
    {
        out << blurInfo.blurPath;
        out << int(blurInfo.blurEfTp);
        return out;
    }

    friend QDataStream &operator>>(QDataStream &in, SBlurInfo &head)
    {
        in >> head.blurPath;
        int bluref = UnknowEffect;
        in >> bluref;
        head.blurEfTp = EBlurEffect(bluref);

        return in;
    }

};
Q_DECLARE_METATYPE(SBlurInfo);

//图元头部
struct SGraphicsUnitHead {
    qint8  headCheck[4];             //头部校验
    qint32 dataType = 0;             //图元类型
    qint64 dataLength = 0;           //数据长度

    QPen             pen;                //图元使用的画笔信息
    QBrush           brush;              //图元使用的画刷信息
    QPointF          pos;                //图元起始位置
    qreal            rotate = 0;         //旋转角度(保存了trans的旋转角度值)
    qreal            zValue = 0;         //Z值 用来保存图形层次
    QTransform       trans;              //图元的变换矩阵(自EDdf5_9_0_1_LATER添加)
    int              blurCount = 0;      //模糊的个数
    QList<SBlurInfo> blurInfos;          //所有模糊信息
    QRectF           rect = QRectF(0, 0, 0, 0);      //图元的矩形大小

    SGraphicsUnitHead()
    {
        memset(headCheck, 0, 4);
    }

    friend QDataStream &operator<<(QDataStream &out, const SGraphicsUnitHead &head)
    {
        qDebug() << "save blurCount = " << head.blurCount;
        out << head.headCheck[0];
        out << head.headCheck[1];
        out << head.headCheck[2];
        out << head.headCheck[3];
        out << head.dataType;
        out << head.dataLength;
        out << head.pen;
        out << head.brush;
        out << head.pos;
        out << head.rotate;
        out << head.zValue;
        out << head.blurCount;
        if (head.blurCount > 0) {
            for (auto info : head.blurInfos) {
                out << info;
            }
        }
        out << head.trans;
        out << head.rect;
        return out;
    }

    friend QDataStream &operator>>(QDataStream &in, SGraphicsUnitHead &head)
    {
        in >> head.headCheck[0];
        in >> head.headCheck[1];
        in >> head.headCheck[2];
        in >> head.headCheck[3];
        in >> head.dataType;
        in >> head.dataLength;
        in >> head.pen;
        in >> head.brush;
        in >> head.pos;
        in >> head.rotate;
        in >> head.zValue;

        //这个及之后的版本要加载模糊信息
        EDdfVersion version = getVersion(in);
        if (version >= EDdf5_8_0_84_LATER) {
            in >> head.blurCount;
            //qDebug() << "=====head.blurCount ============== " << head.blurCount;
            for (int i = 0; i < head.blurCount; ++i) {
                SBlurInfo blurInfo;
                in >> blurInfo;
                head.blurInfos.append(blurInfo);
                qDebug() << "blurInfo tp = " << blurInfo.blurEfTp;
            }
            if (version >= EDdf5_9_0_1_LATER) {
                in >> head.trans;

                if (version >= EDdf5_9_0_3_LATER) {
                    in >> head.rect;
                }
            }
        }
        return in;
    }
};

//图元尾部
struct SGraphicsUnitTail {
    qint8 tailCheck[4]; //尾部校验


    friend QDataStream &operator<<(QDataStream &out, const SGraphicsUnitTail &tail)
    {
        out << tail.tailCheck[0];
        out << tail.tailCheck[1];
        out << tail.tailCheck[2];
        out << tail.tailCheck[3];

        return out;
    }

    friend QDataStream &operator>>(QDataStream &in, SGraphicsUnitTail &tail)
    {
        in >> tail.tailCheck[0];
        in >> tail.tailCheck[1];
        in >> tail.tailCheck[2];
        in >> tail.tailCheck[3];

        return in;
    }
};

//组合信息
struct SGraphicsGroupUnitData {
    bool     isCancelAble = true;

    //组合的名字
    int       nameByteSz = 0;
    QString   name;

    //组合的类型
    int      groupType = 0;

    friend QDataStream &operator<<(QDataStream &out, const SGraphicsGroupUnitData &groupUnitData)
    {
        out << groupUnitData.isCancelAble;

        if (!groupUnitData.name.isEmpty()) {
            QByteArray arry = groupUnitData.name.toUtf8();
            int count = arry.size();
            char *p = arry.data();
            out << count;
            out.writeRawData(p, count);
        } else {
            out << int(0);
        }
        out << groupUnitData.groupType;
        return out;
    }

    friend QDataStream &operator>>(QDataStream &in, SGraphicsGroupUnitData &groupUnitData)
    {
        EDdfVersion version = getVersion(in);
        if (version >= EDdf5_9_0_3_LATER) {
            in >> groupUnitData.isCancelAble;
            int count = 0;
            in >> count;
            if (count > 0) {
                in >> groupUnitData.nameByteSz;
                if (groupUnitData.nameByteSz > 0) {
                    char *nameTemp = new char[unsigned(groupUnitData.nameByteSz)];
                    in.readRawData(nameTemp, int(groupUnitData.nameByteSz));
                    groupUnitData.name = QString::fromUtf8(nameTemp, groupUnitData.nameByteSz);
                }
            }
            in >> groupUnitData.groupType;
        }
        return in;
    }
};

//矩形
struct SGraphicsRectUnitData {
    QPointF topLeft;
    QPointF bottomRight;
    int xRedius = 0;
    int yRedius = 0;
    friend QDataStream &operator<<(QDataStream &out, const SGraphicsRectUnitData &rectUnitData)
    {
        out << rectUnitData.topLeft;
        out << rectUnitData.bottomRight;
        out << rectUnitData.xRedius;
        out << rectUnitData.yRedius;
        return out;
    }

    friend QDataStream &operator>>(QDataStream &in, SGraphicsRectUnitData &rectUnitData)
    {
        if (getVersion(in) > EDdf5_8_0_Base) {
            in >> rectUnitData.topLeft;
            in >> rectUnitData.bottomRight;
            in >> rectUnitData.xRedius;
            in >> rectUnitData.yRedius;
        } else {
            in >> rectUnitData.topLeft;
            in >> rectUnitData.bottomRight;
            rectUnitData.xRedius = 0;
            rectUnitData.yRedius = 0;
        }
        return in;
    }
};

//圆
struct SGraphicsCircleUnitData {
    SGraphicsRectUnitData rect;
    friend  QDataStream &operator<<(QDataStream &out, const SGraphicsCircleUnitData &cirUnitData)
    {
        out << cirUnitData.rect;
        return out;
    }

    friend QDataStream &operator>>(QDataStream &in, SGraphicsCircleUnitData &cirUnitData)
    {
        in >> cirUnitData.rect;
        return in;
    }
};

//三角形
struct SGraphicsTriangleUnitData {
    SGraphicsRectUnitData rect;
    //TODO 序列化和反序列化
    friend  QDataStream &operator<<(QDataStream &out, const SGraphicsTriangleUnitData &triangleUnitData)
    {
        out << triangleUnitData.rect;
        return out;
    }

    friend QDataStream &operator>>(QDataStream &in, SGraphicsTriangleUnitData &triangleUnitData)
    {
        in >> triangleUnitData.rect;
        return in;
    }
};

//多边形
struct SGraphicsPolygonUnitData {
    SGraphicsRectUnitData rect; //外接矩形
    qint32 pointNum = 0;

    friend  QDataStream &operator << (QDataStream &out, const SGraphicsPolygonUnitData &polygonUnitData)
    {
        out << polygonUnitData.rect;
        out << polygonUnitData.pointNum;
        return out;
    }

    friend QDataStream &operator>>(QDataStream &in, SGraphicsPolygonUnitData &polygonUnitData)
    {
        in >> polygonUnitData.rect;
        in >> polygonUnitData.pointNum;
        return in;
    }
};

//多角星
struct SGraphicsPolygonStarUnitData {
    SGraphicsRectUnitData rect; //外接矩形
    qint32 anchorNum = 0;
    qint32 radius = 0;      //内接半径

    friend  QDataStream &operator << (QDataStream &out, const SGraphicsPolygonStarUnitData &polygonStarUnitData)
    {
        out << polygonStarUnitData.rect;
        out << polygonStarUnitData.anchorNum;
        out << polygonStarUnitData.radius;
        return out;
    }

    friend QDataStream &operator>>(QDataStream &in, SGraphicsPolygonStarUnitData &polygonStarUnitData)
    {
        in >> polygonStarUnitData.rect;
        in >> polygonStarUnitData.anchorNum;
        in >> polygonStarUnitData.radius;
        return in;
    }
};

//线
struct SGraphicsLineUnitData {
    QPointF point1;
    QPointF point2;
    ELineType start_type = noneLine; // 起点箭头样式
    ELineType end_type = noneLine;   // 终点箭头样式

    friend  QDataStream &operator << (QDataStream &out, const SGraphicsLineUnitData &lineUnitData)
    {
        out << lineUnitData.point1;
        out << lineUnitData.point2;
        out << lineUnitData.start_type;
        out << lineUnitData.end_type;

        return out;
    }

    friend QDataStream &operator>>(QDataStream &in, SGraphicsLineUnitData &lineUnitData)
    {
//        quint32 start_type = 0;
//        quint32 end_type = 0;
//        qint64 pos = in.device()->pos();
//        in.device()->seek(0);
//        quint32 type;
//        in >> type;
//        int version;
//        in >> version;
//        in.device()->seek(pos);
//        in >> lineUnitData.point1;
//        in >> lineUnitData.point2;
//        in >> start_type;
//        lineUnitData.start_type = static_cast<ELineType>(start_type);
//        if (type == (quint32)0xA0B0C0D0 && version >= LineStartAndEndType) {
//            in >> end_type;
//            lineUnitData.end_type = static_cast<ELineType>(end_type);
//        }
        EDdfVersion ver = getVersion(in);
        switch (ver) {
        case EDdf5_8_0_Base:
        case EDdf5_8_0_9_1: {
            in >> lineUnitData.point1;
            in >> lineUnitData.point2;
            break;
        }
        default: {
            in >> lineUnitData.point1;
            in >> lineUnitData.point2;

            qint32 startTp, endTp;
            in >> startTp;
            in >> endTp;

            lineUnitData.start_type = ELineType(startTp);
            lineUnitData.end_type   = ELineType(endTp);

            break;
        }
        }
        return in;
    }
};

//文本
struct SGraphicsTextUnitData {
    SGraphicsRectUnitData rect;
    QFont font;
    bool manResizeFlag = false;
    QString content;
    QColor color;  //不用保存到ddf

    friend  QDataStream &operator << (QDataStream &out, const SGraphicsTextUnitData &textUnitData)
    {
        out << textUnitData.rect;
        out << textUnitData.font;
        out << textUnitData.manResizeFlag;
        out << textUnitData.content;

        return out;
    }

    friend QDataStream &operator>>(QDataStream &in, SGraphicsTextUnitData &textUnitData)
    {
        in >> textUnitData.rect;
        in >> textUnitData.font;
        in >> textUnitData.manResizeFlag;
        in >> textUnitData.content;

        return in;
    }
};

//图片
struct SGraphicsPictureUnitData {
    SGraphicsRectUnitData rect;
    QImage image;
    QByteArray srcByteArry;
    bool flipHorizontal = false; // 水平翻转
    bool flipVertical = false;   // 垂直翻转

    friend  QDataStream &operator << (QDataStream &out, const SGraphicsPictureUnitData &pictureUnitData)
    {
        out << pictureUnitData.rect;

        //out << pictureUnitData.image;

        qDebug() << "save pictureUnitData.srcByteArry = size = " << pictureUnitData.srcByteArry.size();

        out << pictureUnitData.srcByteArry;

        out << pictureUnitData.flipHorizontal;

        out << pictureUnitData.flipVertical;

        return out;
    }

    friend QDataStream &operator>>(QDataStream &in, SGraphicsPictureUnitData &pictureUnitData)
    {
        in >> pictureUnitData.rect;

        EDdfVersion curDdfVersion = getVersion(in);
        if (curDdfVersion < EDdf5_8_0_16_1) {
            in >> pictureUnitData.image;
        } else {
            QByteArray arryData;
            in >> arryData;
            //qDebug() << "load arrydata ================== " << "size === " << arryData.size();
            pictureUnitData.srcByteArry = arryData;
            pictureUnitData.image = QImage::fromData(arryData);

            if (curDdfVersion >= EDdf5_8_0_48_LATER) {
                in >> pictureUnitData.flipHorizontal;
                in >> pictureUnitData.flipVertical;
            }
        }

        return in;
    }
};

//画笔
struct SGraphicsPenUnitData {

    //version 2
    ELineType start_type = noneLine; // 起点箭头样式
    ELineType end_type = noneLine;   // 终点箭头样式
    QPainterPath path;

    //version 1
    QPolygonF arrow;

    friend  QDataStream &operator << (QDataStream &out, const SGraphicsPenUnitData &penUnitData)
    {
        out << penUnitData.start_type;

        out << penUnitData.path;
//        out << penUnitData.arrow;
        out << penUnitData.end_type;

        return out;
    }

    friend QDataStream &operator>>(QDataStream &in, SGraphicsPenUnitData &penUnitData)
    {
//        quint32 start_type = 0;
//        quint32 end_type = 0;

//        //in >> end_type;
//        //in >> penUnitData.path;
////        in >> penUnitData.arrow;
//        QPolygonF plog;
//        in >> plog;
//        //in >> start_type;

//        penUnitData.start_type = static_cast<ELineType>(start_type);
//        penUnitData.end_type = static_cast<ELineType>(end_type);

//        return in;
        EDdfVersion ver = getVersion(in);

        switch (ver) {
        case EDdf5_8_0_Base:
        case EDdf5_8_0_9_1:
        case EDdf5_8_0_10_1: {
            qint8 pentype;
            in >> pentype;
            in >> penUnitData.path;
            in >> penUnitData.arrow;

            //以当前版本的形式保存之前的版本样式
            if (!penUnitData.arrow.isEmpty()) {
                penUnitData.start_type = soildArrow;
            }

            break;
        }
        default: {
            qint32 startTp, endTp;
            QPainterPath pathTemp;
            in >> startTp;
            in >> pathTemp;
            in >> endTp;
            penUnitData.start_type = ELineType(startTp);
            penUnitData.path       = pathTemp;
            penUnitData.end_type   = ELineType(endTp);
            break;
        }
        }
        return in;
    }
};

struct SGraphicsBlurUnitData {
    SGraphicsPenUnitData data;
    qint8 effect = 0;

    friend  QDataStream &operator << (QDataStream &out, const SGraphicsBlurUnitData &blurUnitData)
    {
        out << blurUnitData.data;
        out << blurUnitData.effect;

        return out;
    }

    friend QDataStream &operator>>(QDataStream &in, SGraphicsBlurUnitData &blurUnitData)
    {
        in >> blurUnitData.data;
        in >> blurUnitData.effect;
        return in;
    }
};

struct SGraphicsEraseUnitData {
    QPainterPath path;
};
struct SGraphicsFillColorUnitData {
    QPolygonF poses;
    QColor oldColor;
    QColor newColor;
};

class JDyLayerCmdBase
{
public:
    virtual void doCommand() = 0;
    virtual int  cmdType() = 0;
    virtual void serialization(QDataStream &out) {}
    virtual void deserialization(QDataStream &in) {}

    static JDyLayerCmdBase *creatCmd(int tp);
};
struct SDynamicLayerUnitData {
    QList<QSharedPointer<JDyLayerCmdBase>> commands;
    QImage baseImg;
    bool   blocked = false;
    int    layerType = 0;
    friend QDataStream &operator<<(QDataStream &out, const SDynamicLayerUnitData &layUnit)
    {
        out << layUnit.baseImg;
        out << layUnit.blocked;
        out << layUnit.layerType;
        out << layUnit.commands.count();
        foreach (auto cmd, layUnit.commands) {
            out << cmd->cmdType();
            cmd->serialization(out);
        }
        return out;
    }

    friend QDataStream &operator>>(QDataStream &in, SDynamicLayerUnitData &layUnit)
    {
        in >> layUnit.baseImg;
        in >> layUnit.blocked;
        if (getVersion(in) >= EDdf5_10_2_7_LATER) {
            in >> layUnit.layerType;
        } else {
            //0 mean is pen draw,1 mean is one imag;
            layUnit.layerType = layUnit.baseImg.isNull() ? 0 : 1;
        }
        int count = 0;
        in >> count;
        for (int i = 0; i < count; ++i) {
            int tp = 0;
            in >> tp;
            auto cmd = JDyLayerCmdBase::creatCmd(tp);
            if (cmd != nullptr) {
                cmd->deserialization(in);
                QSharedPointer<JDyLayerCmdBase> p(cmd);
                layUnit.commands.append(/*cmd*/p);
            }
        }
        return in;
    }
};

//数据封装
union CGraphicsItemData {
    SGraphicsGroupUnitData *pGroup;               //组合图元数据
    SGraphicsRectUnitData *pRect;                 //矩形图元数据
    SGraphicsCircleUnitData *pCircle;             //椭圆图元数据
    SGraphicsTriangleUnitData *pTriangle;         //三角形图元数据
    SGraphicsPolygonUnitData *pPolygon;           //多边形图元数据
    SGraphicsPolygonStarUnitData *pPolygonStar;   //星形图元数据
    SGraphicsLineUnitData *pLine;                 //直线图元数据
    SGraphicsTextUnitData *pText;                 //文字图元数据
    SGraphicsPictureUnitData *pPic;               //图片图元数据
    SGraphicsPenUnitData *pPen;                   //画笔图元数据
    SGraphicsBlurUnitData *pBlur;                 //模糊图元数据

    SGraphicsEraseUnitData *pErase;
    SGraphicsFillColorUnitData *pFillColor;
    SDynamicLayerUnitData *pDyLayer;

    CGraphicsItemData() {
        pGroup = nullptr;
        pRect = nullptr;
        pCircle = nullptr;
        pTriangle = nullptr;
        pPolygon = nullptr;
        pPolygonStar = nullptr;
        pLine = nullptr;
        pText = nullptr;
        pPic = nullptr;
        pPen = nullptr;
        pBlur = nullptr;
        pErase = nullptr;
        pFillColor = nullptr;
        pDyLayer = nullptr;
    }
};

enum EDataReason {EDuplicate, EUndoRedo, ESaveToDDf, ENormal};
//单个图数据
struct CGraphicsUnit {
    SGraphicsUnitHead head;          //单个图元的头部信息及校验
    CGraphicsItemData data;          //单个图元的数据部分
    SGraphicsUnitTail tail;          //单个图元的尾部校验
    QList<CGraphicsUnit> chidren;
    EDataReason       reson = ENormal;

    CGraphicsUnit() {}

    static void deepCopy(CGraphicsUnit &des, const CGraphicsUnit &source)
    {
        des.head = source.head;
        //qDebug() << "source blurCount = " << source.head.blurCount << "des blurCount = " << des.head.blurCount;
        switch (source.head.dataType) {
        case RectType: {
            if (source.data.pRect == nullptr) {
                des.release();
                break;
            }

            if (des.data.pRect == nullptr) {
                des.data.pRect = new SGraphicsRectUnitData;
            }
            *des.data.pRect = *source.data.pRect;
            break;
        }
        case EllipseType: {
            if (source.data.pCircle != nullptr) {
                if (des.data.pCircle == nullptr) {
                    des.data.pCircle = new SGraphicsCircleUnitData;
                }
                *des.data.pCircle = *source.data.pCircle;
            } else {
                des.release();
            }
            break;
        }
        case TriangleType: {
            if (source.data.pTriangle != nullptr) {
                if (des.data.pTriangle == nullptr) {
                    des.data.pTriangle = new SGraphicsTriangleUnitData;
                }
                *des.data.pTriangle = *source.data.pTriangle;
            } else {
                des.release();
            }
            break;
        }
        case PolygonalStarType: {
            if (source.data.pPolygonStar == nullptr) {
                des.release();
                break;
            }
            if (des.data.pPolygonStar == nullptr) {
                des.data.pPolygonStar = new SGraphicsPolygonStarUnitData;
            }
            *des.data.pPolygonStar = *source.data.pPolygonStar;
            break;
        }
        case PolygonType: {
            if (source.data.pPolygon == nullptr) {
                des.release();
                break;
            }
            if (des.data.pPolygon == nullptr) {
                des.data.pPolygon = new SGraphicsPolygonUnitData;
            }
            *des.data.pPolygon = *source.data.pPolygon;
            break;
        }
        case LineType: {
            if (source.data.pLine == nullptr) {
                des.release();
                break;
            }
            if (des.data.pLine == nullptr) {
                des.data.pLine = new SGraphicsLineUnitData;
            }
            *des.data.pLine = *source.data.pLine;
            break;
        }
        case PenType: {
            if (source.data.pPen == nullptr) {
                des.release();
                break;
            }
            if (des.data.pPen == nullptr) {
                des.data.pPen = new SGraphicsPenUnitData;
            }
            *des.data.pPen = *source.data.pPen;
            break;
        }
        case TextType: {
            if (source.data.pText == nullptr) {
                des.release();
                break;
            }
            if (des.data.pText == nullptr) {
                des.data.pText = new SGraphicsTextUnitData;
            }
            *des.data.pText = *source.data.pText;
            break;
        }
        case PictureType: {
            if (source.data.pPic == nullptr) {
                des.release();
                break;
            }
            if (des.data.pPic == nullptr) {
                des.data.pPic = new SGraphicsPictureUnitData;
            }
            *des.data.pPic = *source.data.pPic;
            break;
        }
        case BlurType: {
            if (source.data.pBlur == nullptr) {
                des.release();
                break;
            }
            if (des.data.pBlur == nullptr) {
                des.data.pBlur = new SGraphicsBlurUnitData;
            }
            *des.data.pBlur = *source.data.pBlur;
            break;
        }
        case MgrType: {
            if (source.data.pGroup == nullptr) {
                des.release();
                break;
            }
            if (des.data.pGroup == nullptr) {
                des.data.pGroup = new SGraphicsGroupUnitData;
            }
            *des.data.pGroup = *source.data.pGroup;
            break;
        }
        default:
            break;
        };

        des.tail = source.tail;
        des.reson = source.reson;
    }


    friend  QDataStream &operator << (QDataStream &out, const CGraphicsUnit &graphicsUnitData)
    {
        out << graphicsUnitData.head;

        if (RectType == graphicsUnitData.head.dataType && nullptr != graphicsUnitData.data.pRect) {
            out << *(graphicsUnitData.data.pRect);
        } else if (EllipseType == graphicsUnitData.head.dataType && nullptr != graphicsUnitData.data.pCircle) {
            out << *(graphicsUnitData.data.pCircle);
        } else if (TriangleType == graphicsUnitData.head.dataType && nullptr != graphicsUnitData.data.pTriangle) {
            out << *(graphicsUnitData.data.pTriangle);
        } else if (PolygonType == graphicsUnitData.head.dataType && nullptr != graphicsUnitData.data.pPolygon) {
            out << *(graphicsUnitData.data.pPolygon);
        } else if (PolygonalStarType == graphicsUnitData.head.dataType && nullptr != graphicsUnitData.data.pPolygonStar) {
            out << *(graphicsUnitData.data.pPolygonStar);
        } else if (LineType == graphicsUnitData.head.dataType && nullptr != graphicsUnitData.data.pLine) {
            out << *(graphicsUnitData.data.pLine);
        } else if (TextType == graphicsUnitData.head.dataType && nullptr != graphicsUnitData.data.pText) {
            out << *(graphicsUnitData.data.pText);
        } else if (PictureType == graphicsUnitData.head.dataType && nullptr != graphicsUnitData.data.pPic) {
            out << *(graphicsUnitData.data.pPic);
        } else if (PenType == graphicsUnitData.head.dataType && nullptr != graphicsUnitData.data.pPen) {
            out << *(graphicsUnitData.data.pPen);
        } else if (BlurType == graphicsUnitData.head.dataType && nullptr != graphicsUnitData.data.pBlur) {
            out << *(graphicsUnitData.data.pBlur);
        } else if (MgrType == graphicsUnitData.head.dataType && nullptr != graphicsUnitData.data.pGroup) {
            out << *(graphicsUnitData.data.pGroup);
        } else if (DyLayer == graphicsUnitData.head.dataType && nullptr != graphicsUnitData.data.pDyLayer) {
            out << *(graphicsUnitData.data.pDyLayer);
        }

        out << graphicsUnitData.tail;
        return out;
    }

    friend QDataStream &operator>>(QDataStream &in, CGraphicsUnit &graphicsUnitData)
    {
        in >> graphicsUnitData.head;

        switch (graphicsUnitData.head.dataType) {
        case RectType: {
            qDebug() << "load rect item -------------- ";
            SGraphicsRectUnitData *pData = new  SGraphicsRectUnitData();
            in >> *pData;
            graphicsUnitData.data.pRect = pData;
            break;
        }
        case EllipseType: {
            SGraphicsCircleUnitData *pData = new  SGraphicsCircleUnitData();
            memset(pData, 0, sizeof(SGraphicsCircleUnitData));
            in >> *pData;
            graphicsUnitData.data.pCircle = pData;
            break;
        }
        case TriangleType: {
            SGraphicsTriangleUnitData *pData = new SGraphicsTriangleUnitData();
            in >> *pData;
            graphicsUnitData.data.pTriangle = pData;
            break;
        }
        case PolygonType: {
            SGraphicsPolygonUnitData *pData = new SGraphicsPolygonUnitData();
            in >> *pData;
            graphicsUnitData.data.pPolygon = pData;
            break;
        }
        case PolygonalStarType: {
            SGraphicsPolygonStarUnitData *pData = new SGraphicsPolygonStarUnitData();
            in >> *pData;
            graphicsUnitData.data.pPolygonStar = pData;
            break;
        }
        case LineType: {
            SGraphicsLineUnitData *pData = new SGraphicsLineUnitData();
            in >> *pData;
            graphicsUnitData.data.pLine = pData;
            break;
        }
        case TextType: {
            SGraphicsTextUnitData *pData = new SGraphicsTextUnitData();
            in >> *pData;
            graphicsUnitData.data.pText = pData;
            break;
        }
        case PictureType: {
            SGraphicsPictureUnitData *pData = new SGraphicsPictureUnitData();
            in >> *pData;
            graphicsUnitData.data.pPic = pData;
            break;
        }
        case PenType: {
            SGraphicsPenUnitData *pData = new SGraphicsPenUnitData();
            in >> *pData;
            graphicsUnitData.data.pPen = pData;
            break;
        }
        case BlurType: {
            SGraphicsBlurUnitData *pData = new SGraphicsBlurUnitData();
            in >> *pData;
            graphicsUnitData.data.pBlur = pData;
            break;
        }
        case MgrType: {
            EDdfVersion version = getVersion(in);
            if (version >= EDdf5_9_0_3_LATER) {
                SGraphicsGroupUnitData *pData = new SGraphicsGroupUnitData();
                in >> *pData;
                graphicsUnitData.data.pGroup = pData;
            }
            break;
        }
        case DyLayer: {
            EDdfVersion version = getVersion(in);
            if (version >= EDdf5_9_9_0_LATER) {
                SDynamicLayerUnitData *pData = new SDynamicLayerUnitData();
                in >> *pData;
                graphicsUnitData.data.pDyLayer = pData;
            }
            break;
        }
        default:
            break;

        }

        in >> graphicsUnitData.tail;

        return in;
    }

    void release()
    {
        switch (head.dataType) {
        case RectType: {
            if (data.pRect != nullptr) {
                delete data.pRect;
            }
            break;
        }
        case EllipseType: {
            if (data.pCircle != nullptr) {
                delete data.pCircle;
            }
            break;
        }
        case TriangleType: {
            if (data.pTriangle != nullptr) {
                delete data.pTriangle;
            }
            break;
        }
        case PolygonType: {
            if (data.pPolygon != nullptr) {
                delete data.pPolygon;
            }
            break;
        }
        case PolygonalStarType: {
            if (data.pPolygonStar != nullptr) {
                delete data.pPolygonStar;
            }
            break;
        }
        case LineType: {
            if (data.pLine != nullptr) {
                delete data.pLine;
            }
            break;
        }
        case TextType: {
            if (data.pText != nullptr) {
                delete data.pText;
            }
            break;
        }
        case PictureType: {
            if (data.pPic != nullptr) {
                delete data.pPic;
            }
            break;
        }
        case PenType: {
            if (data.pPen != nullptr) {
                delete data.pPen;
            }
            break;
        }
        case BlurType: {
            if (data.pBlur != nullptr) {
                delete data.pBlur;
            }
            break;
        }
        case MgrType: {
            if (data.pGroup != nullptr) {
                delete data.pGroup;
            }
            break;
        }
        case DyLayer: {
            if (data.pDyLayer != nullptr) {
                delete data.pDyLayer;
            }
            break;
        }
        default:
            break;

        }
        memset(&data, 0, sizeof(data));
    }
};
Q_DECLARE_METATYPE(CGraphicsUnit)

//整个图元数据
struct CGraphics {
    qint32        version = qint64(EDdfCurVersion);             //数据版本 默认给最新的版本枚举值 高位预留 低位由datastrem ddf版本组成
    qint64        unitCount = 0;                                //图元数量
    QRectF        rect;                                         // 画板大小和位置
    QVector<CGraphicsUnit> vecGraphicsUnit; //所有图元集合(不用保存到ddf)

    friend QDataStream &operator<<(QDataStream &out, const CGraphics &graphics)
    {
        qDebug() << "save to ddf, graphics.version = " << graphics.version << "graphics.unitCount = "
                 << graphics.unitCount << "raphics.rect = " << graphics.rect;

        out << static_cast<quint32>(0xA0B0C0D0);
        out << graphics.version;
        out << graphics.unitCount;
        out << graphics.rect;

        return out;
    }

    friend  QDataStream &operator>>(QDataStream &in, CGraphics &graphics)
    {
        EDdfVersion ver = getVersion(in);

        //兼容最早的版本(那个时候还没有版本标记 所以不用解析versions)
        if (ver > EDdf5_8_0_Base) {
            int flag;    //肯定为0xA0B0C0D0
            in >> flag;
            in >> graphics.version;
        } else {
            graphics.version = EDdf5_8_0_Base;
        }
        in >> graphics.unitCount;
        in >> graphics.rect;
        qDebug() << "get from ddf, graphics.version = " << graphics.version << "graphics.unitCount = "
                 << graphics.unitCount << "raphics.rect = " << graphics.rect;

        return in;
    }

//    static void recordMd5(QDataStream &out, const QByteArray &srcBinArry)
//    {
//        int srcSize =  srcBinArry.size();

//        //加密得到md5值
//        QByteArray md5 = QCryptographicHash::hash(srcBinArry, QCryptographicHash::Md5);

//        qDebug() << "---------- md5 size =============== " << md5.size() << " value = " << QString(md5).toUpper();

//        out << md5;
//    }
};
template<class T>
struct CBzGroupTree {
    QList<T>             bzItems;
    QList<CBzGroupTree>  childGroups;

    int groupType() const
    {
        if (data.data.pGroup != nullptr)
            return data.data.pGroup->groupType;

        return -1;
    }
    void setGroupType(int type)
    {
        if (data.data.pGroup != nullptr)
            data.data.pGroup->groupType = type;
    }

    CGraphicsUnit data = CGraphicsUnit();

    void *pGroup = nullptr;

    CBzGroupTree()
    {
        data.head.dataType = MgrType;
        data.data.pGroup = new SGraphicsGroupUnitData;
    }
    ~CBzGroupTree()
    {
        data.release();
    }

    //保证深复制
    CBzGroupTree(const CBzGroupTree &other): bzItems(other.bzItems),
        childGroups(other.childGroups), pGroup(other.pGroup)
    {
        CGraphicsUnit::deepCopy((this->data), other.data);
    }

    //保证深复制
    CBzGroupTree &operator=(const CBzGroupTree &other)
    {
        this->pGroup = other.pGroup;
        this->bzItems = other.bzItems;
        this->childGroups = other.childGroups;
        CGraphicsUnit::deepCopy((this->data), other.data);
        return *this;
    }
};
using CGroupBzItemsTreeInfo = CBzGroupTree<CGraphicsUnit>;

#pragma pack(pop)

#endif // SITEMDATA_H
