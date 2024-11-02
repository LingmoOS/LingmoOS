#ifndef BOTTOMPOINTLABEL_H
#define BOTTOMPOINTLABEL_H
#include<QLabel>
#include<QPainter>
#include<QPaintEvent>
#include<QColor>
class bottompointlabel : public QLabel
{
    Q_OBJECT
public:
    bottompointlabel(const QString& text,QWidget* parent=nullptr,int size=24,QColor backgroundColor = QColor(0xCC,0XCC,0XCC));
    virtual ~bottompointlabel();
    void setBackgroundColor(QColor backgroundColor);
protected:
    virtual void paintEvent(QPaintEvent *);
private:
    QColor m_backgroundColor;
    bool m_bAutoTheme;
};

#endif // BOTTOMPOINTLABEL_H

