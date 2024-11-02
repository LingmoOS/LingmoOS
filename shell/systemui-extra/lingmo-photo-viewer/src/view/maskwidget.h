#ifndef MASKWIDGET_H
#define MASKWIDGET_H

#include <QWidget>

class MaskWidget: public QWidget
{
    Q_OBJECT
public:
    explicit MaskWidget(QWidget *parent);
    ~MaskWidget();

public:
    void setDemandWidth(int w);
    void setDemandHigh(int h);
    void setDemandRadius(int r);
    void setDemandColor(int color);

protected:
    void paintEvent(QPaintEvent *event);

private:
    int pWidth;
    int pHeigh;
    int pRadius;
    int pBorder;
    QString pColor;
};

#endif // MASKWIDGET_H
