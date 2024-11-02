#include "lightlabel.h"

LightLabel::LightLabel(QWidget *parent):
    QLabel(parent)
{

}

LightLabel::LightLabel(QString text , QWidget *parent):
    QLabel(parent)
{
    this->setText(text);
}

LightLabel::~LightLabel()
{

}

void LightLabel::paintEvent(QPaintEvent *event)
{
    QPalette paltte;
    paltte.setColor(QPalette::WindowText, paltte.placeholderText().color());
    this->setPalette(paltte);
    QLabel::paintEvent(event);
    return;
}
