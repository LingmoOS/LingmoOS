#ifndef TOPBORDER_H
#define TOPBORDER_H

#include <QWidget>
#include <QPainter>

class topborder : public QWidget
{
    Q_OBJECT
public:
    explicit topborder(QWidget *parent = nullptr);
    void paintEvent(QPaintEvent *event);

signals:

};

#endif // TOPBORDER_H
