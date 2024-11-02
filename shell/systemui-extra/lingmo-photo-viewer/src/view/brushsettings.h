#ifndef BRUSHSETTINGS_H
#define BRUSHSETTINGS_H
#define FITTHEMEWINDOWA "org.lingmo.style"
#include <QWidget>
#include <QPainter>
#include <QLabel>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QMouseEvent>
#include <QButtonGroup>
#include <QDebug>
#include <QPoint>
#include <QComboBox>
#include <QGSettings>
#include <QGraphicsDropShadowEffect>
#include <QFontDatabase>
#include <QListView>
#include <QSpinBox>
#include <QList>
#include <QMap>
#include <QVector4D>
#include <QVariant>
#include <QPainterPath>
#include "sizedate.h"
#include "painterthick.h"
#include "global/variable.h"
class BrushSettings : public QWidget
{
    Q_OBJECT
public:
    explicit BrushSettings(QWidget *parent = nullptr);
    void setStartPos(int startX);                // 设置小三角起始位置;
    void setShowPos(int showPosX, int showPosY); //设置弹出位置
    void setSize(bool isTextOrNot);              //设置窗口尺寸--text窗口
    void setTextSettings();                      //设置text设置栏的qss
    MarkPainterSet recordDate;

private:
    QGSettings *m_pGsettingThemeData = nullptr;
    //左边按钮组
    QButtonGroup *m_buttonGroupLeft;
    //右边按钮组
    QButtonGroup *m_buttonGroupRight;
    //画笔粗细
    PainterThick *m_smallest; //最细
    PainterThick *m_thin;     //细
    PainterThick *m_medium;   //中等
    PainterThick *m_thick;    //粗
    //画笔颜色
    PainterThick *m_red;
    PainterThick *m_blue;
    PainterThick *m_green;
    PainterThick *m_yellow;
    PainterThick *m_pink;
    PainterThick *m_black;
    PainterThick *m_grey;
    PainterThick *m_white;
    QLabel *m_line;                          //分割线
    QGraphicsDropShadowEffect *shadowEffect; //阴影
    QComboBox *m_fontComBox;                 //字体下拉框
    QSpinBox *m_fontSizeComBox;              //字号下拉框
    QCheckBox *m_deleteLine;                 //删除线
    QCheckBox *m_underLine;                  //下划线
    QCheckBox *m_bold;                       //加粗
    QCheckBox *m_italics;                    //斜体
    QFont m_font;                            //设置字号
    QString m_themeChange;
    int m_startX;             // 小三角起始位置;
    int m_triangleWidth = 15; // 小三角的宽度;
    int m_triangleHeight = 7; // 小三角高度;
    int m_trianglePointy = 41;
    bool m_showType = false; //显示类型--区分text和其他按钮弹窗
    QRect m_brush = QRect(0, 0, 308 * 72 / 37 + 4, 65);
    QRect m_brushText = QRect(0, 0, 410 * 72 / 37 + 4, 65);
    void setComboxSize();                 //设置字号字体
    void init();                          //初始化控件
    void initLayout(bool isTextOrNot);    //布局
    void leftWidChange(bool isTextOrNot); //改变左边布局
    void setButtonGroup();                //设置互斥组
    void initConnect();                   //初始化信号槽
    //主题
    void dealSystemGsettingChange(const QString);
    void initGsetting();
    void recordPaintThick(int type, bool isChecked);                      //记录画笔粗细
    void recordPaintColor(int type, bool isChecked, QColor painterColor); //记录画笔颜色
    void resetFontComboxSize();                                           //设置字号框的尺寸
    void resetBtnSize();

private Q_SLOTS:

    void smallestClicked();                       //极细-3px
    void thinClicked();                           //细-5px
    void mediumClicked();                         //粗-9px
    void thickClicked();                          //最粗-13px
    void redClicked();                            //红色
    void blueClicked();                           //蓝色
    void greenClicked();                          //绿色
    void yellowClicked();                         //黄色
    void pinkClicked();                           //粉色
    void blackClicked();                          //黑色
    void greyClicked();                           //灰色
    void whiteClicked();                          //白色
    void deleteLineClicked();                     //删除线
    void underLineClicked();                      //下划线
    void boldClicked();                           //加粗
    void italicsClicked();                        //斜体
    void currFontCombox(QString currentFont);     //当前字体
    void currFontSizeCombox(int currentFontSize); //当前字号

protected:
    void paintEvent(QPaintEvent *event);
Q_SIGNALS:
};

#endif // BRUSHSETTINGS_H
