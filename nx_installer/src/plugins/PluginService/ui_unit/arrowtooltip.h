
// ArrowWidget.h
#ifndef ARROWWIDGET_H
#define ARROWWIDGET_H

#include <QWidget>
#include <QGraphicsDropShadowEffect>
#include <QLabel>

const int SHADOW_WIDTH = 30;                 // 窗口阴影宽度;
const int TRIANGLE_WIDTH = 12;               // 小三角的宽度;
const int TRIANGLE_HEIGHT = 10;              // 小三角的高度;
const int BORDER_RADIUS = 6;                 // 窗口边角的弧度;


class ArrowWidget : public QFrame
{
    Q_OBJECT
public:
    ArrowWidget(bool flag = true, QWidget* parent = 0);

    enum Derection{
        left,
        right,
        up,
        down
    };
    // 设置小三角起始位置;
    void setStartPos(int startX);
    // 设置小三角宽和高;
    void setTriangleInfo(int width, int height);
    // 设置展示文本
    void setText(QString s);
    // 设置小三角的位置
    void setDerection(Derection d);
    //
    QString text();
    // 比起左上角的位置  用户更关心小三角的尖尖的位置 重载move以便用户更容易定位气泡框的位置
    // x,y 是气泡窗口小贱贱的坐标
    void myMove(int x, int y);

protected:

    void paintEvent(QPaintEvent *);
    void mousePressEvent (QMouseEvent *);
private:
    // 小三角的偏移量;
    int m_offset;
    // 小三角的宽度;
    int m_triangleWidth;
    // 小三角高度;
    int m_triangleHeight;
    Derection derect;
    QLabel *tooltip_text;
    bool m_flag;



};

#endif // ARROWWIDGET_H


//#ifndef WIDGET_H
//#define WIDGET_H

//#include <QWidget>
//#include <QFrame>
//#include <QDialog>
//enum Derection{
//       Left,
//       Right,
//       Up,
//       Down
//   };
//class ArrowWidget : public QWidget
//{
//    Q_OBJECT

//public:

//    ArrowWidget(QWidget *parent = 0);

//    // 设置小三角起始位置;
//    void setStartPos(int startX);

//    // 设置小三角宽和高;
//    void setTriangleInfo(int width, int height);
//    void setDerection(Derection d);
//    // 设置中间区域widget;
//    void setCenterWidget(QWidget* widget);

//protected:
//    void paintEvent(QPaintEvent *);
//    void mousePressEvent (QMouseEvent *);
//private:
//    // 小三角起始位置;
//    int m_startX;
//    // 小三角的宽度;
//    int m_triangleWidth;
//    // 小三角高度;
//    int m_triangleHeight;
//    Derection derect;
//};

//#endif
