#ifndef SCANIMAGEWIDGET_H
#define SCANIMAGEWIDGET_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QQuickWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <kborderlessbutton.h>
#include <QPushButton>
#include <QToolButton>
#include <QMenu>
#include <QAction>
#include <QGraphicsDropShadowEffect>
#include <QRadioButton>
#include <QButtonGroup>
#include <QDialog>
#include <QPrintDialog>
#include <QInputDialog>
#include <QFrame>
#include <kinputdialog.h>
#include <QFileDialog>
#include <QQuickItem>
#include <QBuffer>
#include <QMessageBox>
#include <QStackedWidget>
#include <QApplication>
#include "view/sizedate.h"
#include "global/variable.h"
#include "controller/core/coreinteraction.h"
#include "global/scannerinteractiveqml.h"
#include "./scannerimageprovider.h"

class ScanImageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ScanImageWidget(QWidget *parent = nullptr);
//  ~ScanImageWidget();

    //主界面
    QVBoxLayout *m_layout = nullptr;          //界面整体布局
    QHBoxLayout *m_toolLayout = nullptr;      //对比+工具栏布局
    QHBoxLayout *m_toolBtnLayout;             //工具栏布局
    QHBoxLayout *m_saveLayout = nullptr;

    QStackedWidget *m_bottomStackWidget = nullptr; //左侧工具栏区域的堆叠布局
    QFrame *m_toolWid = nullptr;              //对比+工具栏
    QWidget *m_toolBtnWid = nullptr;          //工具栏按钮
    QWidget *m_zoomWid = nullptr;             //缩、放按钮以及百分比
    QWidget *m_saveWid = nullptr;
    QWidget *m_showImageWidget  ;             //图片显示qml

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
    QLabel *m_line1;
    kdk::KBorderlessButton *m_watermark;       //水印
    kdk::KBorderlessButton *m_cutImage;        //裁剪
    kdk::KBorderlessButton *m_print;           //打印
    QLabel *m_line2;
    QPushButton *m_cancelBtn;                  //取消
    QPushButton *m_saveBtn;                    //保存按钮
    QToolButton *m_pullDownBtn;                //下拉菜单
    QLabel *m_line3;
    QMenu *m_saveMenu;                         //下拉菜单按钮
    QAction *m_savePDF;                        //保存为PDF
    QAction *m_saveImage;                      //保存为图片

    //扫描图片
    QImage m_origImage;                        //原图
    QImage m_scanImage;                        //扫描之后的图片
    QString m_imagePath = "";                  //原图路径
    QString m_savePath = "";                   //默认保存路径（包括文件名）

    //水印
    QImage m_watermarkImage;                    //添加水印后的扫描图片
    bool isWatermark = false;                   //图片是否添加水印
    QString lastwmcontent;                      //水印内容
    QStringList notsupportWaterMark;            //水印不支持保存原格式的
    kdk::KInputDialog * kinput = nullptr;       //添加水印对话框

   //qml显示
    QQuickWidget *m_showImageQml = nullptr;                     //qml显示界面
    ScannerImageProvider *m_ScannerImageProvider = nullptr;     //qml加载图片
    QObject *m_qmlObj = nullptr;                //qml根对象
    QObject *qmlScannerImageRect;               //子对象
    QObject *scannerImageLoader;
    QObject *scannerImageItem;
    QObject *targatImage;                       //扫描图片对象

    int m_imageShowWay = 3;                     //图片显示方式，默认为自适应
    int m_proportion = 100;                     //缩放显示比例
    int m_lastProportion = 0;                   //上次缩放图片显示比例
    int m_adaptiveProportion;                   //图片自适应窗口的显示比例

    QSize m_showImageWidSize;                   //显示图片窗口的尺寸
    QSize m_tmpImageSize;                       //按比例缩放后的图片大小
    QImage m_tempImg;                           //待处理图片
    bool m_isEnlarged = false;                  //图片显示窗口为整个窗口

    double m_tran = 0.65;                       //透明度

    //其他
    QString widRadius = QString("6px");
    QString normalIconPath = QString("sp1");
    QString nowThemeStyle;

    //遮罩
    QWidget *m_maskWidget  = nullptr;
    QLabel *m_loadingIco;
    QLabel *m_loadingText;
    QWidget *m_loadWidget = nullptr;

    //裁剪界面
    QWidget *m_cutToolWidget = nullptr;             //裁剪窗口
    QHBoxLayout *m_cutToolLayout;

    QWidget *m_cutBtnWid = nullptr;
    QHBoxLayout *m_cutBtnLayout = nullptr;
    QPushButton *m_cutCancel;
    QPushButton *m_cutSave;

    //全局打印界面
    QPrintDialog *m_globalPrintDialog = nullptr;
    QPrinter printer;
    //打印PDF界面
    bool isPrintPDF = false;
    QDialog *m_printDialog = nullptr;           //打印界面窗口
    QLabel *m_printShowImage = nullptr;         //打印图片显示
    QLabel *m_sizeText;
    QLabel *m_direText;
    QButtonGroup *m_SizeGroupBtn;
    QButtonGroup *m_DireGroupBtn;
    QRadioButton *m_sizeA4Btn ;                 //纸张大小选项-A4
    QRadioButton *m_sizeOrigBtn ;               //纸张大小选项-原图
    QRadioButton *m_direHBtn ;                  //纸张方向选项-水平
    QRadioButton *m_direVBtn ;                  //纸张方向选项-垂直
    QPushButton *m_cancelPrintBtn;              //取消打印按钮
    QPushButton *m_confirmPrintBtn;             //开始打印按钮

    QWidget *m_printTopwid = nullptr;
    QWidget *m_printMidwid = nullptr;
    QWidget *m_printBottomwid = nullptr;
    QVBoxLayout *m_prinLayout = nullptr;        //打印界面布局
    QVBoxLayout *m_printTopLayout = nullptr;
    QGridLayout *m_printMidLayout = nullptr;
    QVBoxLayout *m_sizeLayout = nullptr;
    QHBoxLayout *m_sizeBtnLayout = nullptr;
    QVBoxLayout *m_direLayout = nullptr;
    QHBoxLayout *m_direBtnLayout = nullptr;
    QHBoxLayout *m_printBottomLayout = nullptr;

    QPrinter *printerPDF = nullptr;             //PDf打印
    QImage m_printImage;                        //需要打印的图片

    //扫描窗口
    void setShadowEffect(QWidget *widget);
    void initComponents();
    void initLayout();                        //初始化布局
    void initConnect();                       //初始化信号连接
    void setBtnIcon();                        //设置按钮图标
    void toolBtnIconSize(QSize size);         //工具栏按钮图标尺寸设置
    void toolButtonSize(QSize size);          //工具栏按钮尺寸设置

    void platformType();
    void initGsetting();                      //监听主题
    void changeStyle();                       //根据主题改变样式表

    //其他窗口
    void initMaskWidget();                    //初始化遮罩Wid
    void setFormatedText(QSize size);         //智能扫描中
    void initCutWidget();                     //初始化裁剪wid
    void initPrintWidget();                   //初始化打印PDFwid

    //qml相关
    void initQmlObject();                     //初始化qml
    bool getQmlObjAndJudgeNullptr(QObject *objParent, QObject **objFirst, QObject **objSecond,
                                  QObject **objThird, QObject **objFourth);
    void qmlAndWidgetConnect(QObject *obj);
    void creatImage(const int &proportion = -1,bool defaultOpen = true);    //生成图像，defaultOpen区分是否是无其他操作显示图片
    void defaultImage(int proportion, int defaultProportion);               //默认显示图片原则
    void operateImage(int proportion, int defaultProportion);               //进行操作显示图片原则
    bool judgeSizeIsZero(QSize size);                                       //极小的图缩放后尺寸低于1的不处理
    void showImage();                                                       //显示图片

    void changeQmlSizePos(int w, int h, int x, int y);
    void changeQmlColor();
    void sizeChanged(QSize size, int x,int y);                              //设置图片显示窗口大小及位置

protected:
    bool eventFilter(QObject *obj, QEvent *event);
    void resizeEvent(QResizeEvent *event);
    void keyPressEvent(QKeyEvent *event);

public Q_SLOTS:
    void startScanImage(QImage image, QString path);    //开始扫描黑白件
    void exitScanImage();                               //退出扫描黑白件
    void needSave();                                    //保存图片
    void saveResult(bool result);                       //保存成功后返回主界面
    void reduceImage();                                 //缩小图片
    void enlargeImage();                                //放大图片
    void originalSize();                                //原始尺寸
    void adaptiveWidget();                              //适应窗口
    void rotate();                                      //逆时针旋转
    void flipH();                                       //水平翻转
    void flipV();                                       //垂直翻转
    void addWatermark();                                //水印
    void cutImage();                                    //裁剪
    void print();                                       //打印
    void showSaveMenu();                                //显示子菜单
    void saveAsPDF();                                   //保存为PDF

    void printSize(bool checked);                       //打印纸张大小选项改变
    void printDire(bool checked);                       //打印纸张方向选项改变
    void cancelPrint();                                 //取消打印
    void confirmPrint();                                //确认打印

    void getScanAndOrigImage(QImage scanImage, QImage origImage);   //返回显示扫描图片
    void getFlipAndOrigImage(QImage flipImage, QImage origImage);   //返回显示flip图片
    void setScale(QVariant scale);                      //设置缩放比例
    void needChangeOperateWay();
    void comparePressed();                              //长按对比显示原图
    void compareReleased();                             //释放显示当前扫描图片
    void zoomToChangeWid();                             //缩放时改变图片显示窗口大小

    void exitCut();                                     //退出裁剪
    void needSaveCut();                                 //需要保存(裁剪)
Q_SIGNALS:
    void toBackShowImageWid();
    void toolWidChanged();
};

#endif // SCANIMAGEWIDGET_H
