// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GLODEFINE_H
#define GLODEFINE_H

#include <QGraphicsItem>
#include <QtCore/QtGlobal>
#include <QSharedPointer>
#include <QWidget>

#include "config.h"

#if defined(DRAWSHARED_LIBRARY)
#  define DRAWLIB_EXPORT Q_DECL_EXPORT
#else
#  define DRAWLIB_EXPORT Q_DECL_IMPORT
#endif

enum EDdfVersion {
    EDdfUnknowed = -1,       //未知的版本(ddf被修改过)

    EDdf5_8_0_Base = 0,      //最原始的ddf版本
    EDdf5_8_0_9_1,           //添加了 [矩形属性,A0B0C0D0,version] 的ddf版本
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

    EDdf_DrawBorad,          //公共绘制库drawboard不再使用UnitTree进行数据保存，同时添加了layerunit

    EDdfVersionCount,

    EDdfCurVersion = EDdfVersionCount - 1  //最新的版本号(添加新的枚举必须添加到EDdfUnknowed和EDdfVersionCount之间)
};

enum EDrawToolMode {
    //checked tool (can handle tool events)
    ECheckedTool = -1,
    selection,      //选择
    picture,        //图片工具
    importPicture,  //导入图片
    rectangle,      //矩形
    ellipse,        //圆形
    triangle,       //三角形
    polygonalStar,  //多角星
    polygon,        //多边形
    line,           //线
    text,           //文本
    pen,            //画笔
    eraser,         //橡皮擦
    blur,           //模糊
    cut,            //裁剪画板
    EDrawComItemTool,

    //no checked tool (no need to handle tool events)
    ECustomNoCheckedTool = 1000,
    EUndoTool,
    ERedoTool,
    EClearTool,
    ESaveTool,
    EOpenTool,
    ECloseTool,

    MoreTool = 10000
};

enum PageObjectType {
    PageSceneObject,
    PageLayerObject,
    PageItemObject,
};


enum EItemType {
    NoType = QGraphicsItem::UserType,      //公共选择没有公共属性的时候用
    //inner page item
    RectType,           //矩形
    EllipseType,        //圆形
    TriangleType,       //三角形
    PolygonalStarType,  //多角星
    PolygonType,        //多边形
    LineType,           //线
    PenType,            //画笔
    TextType,           //文本
    PictureType,        //图片
    CutType,            //裁剪
    BlurType,           //模糊
    RasterItemType,
    LayerItemType,
    GroupItemType = NoType + 100, //组合图元
    SelectionItemType,
    TrashBinItemType,
    MaxInnerItemType = TrashBinItemType + 50000,

    //user item
    UserPageItem
};

enum EBlurEffect {
    BlurEffect = 0,
    MasicoEffect,
    UnknowEffect
};
Q_DECLARE_METATYPE(EBlurEffect)

enum EChangedPhase {
    EChangedBegin,       //一系列变化的开始(一般用于记录撤销undo点信息)
    EChangedUpdate,      //一系列变化中的过程值(一般用来预览显示，且不会入undo栈)
    EChangedFinished,    //一系列变化的结束(一般用于记录还原redo点信息)
    EChangedAbandon,     //一系列变化的结束，不同于EChangedFinished，一般是抛弃该轮变化，不再执行入栈

    EChanged             //发生了变化
};

enum MoveItemZType {DownItemZ, UpItemZ};

#define PRIVATECLASS(classname) \
    public:\
    class classname##_private;\
    inline classname##_private* d_##classname()const{return classname##_d.data();}\
    private:\
    QSharedPointer<classname##_private> classname##_d;\
    friend class classname##_private;


#define PRIVATESAFECLASS(classname) \
    public:\
    class classname##_private;\
    inline classname##_private* d_##classname()const{return classname##_d;}\
    private:\
    classname##_private* classname##_d = nullptr;\
    friend class classname##_private;


static void setWgtAccesibleName(QWidget *w, const QString &name)
{
    if (w != nullptr) {
        w->setObjectName(name);
#ifdef ENABLE_ACCESSIBILITY
        w->setAccessibleName(name);
#endif
    }
}
struct SBLurEffect {
    int width = 20;
    int type = 1;
};
Q_DECLARE_METATYPE(SBLurEffect)

//EDesSort降序（第一个为最顶层）   EAesSort升序（第一个为最底层）
enum ESortItemTp {EDesSort, EAesSort, ESortCount};

#define RCC_DRAWBASEPATH QString(":/icons/deepin/builtin/texts/")
#define RCC_CURSORPATH QString(":/cursorIcons/")


#define TRASHBINTYPE -55555

#define SPECIAL_TEXT "..."

#endif // GLODEFINE_H
