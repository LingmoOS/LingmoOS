
// ArrowWidget.cpp
#include "arrowtooltip.h"

#include <QWidget>
#include <QHBoxLayout>
#include <QPoint>
#include <QPainter>
#include <QPainterPath>
#include <QImage>
#include <QVariant>

ArrowWidget::ArrowWidget(bool flag, QWidget *parent):
    QFrame(parent),
    m_offset(12),
    m_triangleWidth(TRIANGLE_WIDTH),
    m_triangleHeight(TRIANGLE_HEIGHT),
    m_flag(true)
{
    m_flag = flag;
//    setWindowFlags(Qt::FramelessWindowHint |
//                   Qt::WindowStaysOnTopHint |
//                   Qt::WindowDoesNotAcceptFocus);
    setWindowFlags(Qt::ToolTip);
    setAttribute(Qt::WA_TranslucentBackground);
    //    setWindowModality(Qt::NonModal);
//    this->setStyleSheet("QWidget{border-radius: 6px;background:#FFFFFF;font-size:14px;padding: 6px 6px 6px 6px;}");

    QHBoxLayout* hMainLayout = new QHBoxLayout;
    hMainLayout->setContentsMargins(0,0,0,0);
    setLayout(hMainLayout);

    tooltip_text = new QLabel();
    tooltip_text->setObjectName("tooltip_text");
    tooltip_text->setProperty("class", "font14px");

    tooltip_text->setWordWrap(true);
    tooltip_text->setAlignment(Qt::AlignLeft);
    if(m_flag) {

       tooltip_text->setStyleSheet("QWidget{border-radius: 4px;background:#FFFFFF;padding: 5px 5px 5px 5px;color:#000000;font-size:14px;}");
    } else
        tooltip_text->setStyleSheet("QWidget{border-radius: 4px;padding: 6px 6px 6px 6px;color:#FFFFFF;font-size:14px;}");

    hMainLayout->addWidget(tooltip_text);
}

// 设置小三角显示的起始位置;
void ArrowWidget::setStartPos(int startX)
{
    m_offset = startX;
    repaint();
}
void ArrowWidget::setTriangleInfo(int width, int height)
{
    m_triangleWidth = width;
    m_triangleHeight = height;
}

void ArrowWidget::setText(QString s)
{
    tooltip_text->setText(s);
    adjustSize();
}

void ArrowWidget::setDerection(ArrowWidget::Derection d)
{
    derect = d;
}

QString ArrowWidget::text()
{
    return tooltip_text->text();
}

void ArrowWidget::myMove(int x, int y)
{
    int top_left_x, top_left_y;
    switch (derect) {
    case down:
        top_left_x = x - m_offset - m_triangleWidth / 2 - tooltip_text->x();
        top_left_y = y - m_triangleHeight - tooltip_text->height() - tooltip_text->y();
        move(QPoint(top_left_x, top_left_y));
        break;
    case up:
        top_left_x = x - m_offset - m_triangleWidth / 2 - tooltip_text->x();
        top_left_y = y + m_triangleHeight - tooltip_text->y();
        move(QPoint(top_left_x, top_left_y));
        break;
    case left:
        top_left_x = x + m_triangleHeight - tooltip_text->x();
        top_left_y = y - m_offset - m_triangleWidth / 2 - tooltip_text->y();
        move(QPoint(top_left_x, top_left_y));
        break;
    case right:
        top_left_x = x - m_triangleHeight - tooltip_text->width() - tooltip_text->x();
        top_left_y = y - m_triangleWidth / 2 - m_offset - tooltip_text->y();
        move(QPoint(top_left_x, top_left_y));
        break;
    default:
        break;
    }
}

void ArrowWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 38));

    QPainterPath drawPath;
    // 小三角区域;
    QPolygon trianglePolygon;

    QRect myRect(tooltip_text->x(), tooltip_text->y(), tooltip_text->width(), tooltip_text->height());

    // 设置小三的具体位置
    int tri_pos_x, tri_pos_y;
    switch (derect)
    {
    case up:{
        // 小三角左边的点的位置
        tri_pos_x = myRect.x() + m_offset;
        tri_pos_y = myRect.y();
        trianglePolygon << QPoint(tri_pos_x, tri_pos_y);
        trianglePolygon << QPoint(tri_pos_x + m_triangleWidth, tri_pos_y);
        trianglePolygon << QPoint(tri_pos_x + m_triangleWidth / 2, tri_pos_y - m_triangleHeight);
    }
        break;
    case left:{
        // 小三上边点的位置
        tri_pos_x = myRect.x();
        tri_pos_y = myRect.y() + m_offset;
        trianglePolygon << QPoint(tri_pos_x, tri_pos_y);
        trianglePolygon << QPoint(tri_pos_x - m_triangleHeight, tri_pos_y + m_triangleWidth / 2);
        trianglePolygon << QPoint(tri_pos_x, tri_pos_y + m_triangleWidth);
    }
        break;
    case right:{
        // 小三上边点的位置
        tri_pos_x = myRect.x() + myRect.width();
        tri_pos_y = myRect.y() + m_offset;
        trianglePolygon << QPoint(tri_pos_x, tri_pos_y);
        trianglePolygon << QPoint(tri_pos_x + m_triangleHeight, tri_pos_y + m_triangleWidth / 2);
        trianglePolygon << QPoint(tri_pos_x, tri_pos_y + m_triangleWidth);
    }

        break;
    case down:{
        // 小三左边点的位置
        tri_pos_x = myRect.x() + m_offset;
        tri_pos_y = myRect.y() + myRect.height();
        trianglePolygon << QPoint(tri_pos_x, tri_pos_y);
        trianglePolygon << QPoint(tri_pos_x + m_triangleWidth / 2, tri_pos_y + m_triangleHeight);
        trianglePolygon << QPoint(tri_pos_x + m_triangleWidth, tri_pos_y);
    }
        break;
    default:
        break;
    }
    drawPath.addRoundedRect(myRect, BORDER_RADIUS, BORDER_RADIUS);
    if(!m_flag)
         drawPath.addPolygon(trianglePolygon);
    painter.drawPath(drawPath);
}

void ArrowWidget::mousePressEvent(QMouseEvent *)
{
     this->close ();
}


//#include "arrowtooltip.h"
//#include <QHBoxLayout>
//#include <QPainter>
//#include <QGraphicsDropShadowEffect>

//#define SHADOW_WIDTH 15                 // 窗口阴影宽度;
//#define TRIANGLE_WIDTH 15               // 小三角的宽度;
//#define TRIANGLE_HEIGHT 10              // 小三角的高度;
//#define BORDER_RADIUS 5                 // 窗口边角的弧度;

//ArrowWidget::ArrowWidget(QWidget *parent)
//    : QWidget(parent)
//    , m_startX(50)
//    , m_triangleWidth(TRIANGLE_WIDTH)
//    , m_triangleHeight(TRIANGLE_HEIGHT)
//{
//    setWindowFlags(Qt::FramelessWindowHint);
//    setAttribute(Qt::WA_TranslucentBackground);
//    this->setObjectName("ArrowWidget");

//    // 设置阴影边框;
////    auto shadowEffect = new QGraphicsDropShadowEffect(this);
////    shadowEffect->setOffset(0, 0);
////    shadowEffect->setColor(Qt::gray);
////    shadowEffect->setBlurRadius(SHADOW_WIDTH);
////    this->setGraphicsEffect(shadowEffect);

//    this->setStyleSheet("QWidget{background: rgba(60, 90, 90, 0.5);border-radius: 6px;color:#FFFFFF;font-size:18;}");


//}
//void ArrowWidget::setDerection(Derection d)
//{
//    derect = d;
//}
//void ArrowWidget::setCenterWidget(QWidget* widget)
//{
//    QHBoxLayout* hMainLayout = new QHBoxLayout(this);
//    this->setLayout(hMainLayout);
//    hMainLayout->addWidget(widget);
//    hMainLayout->setSpacing(0);
//    hMainLayout->setContentsMargins(SHADOW_WIDTH,0,0,0);
////    hMainLayout->setContentsMargins(SHADOW_WIDTH , SHADOW_WIDTH + TRIANGLE_HEIGHT, SHADOW_WIDTH, SHADOW_WIDTH);
//}

//// 设置小三角显示的起始位置;
//void ArrowWidget::setStartPos(int startX)
//{
//    m_startX = startX;
//}

//void ArrowWidget::setTriangleInfo(int width, int height)
//{
//    m_triangleWidth = width;
//    m_triangleHeight = height;
//}

//void ArrowWidget::paintEvent(QPaintEvent *)
//{
///*    QPainter painter(this);
//    painter.setRenderHint(QPainter::Antialiasing, true);
//    painter.setPen(Qt::NoPen);
//    painter.setBrush(QColor(255, 255, 255, 200));

//    QPainterPath drawPath;
//    QPolygon trianglePolygon;

//    switch (derect) {
//    case Up:
//        trianglePolygon << QPoint(m_startX, m_triangleHeight + SHADOW_WIDTH);
//        trianglePolygon << QPoint(m_startX + m_triangleWidth / 2,SHADOW_WIDTH);
//        trianglePolygon << QPoint(m_startX + m_triangleWidth, m_triangleHeight + SHADOW_WIDTH);
//        drawPath.addRoundedRect(QRect(SHADOW_WIDTH, m_triangleHeight + SHADOW_WIDTH,
//                                      width() - SHADOW_WIDTH * 2, height() - SHADOW_WIDTH*2 - m_triangleHeight),
//                                      BORDER_RADIUS, BORDER_RADIUS);
//        break;
//    case Left:
//        trianglePolygon << QPoint(3 + m_triangleHeight, 43);
//        trianglePolygon << QPoint(8, 48 + m_triangleWidth / 2);
//        trianglePolygon << QPoint(3 + m_triangleHeight, 39 + m_triangleWidth);
//        drawPath.addRoundedRect(QRect(SHADOW_WIDTH, m_triangleHeight + SHADOW_WIDTH,
//                                      width() - SHADOW_WIDTH * 2, height() - SHADOW_WIDTH * 2 - m_triangleHeight),
//                                      BORDER_RADIUS, BORDER_RADIUS);
//        break;
//        case Right:
//        trianglePolygon << QPoint(115, 40);
//        trianglePolygon << QPoint(116 + m_triangleHeight, 48 + m_triangleWidth / 2);
//        trianglePolygon << QPoint(115, 39 + m_triangleWidth);
//        drawPath.addRoundedRect(QRect(SHADOW_WIDTH, m_triangleHeight + SHADOW_WIDTH,
//                                    width() - SHADOW_WIDTH * 2, height() - SHADOW_WIDTH * 2 - m_triangleHeight),
//                                    BORDER_RADIUS, BORDER_RADIUS);

//        break;
//        case Down:
//        trianglePolygon << QPoint(m_startX, 65);
//        trianglePolygon << QPoint(m_startX + m_triangleWidth / 2, 65 + m_triangleHeight);
//        trianglePolygon << QPoint(m_startX + m_triangleWidth, 65);
//        drawPath.addRoundedRect(QRect(SHADOW_WIDTH, m_triangleHeight + SHADOW_WIDTH,
//                                    width() - SHADOW_WIDTH * 2, height() - SHADOW_WIDTH * 2 - m_triangleHeight),
//                                    BORDER_RADIUS, BORDER_RADIUS);

//    default:
//        break;
//    }
//    drawPath.addPolygon(trianglePolygon);
//    painter.drawPath(drawPath);*/
///*    QPainter painter(this);
//    painter.setRenderHint(QPainter::Antialiasing, true);
//    painter.setPen(Qt::NoPen);
//    painter.setBrush(QColor(255,255,255));

//    // 小三角区域;
//    QPolygon trianglePolygon;
//    trianglePolygon << QPoint(m_startX, 65);
//    trianglePolygon << QPoint(m_startX + m_triangleWidth / 2, 65 + m_triangleHeight);
//    trianglePolygon << QPoint(m_startX + m_triangleWidth, 65);

//    QPainterPath drawPath;
//    drawPath.addRoundedRect(QRect(SHADOW_WIDTH, m_triangleHeight + SHADOW_WIDTH,
//                                       width() - SHADOW_WIDTH * 2, height() - SHADOW_WIDTH * 2 - m_triangleHeight),
//                                       BORDER_RADIUS, BORDER_RADIUS);
//    // Rect + Triangle;
//    drawPath.addPolygon(trianglePolygon);
//    painter.drawPath(drawPath);*/
//    // 小三角区域;
//    QPainter painter(this);
//    painter.setRenderHint(QPainter::Antialiasing, true);
//    painter.setPen(Qt::NoPen);
//    painter.setBrush(QColor(60, 90, 90));

////    // 小三角区域;
////    QPolygon trianglePolygon;
////    trianglePolygon << QPoint(m_startX, 65);
////    trianglePolygon << QPoint(m_startX + m_triangleWidth / 2, 65 + m_triangleHeight);
////    trianglePolygon << QPoint(m_startX + m_triangleWidth, 65);

////    QPainterPath drawPath;
////    drawPath.addRoundedRect(QRect(SHADOW_WIDTH, m_triangleHeight + SHADOW_WIDTH,
////                                      width() - SHADOW_WIDTH * 2, height() - SHADOW_WIDTH * 2 - m_triangleHeight),
////                                      BORDER_RADIUS, BORDER_RADIUS);
//    // Rect + Triangle;
//    QPainterPath drawPath;
//    QPolygon trianglePolygon;

//    switch (derect) {
//    case Up:
//        trianglePolygon << QPoint(m_startX, m_triangleHeight + SHADOW_WIDTH);
//        trianglePolygon << QPoint(m_startX + m_triangleWidth / 2,SHADOW_WIDTH);
//        trianglePolygon << QPoint(m_startX + m_triangleWidth, m_triangleHeight + SHADOW_WIDTH);
////        drawPath.addRoundedRect(QRect(SHADOW_WIDTH, m_triangleHeight + SHADOW_WIDTH,
////                                      width() - SHADOW_WIDTH * 2, height() - SHADOW_WIDTH*2 - m_triangleHeight),
////                                      BORDER_RADIUS, BORDER_RADIUS);
//        drawPath.addRoundedRect(QRect(SHADOW_WIDTH , m_triangleHeight,
//                                    width() - m_triangleHeight, height()),
//                                    BORDER_RADIUS, BORDER_RADIUS);
//        break;
//    case Left:
//        trianglePolygon << QPoint(3 + m_triangleHeight, 43);
//        trianglePolygon << QPoint(8, 48 + m_triangleWidth / 2);
//        trianglePolygon << QPoint(3 + m_triangleHeight, 39 + m_triangleWidth);
////        drawPath.addRoundedRect(QRect(SHADOW_WIDTH, m_triangleHeight + SHADOW_WIDTH,
////                                      width() - SHADOW_WIDTH * 2, height() - SHADOW_WIDTH * 2 - m_triangleHeight),
////                                      BORDER_RADIUS, BORDER_RADIUS);
//        drawPath.addRoundedRect(QRect(SHADOW_WIDTH + m_triangleWidth, 0,
//                                    width(), height()),
//                                    BORDER_RADIUS, BORDER_RADIUS);
//        break;
//        case Right:
//        trianglePolygon << QPoint(115, 40);
//        trianglePolygon << QPoint(116 + m_triangleHeight, 48 + m_triangleWidth / 2);
//        trianglePolygon << QPoint(115, 39 + m_triangleWidth);
////        drawPath.addRoundedRect(QRect(SHADOW_WIDTH, m_triangleHeight + SHADOW_WIDTH,
////                                    width() - SHADOW_WIDTH * 2, height() - SHADOW_WIDTH * 2 - m_triangleHeight),
////                                    BORDER_RADIUS, BORDER_RADIUS);

//        drawPath.addRoundedRect(QRect(SHADOW_WIDTH, 0,
//                                    width() - m_triangleWidth, height()),
//                                    BORDER_RADIUS, BORDER_RADIUS);
//        break;
//        case Down:
//        trianglePolygon << QPoint(m_startX, 65);
//        trianglePolygon << QPoint(m_startX + m_triangleWidth / 2, 65 + m_triangleHeight);
//        trianglePolygon << QPoint(m_startX + m_triangleWidth, 65);
////        drawPath.addRoundedRect(QRect(SHADOW_WIDTH, m_triangleHeight + SHADOW_WIDTH,
////                                    width() - SHADOW_WIDTH * 2, height() - SHADOW_WIDTH * 2 - m_triangleHeight),
////                                    BORDER_RADIUS, BORDER_RADIUS);
//        drawPath.addRoundedRect(QRect(SHADOW_WIDTH, 0,
//                                    width() , height() - m_triangleHeight),
//                                    BORDER_RADIUS, BORDER_RADIUS);


//    default:
//        break;
//    }

//    drawPath.addPolygon(trianglePolygon);
//    painter.drawPath(drawPath);
//}
//void ArrowWidget::mousePressEvent(QMouseEvent *)
//{
//    this->close();
//}
