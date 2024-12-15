// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GLODEFINE_H
#define GLODEFINE_H
#include <QGraphicsItem>
enum EDrawToolMode {
    noselected,     //未选中或无公共属性不显示
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
    cut,             //裁剪画板

    MoreTool
};
//enum EDrawToolMode {
//    noselected,     //未选中或无公共属性不显示
//    selection,      //选择
//    importPicture,  //导入图片
//    rectangle,      //矩形
//    ellipse,        //圆形
//    triangle,       //三角形
//    polygonalStar,  //多角星
//    polygon,        //多边形
//    line,           //线
//    pen,            //画笔
//    text,           //文本
//    blur,           //模糊
//    cut,             //裁剪画板
//    picture,          //图片工具
//    MoreTool
//};

// 显示工具栏所有属性
enum EDrawProperty { // 图片、文字、马赛克根据类型全部显示
    FillColor = 0, // 填充颜色
    LineWidth, // 线宽
    LineColor, // 描边颜色
    RectRadius, // 矩形圆角
    Anchors, // 锚点
    StarRadius, // 五角星圆角
    SideNumber, // 侧边数
    LineStartType, // 线段起点类型
    LineEndType, // 线段终点类型
    PenStartType, // 画笔起点类型
    PenEndType, // 画笔终点类型
    TextColorAlpha, //文本颜色透明度,需要放在颜色前面进行更新
    TextColor, // 文本颜色
    TextFont, // 文本字体
    TextHeavy, // 文本自重
    TextSize, // 文本大小
    FillColorAlpha, //填充颜色透明度
    LineColorAlpha, //描边颜色透明度
    Blurtype, //模糊类型
    BlurWidth, //模糊粗细
    PropertyCutSize, // 裁剪图元的大小
    PropertyCutType, // 裁剪图元的类型
    PropertyImageAdjustScence, // 图片图元自适应场景
//    PropertyImageRotationType, // 图片图元旋转类型
    PropertyImageFlipType,     // 图片图元翻转类型

    EDrawPropertyCount
};

enum ESelectMode {
    none,
    netSelect,
    move,
    size,
    rotate
};

enum DrawStatus {
    Fill,
    Stroke,
    TextFill
};

enum EPenType {
    straight,   // 直线
    arrow       // 箭头
};

enum ELineType {
    noneLine,     // 无
    normalRing,   // 空心圆
    soildRing,     // 实心圆
    normalArrow,  // 普通箭头
    soildArrow   // 实心箭头

};

enum EGraphicUserType {
    NoType = QGraphicsItem::UserType,      //公共选择没有公共属性的时候用
    RectType,      //矩形
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
    DyLayer,
    MgrType =  QGraphicsItem::UserType + 100,    //多选管理图元
    hightLightType      //高亮图元
};

enum ECutAttributeType {
    LineEditeAttribute,
    ButtonClickAttribute,
    NoneAttribute
};

enum ECutType {
    cut_1_1,        //1:1裁剪
    cut_2_3,        //2:3裁剪
    cut_8_5,        //8:5裁剪
    cut_16_9,       //16:9裁剪
    cut_free,        //自由裁剪
    cut_original,  //裁剪框恢复画板初始大小
    cut_count        //裁剪方式个数
};

enum EBlurEffect {
    BlurEffect = 0,
    MasicoEffect,
    UnknowEffect
};
Q_DECLARE_METATYPE(EBlurEffect)

enum ESaveDDFTriggerAction {
    QuitApp,             //保存完后退出画板
    LoadDDF,             //保存完后导入ＤＤＦ文件
    StartByDDF,          //通过DDF文件打开后弹出保存
    NewDrawingBoard,     //保存完后新建画板
    ImportPictrue,       //保存完后打开图片文件
    SaveAction           //正常保存,保存完后不做任何事
};

enum ERotationType { //图元旋转枚举
    NoRotationType,  // 不进行任何操作
    LeftRotate_90,     //左旋转90
    RightRotate_90,    //右旋转90
    FlipHorizontal, //水平镜像翻转
    FlipVertical    //垂直镜像翻转
};

enum EChangedPhase {
    EChangedBegin,       //一系列变化的开始(一般用于记录撤销undo点信息)
    EChangedUpdate,      //一系列变化中的过程值(一般用来预览显示，且不会入undo栈)
    EChangedFinished,    //一系列变化的结束(一般用于记录还原redo点信息)
    EChangedAbandon,     //一系列变化的结束，不同于EChangedFinished，一般是抛弃该轮变化，不再执行入栈

    EChanged             //发生了变化
};


enum EVarUndoOrRedo { UndoVar,
                      RedoVar,
                      VarTpCount
                    };

enum EZMoveType {EDownLayer, EUpLayer, EToGroup};

#define DECLAREPRIVATECLASS(classname) \
    public:\
    class classname##_private;\
    inline classname##_private* d_pri()const{return _pPrivate;}\
    private:\
    classname##_private* _pPrivate;\
    friend class classname##_private;

#include <QWidget>
static void setWgtAccesibleName(QWidget *w, const QString &name)
{
    if (w != nullptr) {
        w->setObjectName(name);
#ifdef ENABLE_ACCESSIBILITY
        w->setAccessibleName(name);
#endif
    }
}
#endif // GLODEFINE_H
