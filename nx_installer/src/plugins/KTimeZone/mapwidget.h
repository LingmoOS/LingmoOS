#ifndef MAPWIDGET_H
#define MAPWIDGET_H

#include <QObject>
#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QMap>
#include <QLabel>
#include <QComboBox>
#include <QMenu>
#include "timezonemodel.h"



class MapWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MapWidget(QWidget *parent = nullptr);
    ~MapWidget();

    double changeCoordinatesTOdouble(QString str);
    TimeZoneStruct findNearestPoint(QPoint pos);
    void setZoneList(TimeZoneStructList list){list.clear(); m_TZLists = list;}

    QPoint getLocationPosition( double longitude, double latitude );
private:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);


public:
    QImage m_backgroundImage;
    QImage m_pointpng;
    QList<QImage> m_timeZones;
    int m_curTimeZone;

    TimeZoneStructList m_TZLists;
    TimeZoneStruct m_zoneItem;
    QMenu* m_pMenu;
    QString m_sel;

signals:
    void signalCurrentZoneChanged(TimeZoneStruct zoneItem);

public slots:
    void curSelectTimeZone(QAction *ac);
    void showLanguageSetZone(TimeZoneStruct zoneItem);
};

#endif // MAPWIDGET_H
