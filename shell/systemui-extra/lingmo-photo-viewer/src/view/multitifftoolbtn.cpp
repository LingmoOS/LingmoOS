#include "multitifftoolbtn.h"
#include <QDebug>
MultiTiffToolBtn::MultiTiffToolBtn(QString btnType, QWidget *parent) : QPushButton(parent)
{
    this->setFixedSize(36, 36);
    this->setBackgroundRole(QPalette::Base);
    m_btnType = btnType;
    this->setIconSize(QSize(36, 36));
    this->setMouseTracking(true);
}

void MultiTiffToolBtn::themeStyle(QString theme)
{
    m_theme = theme;
}

void MultiTiffToolBtn::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QStyleOption opt;
    opt.init(this);
    QPixmap pixmap;
    if (m_theme == "lingmo-dark" || m_theme == "lingmo-black") {
        if (opt.state & QStyle::State_MouseOver) {
            if (this->isDown()) {
                pixmap = QPixmap(":/res/res/tiff/" + m_btnType + "_click_D.png");
            } else {
                pixmap = QPixmap(":/res/res/tiff/" + m_btnType + "_hover_D.png");
            }
        } else {
            pixmap = QPixmap(":/res/res/tiff/" + m_btnType + "_normal_D.png");
        }
    } else {
        if (opt.state & QStyle::State_MouseOver) {
            if (this->isDown()) {
                pixmap = QPixmap(":/res/res/tiff/" + m_btnType + "_click_W.png");
            } else {
                pixmap = QPixmap(":/res/res/tiff/" + m_btnType + "_hover_W.png");
            }
        } else {
            pixmap = QPixmap(":/res/res/tiff/" + m_btnType + "_normal_W.png");
        }
    }
    painter.setPen(Qt::transparent);
    painter.drawRoundedRect(opt.rect, 6, 6);
    painter.drawPixmap(opt.rect.adjusted(0, 0, 0, 0), pixmap);
}
