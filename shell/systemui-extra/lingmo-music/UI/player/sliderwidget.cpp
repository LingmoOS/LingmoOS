#include "sliderwidget.h"
#include "UI/base/widgetstyle.h"
#include "UIControl/player/player.h"

#include <QPainter>
#include <QColor>
#include <QRect>
#include <QTimer>

SliderWidget::SliderWidget(QWidget *parent) : QDialog(parent)
{
    if(WidgetStyle::isWayland){
        setWindowFlags(Qt::Popup);
    }
    this->installEventFilter(this);

    this->setAutoFillBackground(true);
    this->setBackgroundRole(QPalette::Base);

    m_radius = 8;

    initUi();
}


void SliderWidget::initUi()
{
    setFixedSize(30,90);

    vSlider = new QSlider(this);
    vSlider->installEventFilter(this);
    vSlider->setOrientation(Qt::Vertical);
    vSlider->setRange(0, 100);
    vSlider->setValue(playController::getInstance().getVolume());

    HLayout = new QHBoxLayout;
    HLayout->addWidget(vSlider);
    HLayout->setContentsMargins(5, 10, 5, 10);
    this->setLayout(HLayout);

    initColor();
}


/**
 * @brief SliderWidget::eventFilter 鼠标滑块点击
 * @param obj
 * @param event
 * @return
 */
bool SliderWidget::eventFilter(QObject *obj, QEvent *event)
{
   if(obj == vSlider) {
        if (event->type()==QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

            if (mouseEvent->button() == Qt::LeftButton) {
               int dur = vSlider->maximum() - vSlider->minimum();
               int pos = vSlider->minimum() + (dur * (vSlider->height() - ((double)mouseEvent->y())) / vSlider->height());

               if(pos != vSlider->sliderPosition()) {
                    vSlider->setValue(pos);
                }
            }
        }
    }

    return QObject::eventFilter(obj,event);
}

bool SliderWidget::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    Q_UNUSED(result);

    if(eventType != "xcb_generic_event_t") {
        return false;
    }

    xcb_generic_event_t *event = (xcb_generic_event_t*)message;

    switch (event->response_type & ~0x80) {
        case XCB_FOCUS_OUT:
            QRect rect(volunmPosX, volunmPosY, volunmPosWidth, volunmPosHeight);
            if(rect.contains(QCursor::pos(), false)) {
                return 0;
            } else {
                this->hide();
                break;
            }
    }

    return false;
}

void SliderWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 抗锯齿;
    QRect rect = this->rect();
    painter.setBrush(QBrush(m_color));
    painter.setPen(Qt::transparent);
    painter.drawRoundedRect(rect, m_radius, m_radius);

    QWidget::paintEvent(event);
}

void SliderWidget::changeVolumePos(int posX, int posY, int width, int height)
{
    volunmPosX = posX;
    volunmPosY = posY;
    volunmPosWidth = width;
    volunmPosHeight = height;
}

void SliderWidget::initColor()
{
    if (WidgetStyle::themeColor == 1) {
        m_color = QColor(38, 38, 38, 240);

    } else {
        m_color = QColor(249, 249, 249, 240);
    }
}
