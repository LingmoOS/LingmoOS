#ifndef DELAYWIDGET_H
#define DELAYWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPaintEvent>
class DelayWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DelayWidget(QWidget *parent = nullptr);
    ~DelayWidget();
    void updateText(int time);
protected:
    // void  paintEvent(QPaintEvent *event);
private:
    QLabel *label;
    void listenToGsettings();
};

#endif // DELAYWIDGET_H
