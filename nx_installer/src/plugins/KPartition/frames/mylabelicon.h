#ifndef MYLABELICON_H
#define MYLABELICON_H
#include "QLabel"
#include "QPainter"

class mylabelicon : public QLabel
{
public:
    mylabelicon(QPixmap pix,QWidget *parent = nullptr);
    void paintEvent(QPaintEvent *ev);
    void mouseTracking();
    QPixmap m_pix;
    QColor Shadow=QColor(255,0,0,0);
    QPainter pain;
};

#endif // MYLABELICON_H
