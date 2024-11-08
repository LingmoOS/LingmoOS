#ifndef COLORAREA_H
#define COLORAREA_H

#include <QWidget>
#include <QColor>
#include <QDebug>
#include <QPainter>
#include <QGSettings>
class ColorArea : public QWidget {
    Q_OBJECT

public:
    explicit ColorArea(QWidget *parent = nullptr);
     ~ColorArea() Q_DECL_OVERRIDE;
    void setcolor(const QColor &color);
       QColor selectBgColor;
//       QGSettings *StyleSetting =  nullptr;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QColor this_Color;
    QColor color1 ;
};

#endif // COLORAREA_H
