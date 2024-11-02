#include "mapwidget.h"
#include <QPainter>
#include <math.h>
#include <qmath.h>
#include <QFile>
#include <QTextStream>
#include <QEvent>
#include <QDebug>
#include <QPicture>
#include <QGuiApplication>
#include <QScreen>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
MapWidget::MapWidget(QWidget *parent) : QWidget(parent)
{
    this->setObjectName("MapWidget");

    m_backgroundImage = QImage(":/res/png/bg.jpg");
    //当显示器分辨率太小时，就缩小世界地图
    QScreen* NowScreen=QGuiApplication::primaryScreen();
    int SCreenHeight=NowScreen->size().height();
    int SCreenWidth=NowScreen->size().width();
    if(1024 > SCreenWidth)
    {
        if(600 < SCreenWidth)
        {
            m_backgroundImage = m_backgroundImage.scaled(m_backgroundImage.width()*0.6, m_backgroundImage.height()*0.6, Qt::KeepAspectRatio);
        }
        else
        {
            m_backgroundImage = m_backgroundImage.scaled(m_backgroundImage.width()*0.3, m_backgroundImage.height()*0.3, Qt::KeepAspectRatio);
        }
    }

    m_pointpng = QImage(":/res/png/point.png");
    this->setMinimumSize(m_backgroundImage.size());
    this->setMaximumSize(m_backgroundImage.size());
    m_pMenu = new QMenu(this);
//    label = new QLabel(this);
//    combox = new QComboBox(this);
#if 0
    for ( const auto* zoneName :
          { "0.0",  "1.0",  "2.0",  "3.0",  "3.5",  "4.0",  "4.5",  "5.0",  "5.5",   "5.75", "6.0",   "6.5",  "7.0",
            "8.0",  "9.0",  "9.5",  "10.0", "10.5", "11.0", "11.5", "12.0", "12.75", "13.0", "-1.0",  "-2.0", "-3.0",
            "-3.5", "-4.0", "-4.5", "-5.0", "-5.5", "-6.0", "-7.0", "-8.0", "-9.0",  "-9.5", "-10.0", "-11.0" } )
    {
        m_timeZones.append( QImage( QStringLiteral( ":/res/png/timezone_" ) + zoneName + ".png" ) );
    }
#endif

}

MapWidget::~MapWidget()
{
    delete m_pMenu;
}

void MapWidget::paintEvent(QPaintEvent *event)
{
    double width = m_backgroundImage.width();
    double height = m_backgroundImage.height();
    QPainter painter(this);
    painter.drawImage(QRectF(0, 0, width, height), m_backgroundImage);

    QPoint point = getLocationPosition( m_zoneItem.longitude.toDouble(), m_zoneItem.latitude.toDouble());
    painter.drawImage(point.x() - m_pointpng.width() / 2, point.y() - m_pointpng.height(), m_pointpng);

}
//static constexpr double MAP_Y_OFFSET = 0.125;
//static constexpr double MAP_X_OFFSET = -0.0370;
//constexpr static double MATH_PI = 3.14159265;

QPoint MapWidget::getLocationPosition( double longitude, double latitude )
{
    double bottom_lat = -64;
    double top_lat = 84;
    const double width = m_backgroundImage.width();
    const double height = m_backgroundImage.height();
    double x = (width * (180.0 + longitude - 12) / 360.0);
    //地图切分不是从180开始导致的-12 2021-3-6
    if(x < 0) {
        x = width + x;
    }
    double y = height*(top_lat-latitude)/(top_lat-bottom_lat);
    return QPoint(x,y);
   /*
    const int width = this->width();
    const int height = this->height();

    double x = ( width / 2.0 + ( width / 2.0 ) * longitude / 180.0 ) + MAP_X_OFFSET * width;
    double y = ( height / 2.0 - ( height / 2.0 ) * latitude / 90.0 ) + MAP_Y_OFFSET * height;

    if ( latitude > 70.0 )
    {
        y -= sin( MATH_PI * ( latitude - 70.0 ) / 56.0 ) * MAP_Y_OFFSET * height * 0.8;
    }
    if ( latitude > 74.0 )
    {
        y += 4;
    }
    if ( latitude > 69.0 )
    {
        y -= 2;
    }
    if ( latitude > 59.0 )
    {
        y -= 4 * int( ( latitude - 54.0 ) / 5.0 );
    }
    if ( latitude > 54.0 )
    {
        y -= 2;
    }
    if ( latitude > 49.0 )
    {
       y -= int( ( latitude - 44.0 ) / 5.0 );
    }
    // Far south, some stretching occurs as well, but it is less pronounced.
    // Move down by 1 pixel per 5 degrees past 10 south
    if ( latitude < 0 )
    {
        y += int( ( -latitude ) / 5.0 );
    }
    // Antarctica isn't shown on the map, but you could try clicking there
    if ( latitude < -60 )
    {
        y = height - 1;
    }

    if ( x < 0 )
    {
        x = width + x;
    }
    if ( x >= width )
    {
        x -= width;
    }
    if ( y < 0 )
    {
        y = height + y;
    }
    if ( y >= height )
    {
        y -= height;
    }

    return QPoint( int( x ), int( y ) );
    */
}

TimeZoneStruct MapWidget::findNearestPoint(QPoint pos)
{
    TimeZoneStruct nearest;
    QPoint p;
    double x = 0, x1 = 0;
    double y = 0, y1 = 0;
    for(const auto pos_p : m_TZLists) {
        x1 = pos_p.longitude.toDouble();
        y1 = pos_p.latitude.toDouble();
        p = getLocationPosition(x1, y1);

        if((abs(pos.x() - p.x()) + abs(pos.y() - p.y())) < (abs(pos.x() - x) + abs(pos.y() - y))) {
            nearest = pos_p;
            x = p.x();
            y = p.y();
         }
    }
    return nearest;
}

void MapWidget::curSelectTimeZone(QAction *ac)
{
    qDebug() << "城市:" << ac->text();
}

void MapWidget::showLanguageSetZone(TimeZoneStruct zoneItem)
{
    m_zoneItem = zoneItem;
    update();
}

void MapWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->button() != Qt::LeftButton) {
        return;
    }
    m_zoneItem = findNearestPoint(event->pos());
    if(m_zoneItem.zone == "") {
        return;
    }

    QString s = QString("%1  %2").arg(m_zoneItem.latitude).arg(m_zoneItem.longitude);
    qDebug() << "x=" << event->pos().x() << "y=" << event->pos().y() << s;
//    QAction* ptask = new QAction();
//    ptask->setText(QString("%1").arg(m_zoneItem.TZcity));
//    m_pMenu->addAction(ptask);
//    connect(m_pMenu, SIGNAL(triggered(QAction*)), this, SLOT(curSelectTimeZone(QAction*)));

//    m_pMenu->exec(cursor().pos());
//    QList<QAction*> list = m_pMenu->actions();
//    foreach (QAction* pAction, list)
//        delete pAction;
    emit signalCurrentZoneChanged(m_zoneItem);
    repaint();
    QWidget::mousePressEvent(event);
}
