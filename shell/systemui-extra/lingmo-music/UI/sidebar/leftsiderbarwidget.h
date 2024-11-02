#ifndef LEFTSIDERBARWIDGET_H
#define LEFTSIDERBARWIDGET_H

#include <QWidget>
#include <QObject>


class LeftsiderbarWidget : public QWidget
{
    Q_OBJECT
public:
    LeftsiderbarWidget(QWidget *parent = nullptr);
    ~LeftsiderbarWidget();
    void paintEvent(QPaintEvent *event);
private:
    int transparency = 0;


};

#endif // LEFTSIDERBARWIDGET_H
