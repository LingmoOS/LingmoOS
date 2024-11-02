#ifndef SHOWIMAGEWIDGET_H
#define SHOWIMAGEWIDGET_H
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QDebug>
#include <QHBoxLayout>
#include <QFileInfo>
#include <QClipboard>
#include <QProcess>
#include <QMovie>
#include <QGSettings>
#include <QPrinter>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QMouseEvent>
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>
#include "controller/interaction.h"
#include "cutwidget.h"
#include <QtQuickWidgets/QQuickWidget>
#include <QQuickItem>
#include <QQuickView>
#include <QQmlEngine>
#include <QQmlContext>
#include "imageproviderthumb.h"
#include "ocrresultwidget.h"
#include <buried_point.hpp>
#include <kborderlessbutton.h>
#include "multitifftoolwid.h"
#include "multitifftoolbtn.h"
#include "imageprovider.h"
#include "replacebackgroundwidget.h"
#include "scanimagewidget.h"
class ShowImageWidget : public QWidget
{
    Q_OBJECT
public:
    ReplaceBackgroundWidget *m_replaceBackgroundWid = nullptr;
    ScanImageWidget *m_scanImageWid = nullptr;
    ShowImageWidget(QWidget *parent = nullptr);
    ~ShowImageWidget();
    CutWIdget *m_cutWid = nullptr;
    QPushButton *g_next;       //下一张
    QPushButton *g_back;       //上一张
    bool g_buttonState = true; //用来判断只有一张图片时隐藏左右滑动按钮
    bool m_canSet = true;      //针对于动图，只响应第一次的结果显示。
    void reMove(int w, int h);
    void openImage(QString path);   //打开图片
    void nextImage();               //下一张
    void backImage();               //上一张
    void delayShow(bool isLoading); //图片加载过慢时，进行延时展示
    void exitCut();                 //退出裁剪
    void needSave();                //退出裁剪去保存
    void cutWidPosChange();         //裁剪工具栏位置变化
    bool btnWidState();             //按钮栏状态,方便主界面判断按键是否响应的状态
    void changeQmlcomponentColor(QColor color, double opacity);

    //标注
    void undoOperate();
    void exitSign();
    void finishSign();
    //    bool signSaveQuery();

    void setTiffToolbarPos(double posY);
    void initPrint();               //初始化打印

public Q_SLOTS:
    void albumSlot(bool isOpen);                    //判断是否发送相册展示的事件
    void isDelete(bool isDel);                      //判断是删除
    void albumChangeImage(bool isChange);           //相册切换图片
    void acceptPrint(QPrinter *printer);            //打印开始
    void startCutColor();                           //开始裁剪，界面颜色改变
    void exitCutColor();                            //退出裁剪，界面颜色改变
    void startGetText();                            //开始识别文字
    void setGetTextResult(QVector<QString> result); //显示识别结果
    void exitGetText();                             //退出识别文字
    void exportToTxt();                             // OCR识别结果导出为txt
    void saveAsOther();                             //另存为
    void cutBtnWidChange(QString themeTyep);        //裁剪的工具栏窗口样式设置
    int cutQueryWid();                              //裁剪还未保存弹窗
    void cutFinishUi();                             //裁剪完成界面变量重置
    void navigatorState(bool state);                //导航栏状态
    void navigatorMove(bool isDrageImage);          //是否正在拖拽导航栏
    void copy();                                    //复制
    void updateImagePath(QString newImagePath);     //更新改名后的图片路径

    void setScale(QVariant scale);           //工具栏显示缩放比
    void getDropImagePath(QVariant pathUrl); //获取拖入窗口的图片的路径，并打开该张图片
    void operatyImage();                     //区分动图，静图，tiff的显示方式
    void showNavigator(QVariant scale, QVariant leftUpImageX, QVariant leftUpImageY, QVariant visibleWidth,
                       QVariant visibleHeight);          //显示导航栏，并将必要参数传递给后端
    void needCloseNavigator();                           //需要关闭导航栏
    void needUpdateImagePosFromMainWid(bool needUpdate); //用来确定是否通知导航栏更新图片位置
    void needChangeImage(QVariant needChangeImage);      //切图-该函数暂时不需要了
    void noticeQmlAdjustImagePos(bool needRecord);       //通知qml更新图片位置

    void rectPaint(QVariant rect); /*标注 */
    void circlePaint(QVariant leftX, QVariant leftY, QVariant painterW, QVariant painterH);
    void linePaint(QVariant startPos, QVariant endPos);
    void arrowPaint(QVariant startPos, QVariant endPos, QVariant arrowStartPos, QVariant arrowEndPos);
    void pencilPaint(QVariant pointList);
    void markPaint(QVariant startPos, QVariant endPos, QVariant alpha);
    void textPaint(QVariant textContent, QVariant startPos, QVariant type);
    void blurPaint(QVariant blurRect); /*标注 */

    void zoomBuriedPoint();                                                 //缩放埋点
    void needExitApp();                                                     //退出应用
    void changeImageFromViewSwipe(QVariant imagePath, QVariant nextOrBack); //切图-story19807
    void copyFromQml();

    void enterReplaceBackground();
    void exitReplaceBackground();

    void enterScanImage();
    void exitScanImage();

private:
    QMessageBox *m_msg;
    QMessageBox *m_queryMsg;
    //    QWidget *imageWid;
    //    QHBoxLayout *imageLayout;
    QLabel *m_showImage; //用来展示图片的区域
    QQuickWidget *m_showImageQml = nullptr;

    QAction *m_copy;         //复制
    QAction *m_setDeskPaper; //设置为桌面壁纸
    QAction *m_setLockPaper; //设置为锁屏壁纸
    QAction *m_print;        //打印
    QAction *m_deleteImage;  //删除
    QAction *m_signImage;    //标注
    QAction *m_showInFile;   //在文件夹中显示
    QAction *m_reName;       //重命名
    QAction *m_saveAs;       //另存为
    QMenu *m_imageMenu;      //图片右键菜单

    QString m_path = "";      //打开文件夹的路径
    QString m_imagePath = ""; //打开的图片文件的路径
    QPixmap m_copyImage;      //留着复制可能用
    QString m_paperFormat;
    QPrintDialog *m_printDialog;
    QMovie *m_loadingMovie = nullptr;
    int m_imageShowWay = 3;        //图片显示方式，默认为自适应查看
    bool m_imageNeedUpdate = true; //图片需要更新--默认需要
    QPixmap m_showPixmap;
    QPoint m_zoomFocus;  //焦点坐标
    QPointF m_leftUpPos; //左上角坐标
    QPoint m_recordImagePos;
    QPixmap m_pic;
    int m_backProportion;
    double m_actualWidth = 0;
    double m_actualHeight = 0;
    bool m_addOrreduce = true;

    QSize m_displaySizeOfPicture; //正在显示的图片的尺寸，仅指大小不超过窗口的，默认以左上角为缩放起始点的图片
    QPointF m_actualClickPos;

    bool m_isCutting = false;
    int m_typeNum;
    bool m_isOpen = true;          //是否自动打开相册
    bool m_isDelete = false;       //判断是删除
    bool m_isOpenSuccess = true;   //判断是否打开失败，默认成功--true
    bool m_delOrNot = false;       //接收core的判断，是否可删除--默认不可以删除
    int m_proportion = 100;        //缩放比例-默认为100
    bool m_navigatorState = false; //记录导航栏状态，默认为false
    bool m_isDragImage = false;    //是拖拽图片的动作
    bool m_movieImage = false;     //是否是动图，默认不是
    void initInteraction();
    void openFinish(QVariant var); //打开结束
    void initConnect();
    void sideState(int num);   //需要根据条件判断侧栏的显示和大小相关信息
    bool imageNum(int number); //根据num决定界面显示
    void imageNUll();          //根据图片是否为空，显示转圈圈
    void showMovieImage();     //显示动图
    bool operatorBeforeShowImage(QPixmap pixmap);
    void imageMenu(QFileInfo info, QString imageSize, QString colorSpace); //根据图片类型刷新右键菜单内容


    //右键菜单的各功能

    void setDeskPaper();            //设置为桌面壁纸
    void setLockPaper();            //设置为锁屏壁纸
    void finishPrint(int result);   //返回值
    void deleteImage();             //删除图片
    void showInFile();              //在文件夹中显示
    void reName();                  //重命名
    void signImage();               //标注
    void abnormalJudgBeforeClick(); //点击右键菜单之前判断异常，设置选项状态

    void setMenuAction();                  //检查当前图片是否可设置为壁纸
    void setMenuEnable();                  //设置右键菜单按钮是否可点击
    void startWithOpenImage(QString path); //由图片打开
    void canDelete(bool delOrNot);         //是否可删除

    void showBthWid(bool isShow); //展示保存wid
    int positionCorrection(int windowSize, int pictureSize);

    void ocrMode(); //调整到ocr模式对应样式

    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event);
    QWidget *m_btnWid = nullptr;
    QPushButton *m_cancel = nullptr;
    QPushButton *m_save = nullptr;
    void cutBtnWidStyle();

    OCRResultWidget *m_ocrWid = nullptr;
    QWidget *m_ocrBtnWid = nullptr;
    kdk::KBorderlessButton *m_ocrCancel = nullptr;
    //    QPushButton *m_ocrCopy = nullptr; //sxs TODO:功能暂不实现
    //    QPushButton *m_ocrExportToDoc = nullptr;  //sxs TODO:功能暂不实现
    QPushButton *m_ocrExportToTxt = nullptr;
    void ocrBtnWidStyle();

    void stateUpdate(); //给工具栏，告知图片状态
    int makeSureHeightState();
    int m_widChangeHeight = 40;

    bool m_firstChangeSize = true; //首次启动时变更大小，默认为true


    QObject *m_qmlObj = nullptr;
    //    QObject *m_qmlImageRect = nullptr;
    //    QObject *m_imageLoderItem = nullptr;

    //    QObject *m_imageLoader = nullptr;
    //    QObject *m_imageItemLower = nullptr;
    //    QObject *m_targatImage = nullptr;

    QString m_backImagePath = "";
    void initQmlObject();
    void initQWidgetObject();
    void initMenu();
    void initCutWid();
    void initChangeImageBtn();
    void initOcrWid();
    void changeQmlSizePos(int w, int h, int x, int y);
    bool m_recordMovieBefore = false; //记录上一次是否是动图
    void setImagePos(QPointF leftUpPos);

    bool m_needUpdateImagePosFromMainWid = true; //显示导航栏时，是否向后端发送位置更新导航栏显示


    QList<QImage> m_imagePageList;              //图片的页数列表
    MultiTiffToolWid *m_imagePageWid = nullptr; //图片显示页
    MultiTiffToolBtn *m_leftPageBtn = nullptr;
    MultiTiffToolBtn *m_rightPageBtn = nullptr;
    QLabel *m_pageNum = nullptr;
    QHBoxLayout *m_pageLayout = nullptr;
    bool getQmlObjAndJudgeNullptr(QObject *objParent, QObject **objFirst, int isNormal = -1,
                                  QObject **objSecond = nullptr, QObject **objThird = nullptr,
                                  QObject **objFourth = nullptr);
    QTimer *m_timer;
    void realBuriedPoint();
    QString m_imageSize = "0x0";
    void previousTiffImage();
    void nextTiffImage();
    int m_tiffPage = 0;
    void multiTiffOperate();
    bool m_naviState = false; //导航栏状态
    void qmlActiveFocusChanged();
    void qmlAndWidgetConnectNormal(QObject *obj);
    void qmlAndWidgetConnectMovie(QObject *obj);
    void qmlAndWidgetConnectTiff(QObject *obj);
    void qmlAndWidgetDisconnectNormal();
    bool m_hasConnectedNormal = false;
    bool m_hasConnectedMovie = false;
    bool m_hasConnectedTiff = false;
    bool m_programeIsClosing = false;
    //-19807-滑动特效
    QStringList m_loadImageList; //每次需要加载的图片
    int m_imageTotalNum;         //图片总数量
    int m_currentImageIndex;     //当前图片的位置
    QString m_imageDelPath;      //需要删掉的图片路径
    QList<int> m_showImageIndex; //记录当前展示的5个item的所有index，用来加载图片用，与loadImageList是一一对应的
    QList<int> m_imageTypeMovieOrNormal; //记录图片是否是动静图
                                         //-19807-滑动特效
    bool m_copyFromQml = false;

    bool m_isReplacing = false;
    bool m_isScanning = false;

Q_SIGNALS:
    void perRate(QString num, QString path); //改变工具栏的缩小的百分比
    void toShowImage();                      //告诉主界面需要show和hide的控件
    void showNavi(QPixmap img);              //展示导航栏
    void changeInfor(QFileInfo info, QString imageSize, QString colorSpace, QString realname); //获得信息栏的信息
    void titleName(QString imageName, QString imagePath); //需要将图片名字和图片文件路径发送给标题栏显示
    void clearImage();             //删除完列表里的图片时需要清空界面，恢复默认状态
    void reduceChange(QPoint);     //滚轮对图片缩小，需要将此动作信号发送出去进行处理
    void enlargeChange(QPoint);    //滚轮对图片放大，需要将此动作信号发送出去进行处理
    void changeSideSize(int type); //确定相册的尺寸
    void toShowSide();             //当图片大于2张及以上，默认打开侧栏
    void toRename(int mode);       //重命名
    void isRename();               //正在重命名，需要告诉显示两栏
    void fileState(bool isOpenSuccess, bool isCanDel); //判断图片是否打开失败
    //    void toSelectHigh(bool isHigh);//判断删除时高亮的选择
    void changeDelState(bool canDel);       //改变按钮状态
    void needDelete();                      //删除
    void chooseChange(QPoint pos);          //裁剪时，发送鼠标位置
    void needExitCut();                     //退出裁剪
    void sendImageType(bool canCut);        //给工具栏发消息告知图片类型
    void sendImageToProvider(QImage image); //给qml图片提供引擎图片
    void viewExitOCRMode();
    void startWayToSetTitleStyle(); //从相册打开看图
    void sendSuffixToToolbar(QString suffix);

    void decideMainWidDisplay(bool isDefaultDisplay); //确定主界面显示状态

    void needGrabGestureOut(bool needGrabGestureOut);
    void toCloseNavigator(QPixmap image);
    void doubleEventToMainWid();

    //标注
    void exitSignComplete(); //退出标注
    void undoSign();
    void enterSign();
    void startSignToToolbar();


    // story-19807
    void adaptiveWidgetSignal();
    void sendMousePressOrNot();
    // story-19807

    void initMattingConnect();
    void toReplaceBackground();   //开始背景替换
    void quitReplaceBackground(); //退出背景替换
    void toShowOrigImage();

    void initScannerConnect();
    void toScanImage();   //开始扫描黑白件
    void quitScanImage(); //退出扫描黑白件
};

#endif // SHOWIMAGEWIDGET_H
