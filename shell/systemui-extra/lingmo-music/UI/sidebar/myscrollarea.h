#ifndef MYSCROLLAREA_H
#define MYSCROLLAREA_H

#include <QWidget>
#include <QScrollArea>
#include <QScrollBar>

class myScrollArea : public QScrollArea
{
    Q_OBJECT
public:
    explicit myScrollArea(QWidget *parent = nullptr);
    ~myScrollArea();


};

#endif // MYSCROLLAREA_H
