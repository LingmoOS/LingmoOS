#ifndef MARKTOOL_H
#define MARKTOOL_H

#include <QObject>
#include <QColor>
#include <QList>
#include <QPixmap>
#include <QDebug>
#include <QPainterPath>
#include "global/variable.h"
#include "model/processing/processing.h"
#include "model/processing/flip.h"
#include "model/file/file.h"
using namespace cv;
namespace Mark
{

enum MarkType {
    RECTANGLE = 0, //矩形
    CIRCLE,        //椭圆
    LINE,          //直线
    ARROW,         //箭头
    PENCIL,        //画笔
    MARKER,        //标注
    TEXT,          //文字
    BLUR,          //模糊
};

class MarkOperatio
{
public:
    virtual ~MarkOperatio() {}
    MarkType markType; //类型
    double proportion; //缩放比
    QColor color;      //颜色
    int thickness;     //厚度，也用作字号大小
};

class MarkOperatioRectangle : public MarkOperatio
{
public:
    MarkOperatioRectangle(QColor c, QRect rec, double p = 1, int t = 5);
    QRect rect;
};

class MarkOperatioCircle : public MarkOperatio
{
public:
    MarkOperatioCircle(QColor c, double leftX, double leftY, int x, int y, double p = 1, int t = 5);
    double rLeftX;
    double rLeftY;
    double rx;
    double ry;
};

class MarkOperatioLine : public MarkOperatio
{
public:
    MarkOperatioLine(QColor c, QPoint start, QPoint end, double p = 1, int t = 5);
    QPoint startPoint;
    QPoint endPoint;
};

class MarkOperatioArrow : public MarkOperatio
{
public:
    MarkOperatioArrow(QColor c, QPoint start, QPoint end, double p = 1, int t = 5, QPoint arrowStart = QPoint(-1, -1),
                      QPoint arrowEnd = QPoint(-1, -1));
    QPainterPath getArrowHead(QPoint p1, QPoint p2, const int thickness);
    QPoint startPoint;
    QPoint endPoint;
    QPoint arrowStartPoint;
    QPoint arrowEndPoint;
};

class MarkOperatioPencil : public MarkOperatio
{
public:
    MarkOperatioPencil(QColor c, QVector<QPoint> pl, double p = 1, int t = 5);
    QVector<QPoint> pointList;
};

class MarkOperatioMarker : public MarkOperatio
{
public:
    MarkOperatioMarker(QColor c, QPoint start, QPoint end, double p = 1, int t = 10, double a = 0.5);
    QPoint startPoint;
    QPoint endPoint;
    double alpha;
};

class MarkOperatioText : public MarkOperatio
{
public:
    MarkOperatioText(QColor c, QString t, QPoint start, int fs = 12, double p = 1, QString f = "", bool b = false,
                     bool i = false, bool u = false, bool s = false);
    QPoint startPoint;
    QString text;
    QString family;
    bool blod;
    bool italic;
    bool underline;
    bool strikeout;
};

class MarkOperatioBlur : public MarkOperatio
{
public:
    MarkOperatioBlur(QColor c, QRect rec, double p = 1, int t = 5);
    QRect rect;
};

class MarkTool : public QObject
{
    Q_OBJECT
public:
    MarkTool();
    void setPixmap(const QPixmap &pix);
    void setMarkOperatio(MarkOperatio *mo);
    void undo();
    void clear();
    bool save(const QString &path, QList<Processing::FlipWay> flipList, bool backup = true);

private:
    QList<MarkOperatio *> m_operationList;
    QPixmap m_pix;
    bool saveMat(const QString &path, QPixmap pix, QList<Processing::FlipWay> flipList, bool backup = true);
    QRect orderRect(const QRect &orig);
Q_SIGNALS:
    void saveFinish();
};

} // namespace Mark
#endif // MARKTOOL_H
