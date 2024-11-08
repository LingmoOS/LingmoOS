#ifndef MYPROBAR_H
#define MYPROBAR_H

#include <QWidget>
#include <QLabel>
#include <QProgressBar>
#include <QHBoxLayout>
#include <QResizeEvent>
#include "QPainter"
#include "QSpacerItem"

class myprobar : public QWidget
{
    Q_OBJECT
public:
    explicit myprobar(QWidget *parent = nullptr);
    void setValue(int Value);
    void resizeEvent(QResizeEvent *event);

    QSpacerItem* perspace;
    QProgressBar *bar;
    QLabel *percent;
    QHBoxLayout *percentlay;
signals:
public slots:

};

#endif // MYPROBAR_H
