#ifndef REPLACEBACKGROUNDWIDGET_H
#define REPLACEBACKGROUNDWIDGET_H

#include <QWidget>
#include <QObject>
#include <QtDebug>
#include <QImage>
#include <QMessageBox>
#include <QColorDialog>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QAction>
#include <QPalette>
#include <QApplication>
#include <QScreen>
#include <QGuiApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QScrollArea>
#include <QGraphicsDropShadowEffect>
#include <QtQuickWidgets/QQuickWidget>
#include <QQuickItem>
#include <QQmlEngine>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QStackedWidget>
#include <QListWidget>
#include <QBuffer>
#include <kborderlessbutton.h>
#include <kpushbutton.h>
#include "controller/interaction.h"
#include "view/sizedate.h"
#include "global/variable.h"
#include "global/mattinginteractiveqml.h"
#include "colordialog.h"
#include "mattingimageprovider.h"
#include "windowmanager/windowmanager.h"

class ReplaceBackgroundWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ReplaceBackgroundWidget(QWidget *parent = nullptr);

    QWidget *m_showImageWidget = nullptr;     //展示图片wid
    QWidget *m_maskWidget  = nullptr;         //抠图遮罩
    QLabel *m_loadingLabel;                   //遮罩标签
    QString m_origImagePath = "";             //原图路径
    QString m_origFormat;                     //原图格式
    bool m_saveFlag = false;                  //判断是否生成透明背景图片
    bool m_isReplacing = false;               //图片替换中
    QLabel *m_showImage;
    QQuickWidget *m_showMattingImageQml = nullptr;

    void reMove(int W, int H);                //跟随主界面拉伸变化
    void changeQmlColor(QColor color);        //设置qml区域背景
    void closeColorDialog();

private:
    QStackedWidget *m_rightStackWidget = nullptr; //右侧抠图+裁剪堆叠布局
    QWidget *m_cutWidget = nullptr;
    QWidget *m_mattingWidget = nullptr;
    QWidget *m_mattingTopWidget = nullptr;
    QWidget *m_mattingMidWidget = nullptr;
    QWidget *m_mattingBottomWidget = nullptr;
    QWidget *m_colorCustomWidget = nullptr;
    QWidget *m_colorBtnWidget = nullptr;
    QWidget *m_picBtnWidget = nullptr;
    QScrollArea * m_scrollArea;

    QFrame *m_toolWid = nullptr;              //对比+工具栏
    QWidget *m_toolBtnWid = nullptr;          //工具栏按钮
    QWidget *m_zoomWid = nullptr;             //缩、放按钮以及百分比

    QHBoxLayout *m_toolLayout;                //对比+工具栏布局
    QHBoxLayout *m_toolBtnLayout;             //工具栏布局
    QVBoxLayout *m_mattingLayout;             //抠图布局
    QHBoxLayout *m_mattingTopLayout;          //抠图上面背景更换布局
    QVBoxLayout *m_mattingMidLayout;          //抠图中间纯色/图片按钮布局
    QHBoxLayout *m_mattingBottomLayout;       //抠图下面保存取消布局

    QHBoxLayout *m_colorCustomLayout;         //颜色label + 自定义布局
    QGridLayout *m_colorBtnLayout;            //颜色按钮布局
    QGridLayout *m_picBtnLayout;              //图片按钮布局
    QHBoxLayout *m_layout;                    //界面整体布局
    QHBoxLayout *m_customBtnLayout;

    QWidget *m_compareWidget = nullptr;
    kdk::KBorderlessButton *m_compare;        //对比
    kdk::KBorderlessButton *m_reduce;         //缩小
    QLabel *m_percentage;                     //百分比
    kdk::KBorderlessButton *m_enlarge;        //放大
    kdk::KBorderlessButton *m_originalSize;   //原尺寸
    kdk::KBorderlessButton *m_adaptiveWidget; //适应窗口
    kdk::KBorderlessButton *m_rotate;         //旋转
    kdk::KBorderlessButton *m_flipH;          //水平翻转
    kdk::KBorderlessButton *m_flipV;          //垂直翻转
    kdk::KBorderlessButton *m_cutImage;       //裁剪

    QLabel *m_bg;                             //背景更换label
    QLabel *m_bgLabel1;                       //背景更换图片
    QLabel *m_bgLabel2;                       //背景更换文字
    QLabel *m_pureColor;                      //纯色label
    QLabel *m_pic;                            //图片label
    QPushButton *m_customBtn;                 //自定义颜色按钮
    ColorDialog *m_colorDialog = nullptr;     //颜色选择器
    QLabel *m_customBtnLeft;
    QLabel *m_customBtnRight;
    QPushButton *m_transparent;               //透明
    QPushButton *m_white;                     //白
    QPushButton *m_grey;                      //灰
    QPushButton *m_dimGrey;                   //深灰
    QPushButton *m_dark;                      //黑
    QPushButton *m_blue;                      //蓝
    QPushButton *m_red;                       //红
    QPushButton *m_darkOrange;                //深橙
    QPushButton *m_orange;                    //橙
    QPushButton *m_green;                     //绿
    QPushButton *m_purple;                    //紫
    QPushButton *m_pink;                      //粉
    kdk::KPushButton *m_addPicBtn;            //添加图片按钮
    QPushButton *m_pic1;                      //默认图片1
    QPushButton *m_pic2;
    QPushButton *m_pic3;
    QPushButton *m_pic4;
    QPushButton *m_pic5;
    QList<QPushButton*> m_list;
    QMap<QPushButton*, QString> m_picBtnToPathMap;
    QPushButton *m_saveBtn;                   //保存按钮
    QPushButton *m_cancelBtn;                 //取消按钮
    QString m_colorHex = "#000000";

    QHBoxLayout *m_cutToolBtnLayout;
    QHBoxLayout *m_cutBtnLayout;
    QVBoxLayout *m_cutLayout;                 //尺寸裁剪布局
    QHBoxLayout *m_cutTopLayout;              //尺寸裁剪顶部布局
    QVBoxLayout *m_cutMidLayout;
    QVBoxLayout *m_sizeLayout;
    QVBoxLayout *m_cutBottomLayout;
    QVBoxLayout *m_scaleLayout;

    QWidget *m_cutToolBtnWid;
    QWidget *m_cutBtnWid = nullptr;
    QPushButton *m_cutCancel;
    QPushButton *m_cutSave;

    QWidget *m_cutTopWid = nullptr;           //尺寸裁剪顶部wid
    QWidget *m_cutMidWid = nullptr;
    QWidget *m_cutBottomWid = nullptr;
    QLabel *m_cutLabel1;                      //尺寸裁剪图片
    QLabel *m_cutLabel2;                      //尺寸裁剪文字
    QLabel *m_sizeLabel;                      //尺寸label
    QListWidget *m_sizeWid = nullptr;         //尺寸wid
    QListWidgetItem *m_lifeSize;              //原图尺寸
    QListWidgetItem *m_freeSize;              //自由尺寸
    QListWidgetItem *m_standardOneInch;       //标准一寸
    QListWidgetItem *m_standardTwoInch;       //标准二寸
    QListWidgetItem *m_smallOneInch;          //小一寸
    QListWidgetItem *m_smallTwoInch;          //小二寸
    QListWidgetItem *m_idCardSize;            //身份证照尺寸
    QLabel *m_scaleLabel;                     //比例label
    QListWidget *m_scaleWid = nullptr;        //比例wid
    QListWidgetItem *m_originalScale;         //原比例
    QListWidgetItem *m_fourToThree;           //4:3标准横屏
    QListWidgetItem *m_threeTofour;           //3:4标准竖屏
    QListWidgetItem *m_sixteenToNine;         //16:9视频横屏
    QListWidgetItem *m_nineToSixteen;         //9:16视屏竖屏
    QListWidgetItem *m_oneToOne;              //1:1正方形

    QVBoxLayout *m_loadingLayout;             //遮罩图标+文本布局
    QWidget *m_loadingWidget = nullptr;       //遮罩图标+文本wid
    QLabel *m_loadingIcon;                    //遮罩图标

    QString widRadius = QString("6px");
    QString normalIconPath = QString("sp1");
    QString nowThemeStyle;
    QImage m_replaceImage;                    //替换背景之后的图片
    QImage m_origImage;                       //原图
    QColor m_color;                           //当前颜色选择器选取的颜色
    int m_proportion = 100;                   //缩放比例-默认为100
    int m_lastProportion = 0;                 //上次缩放图片显示比例
    QSize m_showImageWidSize;                 //显示图片wid尺寸
    QSize m_tmpImageSize;                     //按比例缩放后的图片大小
    QMessageBox *m_msg;                       //保存错误提示
    int m_imageShowWay = 3;                   //图片显示方式，默认为自适应查看
    int m_adaptiveProportion;                 //图片自适应窗口的比例
    bool m_isCutting = false;
    bool m_needExecSlot = true;

    void setShadowEffect(QWidget *widget);    //绘制阴影
    void initMattingWidget();                 //初始化抠图wid
    void initCutWidget();                     //初始化裁剪wid
    void initLayout();                        //初始化布局
    void initConnect();                       //初始化信号连接
    void setColorBtnStyle();                  //设置纯色按钮样式
    void setBtnIcon();                        //设置按钮图标
    void toolBtnIconSize(QSize size);         //工具栏按钮图标尺寸设置
    void toolButtonSize(QSize size);          //工具栏按钮尺寸设置
    void colorButtonSize(QSize size);         //颜色按钮尺寸设置
    void pictureBtnSize(QSize size);          //图片按钮尺寸设置
    void pictureBtnIconSize(QSize size);      //图片按钮图标尺寸设置
    void platformType();
    void initGsetting();                      //监听主题
    void changeStyle();                       //根据主题改变样式表                
    void setFormatedText();                   //智能抠图中遮罩
    void creatImage(const int &proportion = -1,
                    bool defaultOpen = true); //生成图像，defaultOpen区分是否是无其他操作显示图片
    void defaultImage(int proportion, int defaultProportion); //默认显示图片原则
    void operateImage(int proportion, int defaultProportion); //进行操作显示图片原则
    bool judgeSizeIsZero(QSize size);                   //极小的图缩放后尺寸低于1的不处理
    void showImage();                                   //显示图片
    void comparePressed();                              //按压对比显示原图
    void compareReleased();                             //释放对比显示效果图

    QObject *m_qmlObj = nullptr;
    ShowMattingImage *m_showMattingImage = nullptr;
    void initQmlObject();                               //初始化qml
    bool getQmlObjAndJudgeNullptr(QObject *objParent, QObject **objFirst, QObject **objSecond,
                                  QObject **objThird, QObject **objFourth); //获取qml对象
    void qmlAndWidgetConnect(QObject *obj);             //qml与qwidget连接信号槽
    void changeQmlSizePos(int w, int h, int x, int y);  //设置qml区域位置和尺寸
    void changeQmlCutSelectionPos();  //设置qml裁剪区域位置
    void initQWidgetObject();

protected:
    bool eventFilter(QObject *obj, QEvent *event);
    void resizeEvent(QResizeEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event) override;
public Q_SLOTS:
    void startReplaceBackground();                      //开始背景更换
    void exitReplaceBackground();                       //退出背景更换
    void getReplaceAndOrigImage(QImage replaceImage, QImage origImage);
    void needSave();                                    //备份保存图片
    void reduceImage();                                 //缩小图片
    void enlargeImage();                                //放大图片
    void originalSize();                                //原始尺寸
    void adaptiveWidget();                              //适应窗口
    void rotate();                                      //逆时针旋转
    void flipH();                                       //水平翻转
    void flipV();                                       //垂直翻转
    void cutImage();                                    //裁剪
    void colorSelected(QColor color);                   //颜色选择器选择颜色
    void addPicture();                                  //添加背景图片
    void onSizeItemClicked(QListWidgetItem *item);      //尺寸item槽函数
    void onScaleItemClicked(QListWidgetItem *item);     //比例item槽函数
    void onSizeItemSelectionChanged();
    void onScaleItemSelectionChanged();
    void toMattingPage();                               //返回抠图界面
    void cutDone();                                     //裁剪完成
    void picReplaceBackground();                        //图片背景替换
    void setScale(QVariant scale);                      //设置缩放比例
    void getFlipAndOrigImage(QImage flipImage, QImage origImage);
    void needChangeOperateWay();
    void onCutFreeSize();
    void onCloseChildWindow();
Q_SIGNALS:
    void toBackShowImageWid();
    void backgroundColor(uint b, uint g, uint r);
};

#endif // REPLACEBACKGROUNDWIDGET_H
