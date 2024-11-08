#ifndef GSETTINGSUBJECT_H
#define GSETTINGSUBJECT_H

#include <QObject>
#include <QGSettings>
#include "constant_class.h"
#include <QMutex>
class GsettingSubject : public QObject
{
    Q_OBJECT
public:
    void iniWidgetStyle();
    void iniTimeZone();
    void iniFontSize();
    ~GsettingSubject();
    explicit GsettingSubject(QObject *parent = nullptr);
    void iniMouseWheel();
signals:
     void blackStyle();
     void whiteStyle();
     void iconChnaged();
     void fontChanged(int size);
private:
     static void iniConnect();
     void iniConnection();
     void iniData();
     QGSettings *m_styleSettings = nullptr;
     QStringList m_stylelist ;
     QGSettings *m_formatSettings = nullptr;
     QGSettings *m_mouseSettings = nullptr;
     void getWheelSpeed();
};

#endif // GSETTINGSUBJECT_H
