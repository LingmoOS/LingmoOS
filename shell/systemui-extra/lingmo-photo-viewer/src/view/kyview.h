#ifndef KYVIEW_H
#define KYVIEW_H
//#include <QKyView>
#include <QWidget>
#include <QApplication>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>
#include <QScreen>
#include <QGuiApplication>
#include <QPushButton>
#include <QLabel>
#include <QMouseEvent>
#include <QTimer>
#include <QGSettings>
#include <QStyleOption>
#include <QMimeData>
#include <QUrl>
#include <QMessageBox>

#include <QPinchGesture>
#include <QTouchEvent>
#include <QGestureEvent>
#include <QPanGesture>
#include <QSwipeGesture>
#include <QTapGesture>
#include <QImageReader>
#include <QLocale>
#include <QPainterPath>
#include <QPainter>
#include <QStyleOption>
#include <KF5/KWindowSystem/kwindowsystem.h>
#include <QThread>
#include "toolbar.h"
#include "titlebar.h"
#include "openimage.h"
#include "showimagewidget.h"
#include "navigator.h"
#include "information.h"
#include "marking.h"
#include "brushsettings.h"
#include "sidebar.h"
#include "global/variable.h"
#include "controller/interaction.h"
#include "global/horizontalorverticalmode.h"
#include <system_information.hpp>
#include <user_manual.hpp>
#include <gsettings.hpp>
#define signals Q_SIGNALS
#include "kaboutdialog.h"
using namespace kdk;
class KyView : public QWidget
{
    Q_OBJECT
public:
    explicit KyView(const QStringList &args);
    ~KyView();
    static KyView *mutual; //指针类型静态成员变量
    QPoint leftPoint;
    QString m_locale;

protected:
    void changeEvent(QEvent *event);
    void closeEvent(QCloseEvent *event) override;
private:
    kdk::WindowId m_windowId;
    bool m_isFirst = true;
    int toolbarHeightValue;
    int titlebarHeightValue;
    double m_backWidHeight = 0;
    double m_backWidWidth = 0;
    QSize m_miniSize;
    void initInteraction();
    bool m_isNavigatorShow = false;   //记录导航栏在裁剪之前是否已经show出来了，默认为隐藏状态
    ToolBar *m_toolbar = nullptr;     //工具栏
    TitleBar *m_titlebar = nullptr;   //顶栏
    Navigator *m_navigator = nullptr; //导航器
    OpenImage *m_openImage = nullptr; //打开图片
    ShowImageWidget *m_showImageWidget = nullptr;  //展示图片
    Information *m_information = nullptr;          //信息窗口
    SideBar *m_sideBar = nullptr;                  //相册
    Marking *m_markWid = nullptr;                  //标注
    BrushSettings *m_brushSettings = nullptr;      //画笔设置窗口
    QGSettings *m_gsettingThemeData = nullptr;     //主题
    QGSettings *m_pGsettingControlTrans = nullptr; //控制面板透明度
    HorizontalOrVerticalMode *hOrVMode = nullptr;  //横竖屏Dbus接口和信号监听
    deviceMode hLayoutFlag = PCMode;               //横竖屏标记，默认横屏
    //定时器
    QTimer *m_timer;      //移入移出两栏区域定时器
    QTimer *m_timernavi;  //离开界面或进入显隐事件的定时器
    QTimer *m_timeNomove; //鼠标两秒不动
    QTimer *m_timerSidebar;//移入移出侧边栏区域定时器

    QLocale m_local;           //检测系统语言环境
    bool m_mousePress = false; //按下鼠标左键
    bool m_inforState = true;  //信息栏状态
    bool m_albumState = true;  //相册状态
    bool m_albumShow = true;   //切换图片时，相册状态
    bool m_titleState = true; //鼠标离开进入时，有关菜单下拉列表show时的问题，先默认没有离开界面
    double m_tran = 0.60;     //透明度
    bool m_timestart = false; //记录从标题栏到中间窗体的变化
    bool m_isLeave = true;

    bool m_isPcModeStarted = true; //记录是否是从pc模式启动看图

    QPoint m_pressPosRecord = QPoint(0, 0); //记录拖动时鼠标位置
    int m_rotateLevel = 0;                  //手势旋转级别
    qreal m_lastDistance = 1;               //上次手势距离
    qreal m_minDistance = 0.5;              //最小生效距离
    //放大缩小和旋转同时只能进行一项
    bool m_isRotate = false;
    bool m_isResize = false;
    bool m_panTriggered = false; //长按事件触发中
    QPointF m_touchPoint;        //滑动切换图片事件记录距离
    QGSettings *m_fontSize = nullptr;
    double m_detio = 1;
    QSize m_inforSize;
    bool m_isMaxScreen = false;       //上一次动作非最大化
    bool m_isCutting = false;         //裁剪模式
    bool m_isResponseDbClick = false; //判断是否需要响应双击最大化
    
	QPoint m_dragPos;            // 记录鼠标按下时的位置

    //标注-0910
    QPoint m_recordMarkOldPos;  //记录标注栏起始位置
    QPoint m_recordMarkNowPos;  //记录标注栏当前位置
    QPoint m_recordMarkMovePos; //记录标注栏移动间隔
    int m_recordBrushStartPos;  //记录标注栏小三角位置

    void initconnect();     //初始化连接
    void titlebarChange();  //改变顶栏位置和大小
    void openImageChange(); //改变中间打开图片位置和大小
    void showImageChange(); //改变图片展示的位置和大小
    void toolbarChange();   //改变工具栏位置和大小
    void naviChange();      //改变导航栏位置
    void inforChange();     //改变信息栏位置
    void albumChange();     //改变相册位置
    void albumPosChange();
    void hoverChange(int y); // hover态，顶栏和工具栏的状态
    void initGsetting();     //监听主题变化
    void themeChange();      //主题变化
    void transChange();      //控制面板变化
    void avoidChange();      //防止点击即切换的情况
    void markToolChange();
    //字体变化
    void lable2SetFontSizeSlot(double size);
    void initFontSize();
    void fullScreen();
    //标注-0910
    void markChange();                              //改变标注栏位置
    void brushSettingsChange();                     //改变画笔设置弹窗位置
    void toSetBrushPos(int startPos, int showPosX); //设置画笔设置窗口位置
    void toShowBrushWid(bool isShow, bool isText);  //点击标记工具栏按钮显示画笔设置窗口


    void mouseMoveEvent(QMouseEvent *event); // hover态时两栏和按钮状态--检测鼠标所在区域
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void resizeEvent(QResizeEvent *event);          //改变窗口尺寸，各控件大小位置变化
    void leaveEvent(QEvent *event);                 //鼠标离开，按钮、两栏隐藏
    void enterEvent(QEvent *event);                 //鼠标进入
    void paintEvent(QPaintEvent *event);            // 解决毛玻璃特效的问题
    void keyPressEvent(QKeyEvent *event);           // 键盘响应事件
    void keyReleaseEvent(QKeyEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event); //双击响应--全屏和还原
    void dragEnterEvent(QDragEnterEvent *event);    //文件拖拽显示事件--判断是否响应dropevent
    void dropEvent(QDropEvent *event);              //文件拖拽响应
    bool eventFilter(QObject *obj, QEvent *event);
    void dealMouseDouble();                         //鼠标双击事件处理

    void x11EventEnd(); //窗口移动事件


    //手势相关
    void initGrabGesture(); // 初始化手势
    bool event(QEvent *event);
    bool gestureEvent(QEvent *event);                    // 手势识别
    void pinchTriggered(QPinchGesture *gesture);         // 捏手势
    void tapAndHoldGesture(QTapAndHoldGesture *gesture); // 平移手势
    bool touchRotate(const qreal &lastAngle);            //触控旋转

    void setWindowSize();   //设置界面大小
    void setTitleBtnHide(); //设置三联按钮的显隐

    KAboutDialog *m_aboutDialog = nullptr;
    void initAboutDialog(); //初始化关于窗口
    bool m_needCLoseGrabGesture = false;
    void toolBarPostionSet(); //工具栏位置设置
    bool m_recordFirstOpen = true;
    bool m_clickController = false;
    void toolBarHideOrShow(bool needShow);
    bool m_recordSidebarStateBeforeOtherOperate = false; //侧栏
    bool m_recordInfoStateBeforeOtherOperate = false;    //信息栏
    void restoreAllBallState();
    bool m_nowFullScreen = false;
    QString  getAppVersion();
    QString m_keyState;
    QGSettings *panelsetting = nullptr;
    void connectToPanelSizeChange();
    QGSettings *systemGSettings = nullptr;

    bool m_isReplacing = false;
    void mattingChange();

    bool m_isScanning = false;  //黑白扫描
    void scannerChange();

    bool m_isMaxAndFull = false;    //最大化且全屏
    QRect m_origRect;               //最大化前的位置及大小

private Q_SLOTS:

    void changOrigSize(); //主界面最大化和还原
    void toShowImage();   //显示图片
    void showSidebar();   //判断是否显示相册
                          //    void defaultSidebar();//相册显示--先保留

    void delayHide();      //顶栏工具栏的延时隐藏
    void delayHide_navi(); //导航栏在鼠标离开界面时隐藏
    void delayHide_move(); //鼠标两秒不动，隐藏两栏
    void delayHide_sidebar();         //相册的延时隐藏
    void openState();      //  判断打开应用应该是什么状态
    void aboutShow();      //关于界面打开，两栏隐藏
    void startRename();    //处理重命名，显示两栏
    void showInforWid();   //展示或隐藏图片信息窗口
    void clearImage();     //无图片，需要返回默认界面
    void dealDoubleClick(bool isleaveOrNot);
    void hideTwoBar();                           //隐藏两栏，进入裁剪模式
    void exitCut();                              //退出裁剪
    void slotIntelHVModeChanged(deviceMode sig); //横竖屏切换的响应
    void viewOCRMode();                          //隐藏其他栏，进入OCR模式
    void exitOCRMode();                          //退出OCR模式
    void setWidDisplay(bool isDefaultDisplay);   //设置界面显示
    // 0910-标注
    void toShowMarkWid(); //打开标注工具栏--隐藏工具栏，信息栏，相册，左右按钮
    void exitSign();
    void finshSign();
    void undoOperate();
    void exitSignComplete(); //完全退出标注
    void needExitApp();
    void setSidebarState(int num);
    void getMouseStateFromQml();
    void handlePanelSizeChanged(int size);

    void initMattingConnect();
    void enterReplaceBackgroundMode(); //进入背景替换模式
    void exitReplaceBackgroundMode();  //退出背景替换模式

    void initScannerConnect();
    void enterScanImageMode(); //进入扫描黑白图片模式
    void exitScanImageMode();  //退出扫描黑白图片模式

Q_SIGNALS:
    void albumState(bool isOpen); //相册在主界面隐藏或show的状态需要确定
    void setFontSizeSignal(double);  //字体变化
    void changeInforSize(double fontSize, double detio, QSize sizeWid);
    void changeCutWidPos(); //改变裁剪按钮工具栏位置
};

#endif // KYVIEW_H
