#include "painterthick.h"
PainterThick::PainterThick(int diameter, bool type, QColor color, QWidget *parent) : QCheckBox(parent)
{
    m_diameter = diameter;
    m_color = color;
    m_type = type;
    this->setFixedSize(32, 32);
}
void PainterThick::paintEvent(QPaintEvent *)
{
    QPainter paint(this);
    QPainterPath path;
    paint.setPen(Qt::NoPen);
    if (m_type == false) {
        //圆形
        if (!this->isChecked()) {
            if ("lingmo-dark" == m_themeChange || "lingmo-black" == m_themeChange) {
                paint.setBrush(QColor(255, 255, 255));
                paint.setOpacity(0);
                paint.setRenderHint(QPainter::Antialiasing);
                paint.drawEllipse(QRect(this->width() / 2 - (m_diameter + 4) / 2,
                                        this->height() / 2 - (m_diameter + 4) / 2, m_diameter + 4, m_diameter + 4));
                paint.setRenderHint(QPainter::Antialiasing);
                paint.setBrush(QColor(153, 153, 153)); //设置画刷形式
                paint.setOpacity(1);
                path.addEllipse(this->width() / 2 - m_diameter / 2, this->height() / 2 - m_diameter / 2, m_diameter,
                                m_diameter);
                paint.drawPath(path);
            } else {
                paint.setBrush(QColor(255, 255, 255));
                paint.setOpacity(0);
                paint.setRenderHint(QPainter::Antialiasing);
                paint.drawEllipse(QRect(this->width() / 2 - (m_diameter + 4) / 2,
                                        this->height() / 2 - (m_diameter + 4) / 2, m_diameter + 4, m_diameter + 4));
                paint.setRenderHint(QPainter::Antialiasing);
                paint.setBrush(QColor(78, 78, 78)); //设置画刷形式
                paint.setOpacity(1);
                path.addEllipse(this->width() / 2 - m_diameter / 2, this->height() / 2 - m_diameter / 2, m_diameter,
                                m_diameter);
                paint.drawPath(path);
            }
        } else {
            paint.setBrush(QColor(255, 255, 255));
            paint.setRenderHint(QPainter::Antialiasing);
            paint.drawEllipse(QRect(this->width() / 2 - (m_diameter + 4) / 2, this->height() / 2 - (m_diameter + 4) / 2,
                                    m_diameter + 4, m_diameter + 4));
            paint.setRenderHint(QPainter::Antialiasing);
            paint.setBrush(QColor(0, 0, 0));
            paint.drawEllipse(this->width() / 2 - m_diameter / 2, this->height() / 2 - m_diameter / 2, m_diameter,
                              m_diameter);
            paint.drawPath(path);
        }
    } else {
        //方形
        if (!this->isChecked()) {
            //            paint.setBrush(QColor(255, 255, 255));
            //            paint.setOpacity(0);
            //            paint.setRenderHint(QPainter::Antialiasing);
            //            paint.drawRoundRect(QRect(0, 0, this->width(), this->height()), 0, 0);

            paint.setBrush(QColor(0, 0, 255));
            paint.setOpacity(0);
            paint.setRenderHint(QPainter::Antialiasing);
            paint.drawRoundRect(QRect(this->width() / 2 - (m_diameter + 4) / 2,
                                      this->height() / 2 - (m_diameter + 4) / 2, m_diameter + 4, m_diameter + 4),
                                30, 30);
            paint.setRenderHint(QPainter::Antialiasing);
            paint.setBrush(m_color); //设置画刷形式
            paint.setOpacity(1);
            paint.drawRoundRect(this->width() / 2 - m_diameter / 2, this->height() / 2 - m_diameter / 2, m_diameter,
                                m_diameter, 30, 30);
            paint.drawPath(path);
        } else {
            //            paint.setBrush(QColor(255, 255, 255));
            //            paint.setOpacity(0);
            //            paint.setRenderHint(QPainter::Antialiasing);
            //            paint.drawRoundRect(QRect(0, 0, this->width(), this->height()), 0, 0);

            paint.setBrush(QColor(255, 255, 255));
            paint.setRenderHint(QPainter::Antialiasing);
            paint.drawRoundRect(QRect(this->width() / 2 - (m_diameter + 4) / 2,
                                      this->height() / 2 - (m_diameter + 4) / 2, m_diameter + 4, m_diameter + 4),
                                30, 30);
            paint.setRenderHint(QPainter::Antialiasing);
            paint.setBrush(m_color);
            paint.drawRoundRect(this->width() / 2 - m_diameter / 2, this->height() / 2 - m_diameter / 2, m_diameter,
                                m_diameter, 30, 30);
            paint.drawPath(path);
        }
    }
}

void PainterThick::mousePressEvent(QMouseEvent *event)
{
    this->setChecked(true);
}

void PainterThick::resetSize(QSize realSize)
{
    this->setFixedSize(realSize);
}

void PainterThick::resetColorSize(int diameter)
{
    m_diameter = m_diameter;
}
