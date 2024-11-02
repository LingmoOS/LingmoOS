#include "multitifftoolwid.h"
#include <QDebug>
MultiTiffToolWid::MultiTiffToolWid(QWidget *parent) : QWidget(parent)
{
    this->setFixedSize(138, 48);
    this->setAttribute(Qt::WA_TranslucentBackground, true);
    m_backgroundColor = QColor(255, 255, 255);
}

void MultiTiffToolWid::themeStyle(QString theme)
{
    m_theme = theme;
}
void MultiTiffToolWid::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing); // 反锯齿;
                                                   //    //设置背景颜色
                                                   //    QColor color = this->palette().color(QPalette::Base);
                                                   //    QPalette pal(this->palette());
                                                   //    pal.setColor(QPalette::Background, QColor(color));
                                                   //    this->setPalette(pal);
                                                   //    QBrush brush = QBrush(color);
                                                   //    painter.setBrush(brush);
                                                   //    painter.setOpacity(0.6);
    //    painter.setPen(Qt::transparent);
    QStyleOption opt;
    opt.init(this);
    QPixmap pixmap;
    if (m_theme == "lingmo-dark" || m_theme == "lingmo-black") {
        pixmap = QPixmap(":/res/res/tiff/tiffTool_D.png");
    } else {
        pixmap = QPixmap(":/res/res/tiff/tiffTool_W.png");
    }
    painter.setPen(Qt::transparent);
    painter.drawRoundedRect(opt.rect, 8, 8);
    painter.drawPixmap(opt.rect.adjusted(0, 0, 0, 0), pixmap);
}
