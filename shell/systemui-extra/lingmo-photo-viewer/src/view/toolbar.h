#ifndef TOOLBAR_H
#define TOOLBAR_H
#include <QWidget>
#include <QFrame>
#include <QDialog>
#include <QPushButton>
#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>
#include <QMouseEvent>
#include <QtMath>
#include <QPainter>
#include <QGSettings>
#include <QFont>
#include <QGraphicsDropShadowEffect>
#include "controller/interaction.h"
#include <explor-qt/file-operation-utils.h>
#include <QMessageBox>
//#include <kpressbutton.h>
#include <kborderlessbutton.h>
//#include <kpushbutton.h>
#include <ktoolbutton.h>
class ToolBar : public QWidget
{
    Q_OBJECT
public:
    explicit ToolBar(QWidget *parent = nullptr);
    void updateFileInfo(QFileInfo fileInfo);
    void changePerRate(QString num, QString path);
    void reduceImageSlot();  //缩小图片
    void enlargeImageSlot(); //放大图片
    void delImage();         //删除
    void changeStyle();
    void setButtonState(bool isOpenSuccess, bool isCanOperate); //设置工具栏按钮是否可点击--图片是否打开成功
    QFrame *g_tooleWid = nullptr;                               //布局
    void getImageType(bool canCut);
    void getSuffix(QString suffix); //拿图片的格式
    void enterSign();
    void labelbar(); //标注
    void platformType();
    void initControlQss(); //初始化布局

    void adaptiveWidget(); //适应窗口
    void rotate();      //旋转

    QMenu *m_editMenu;

private:
    QString widRadius = QString("6px");
    QString normalIconPath = QString("sp1");
    QHBoxLayout *m_toolLayout;
    kdk::KBorderlessButton *m_reduce;         //缩小
    QLabel *m_percentage;                     //百分比
    kdk::KBorderlessButton *m_enlarge;        //放大
    kdk::KBorderlessButton *m_originalSize;   //原始尺寸
    kdk::KBorderlessButton *m_adaptiveWidget; //适应图片
    kdk::KBorderlessButton *m_rotate;         //旋转
    kdk::KBorderlessButton *m_flipH;          //水平翻转
    kdk::KBorderlessButton *m_flipV;          //垂直翻转
    kdk::KBorderlessButton *m_cutImage;       //裁剪
    QLabel *m_line1;
    kdk::KBorderlessButton *m_labelbar; //标注栏
    QLabel *m_line2;
    kdk::KBorderlessButton *m_sidebar;     //侧边栏
    kdk::KBorderlessButton *m_information; //详细信息
    kdk::KBorderlessButton *m_delImage;    //删除图片

    QString m_judgmentOperationFlag = "" ;//判断翻转类型标杆
    bool m_ifRotate = false ; //判断是否进行过顺时针操作，默认为false
    int m_record = 0 ;

    QColor m_color = QColor(190, 190, 190);
    QBrush m_brush = QBrush(Qt::white);
    QWidget *m_zoomWid; //缩放按钮和比例窗口
    QFont m_ft;         //固定字号
    QString nowThemeStyle;
    QString m_imagePath;
    bool m_isOpenSuccess = false; //默认图片不进行操作，不是成功打开的状态
    bool m_isCanOperate = false;  //默认无权限，不能删除，不能操作
    QString m_imageSuffix = "";   //图片后缀
    void originalSize();          //原始尺寸

    void rotateN();     //逆时针旋转
    void flipH();       //水平翻转
    void flipV();       //垂直翻转
    void cutImage();    //裁剪
    void sidebar();     //侧边栏--相册
    void information(); //信息栏
    //字体变化
    void lable2SetFontSizeSlot(int size);
    void initFontSize();
    void initGsetting();
    void initConnect();
    void buttonSize(QSize acturalSize);
    void btnIconSize(QSize iconSize);

    QString checkBtnSize = "width: 24px; height: 24px;";
    void updateCheckBtnQss();

    //编辑功能
    kdk::KBorderlessButton *m_edit;       //编辑
    //QAction *m_filter;              //滤镜
    QAction *m_ocr;                 //文字识别
    QAction *m_scanner;             //扫描黑白件
    QAction *m_imageMatting;        //抠图换背景
    //void filter();                  //滤镜
    void getText();                 //文字识别
    void scanner();                 //扫描黑白件
    void matting();                 //抠图


private Q_SLOTS:
    void showEditMenu();
    void changeEditIcon(QPixmap pix);

Q_SIGNALS:
    void showInfor();            //主界面显示信息栏
    void showSidebar();          //主界面显示相册
    void isDelete(bool isDel);   //判断删除
    void setFontSizeSignal(int); //字体变化
    void hideTwoBar();           //两栏隐藏，进入裁剪模式
    void startCutting();
    void viewOCRMode();    // 通知view进入OCR模式
    void startGetText();   //通知showimagewidget做OCR模式对应的样式变换
    void startZoomImage(); //通知界面进行缩放图片
    //标注-0910
    void showMarkingWid();
    void markUpdateImage(QString path);

    //编辑功能
    void startScanner();
    void startMatting();

};

#endif // TOOLBAR_H
