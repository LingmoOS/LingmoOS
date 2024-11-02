#ifndef MARKING_H
#define MARKING_H
#define FITTHEMEWINDOWB "org.lingmo.style"
#include <QWidget>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QVariant>
#include <QDebug>
#include <QGSettings>
#include <QCheckBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QMap>
#include <QGraphicsDropShadowEffect>
#include "sizedate.h"
#include "global/variable.h"
class Marking : public QWidget
{
    Q_OBJECT
public:
    explicit Marking(QWidget *parent = nullptr);
    void changeStyle();
    QList<QCheckBox *> m_list; //存每一个按钮，设置按钮互斥
    MarkPainterSet recordDate;
    //    QMap<QCheckBox*,QString> m_map;//方便判断每个按钮的状态
    void clearButtonState(); //清除按钮状态

    void resetMarkWidQss();
    bool getTextState(); //返回当前是否时text的标注模式

private:
    QGSettings *m_pGsettingThemeData = nullptr;
    QHBoxLayout *m_layout; //水平布局
    QWidget *m_layoutWid;
    QButtonGroup *m_buttonGroup;
    QCheckBox *m_rectangle;   //矩形
    QCheckBox *m_ellipse;     //椭圆形
    QCheckBox *m_segment;     //线段
    QCheckBox *m_arrow;       //箭头
    QCheckBox *m_paintBrush;  //画笔
    QCheckBox *m_markArticle; // mark条
    QCheckBox *m_text;        //文字
    QCheckBox *m_blur;        //模糊
    QPushButton *m_undo;      //撤销
    QPushButton *m_exit;      //退出标注栏
    QPushButton *m_finish;    //完成
    QLabel *m_line;           //竖线
    QString checkBtnSize = "width: 24px; height: 24px;";
    QString widRadius = "4px;";
    bool isSelected = false;
    void dealSystemGsettingChange(const QString);
    void initGsetting();
    int m_triangleStartX = 95;
    int m_triangleWidth = 15;
    int m_positionCoefficient = 0; //其实位置系数

    void toSetLayout(); //设置布局
    void setWidAndBtnSize();
    void initConnect();                                          //初始化信号槽
    void setQSS();                                               //设置样式
    void buttonGroup(bool isChecked, const QCheckBox *checkbox); //设置为单选按钮组
    void recordButtonState(int type, bool buttonState);

Q_SIGNALS:
    void showSetBar(bool isShow, bool isText);   //显示画笔设置窗口
    void showSetPos(int startPos, int showPosX); //显示画笔设置窗口位置
    void exitSign();
    void finishSign();
    void undoSignal();

private Q_SLOTS:
    void rectangle();   //矩形
    void ellipse();     //椭圆
    void segment();     //线段
    void arrow();       //箭头
    void paintBrush();  //画笔
    void markArticle(); // mark条
    void text();        //文字
    void blur();        //模糊
    void undo();        //撤销
    void exit();        //退出标注
    void finish();      //完成
};

#endif // MARKING_H
