#ifndef THEME_H
#define THEME_H
//#include <QGSettings>
#include <QGSettings/QGSettings>
#include <QObject>
#include <QString>
#include <QColor>
class theme : public QObject
{
    Q_OBJECT
public:
    explicit theme(QObject *parent = nullptr);
    QGSettings* qtSettings;
    static QString backcolor_black;
    static QString backcolor_black2;
    static QString backcolor_withe;
    static QString backcolor_withe2;

    static QString pre_backcolor;
    static QString back_backcolor;
    static QColor backcolcr;
    static QColor textcolcr;
    static QColor textcolcr45;


    static QColor baseColorFront;
    static QColor baseColorAfter;
    static QColor pointColor;


    static int themetype;
    ~theme();
signals:

};

#endif // THEME_H
