#include "switchbutton.h"

#include <QDebug>
#define THEME_QT_SCHEMA "org.lingmo.style"
#define THEME_GTK_SCHEMA "org.mate.interface"
#define TIMER_INTERVAL 5 //每隔5ms动画移动一次
#define MOVING_STEPS 40  //动画总共移动40次

SwitchButton::SwitchButton(QWidget *parent) : QWidget(parent)
{
    //    this->resize(QSize(52, 24));
    this->setFixedSize(QSize(50, 24));

    checked = false;
    hover = false;
    disabled = false;
    isMoving = false;
    isAnimation = true;

    space = 4;
    rectRadius = height() / 2;

    mStep = width() / MOVING_STEPS; //也就是40次动画就可以走完，每次时间间隔是固定的5ms
    mStartX = 0;
    mEndX = 0;

    mTimer = new QTimer(this);
    mTimer->setInterval(TIMER_INTERVAL); //动画更新时间
    connect(mTimer, SIGNAL(timeout()), this, SLOT(updatevalue()));
    if (QGSettings::isSchemaInstalled(THEME_GTK_SCHEMA) && QGSettings::isSchemaInstalled(THEME_QT_SCHEMA)) {
        QByteArray qtThemeID(THEME_QT_SCHEMA);
        QByteArray gtkThemeID(THEME_GTK_SCHEMA);

        m_gtkThemeSetting = new QGSettings(gtkThemeID, QByteArray(), this);
        m_qtThemeSetting = new QGSettings(qtThemeID, QByteArray(), this);

        QString style = m_qtThemeSetting->get("styleName").toString();
        changeColor(style);

        connect(m_qtThemeSetting, &QGSettings::changed, [this](const QString &key) {
            QString style = m_qtThemeSetting->get("styleName").toString();
            if (key == "styleName") {
                changeColor(style);
            }
        });
    }
}

SwitchButton::~SwitchButton() {}

void SwitchButton::paintEvent(QPaintEvent *)
{

    QPainter painter(this);
    //启用反锯齿
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    drawBg(&painter);
    if (!isAnimation) //动画如果禁用，则圆形滑块isMoving始终为false
        isMoving = false;
    if (isMoving)
        animation(&painter);
    drawSlider(&painter);
    painter.end();
}

void SwitchButton::changeColor(const QString &themes)
{
    if (hover) {
        return; //在鼠标下，禁止切换颜色鼠标离开时切换颜色
    }
    if (themes == "lingmo-dark" || themes == "lingmo-black") {
        bgColorOff = QColor(OFF_BG_DARK_COLOR);
        bgColorOn = QColor(ON_BG_DARK_COLOR);
        rectColorEnabled = QColor(ENABLE_RECT_DARK_COLOR);
        rectColorDisabled = QColor(DISABLE_RECT_DARK_COLOR);
        sliderColorDisabled = QColor(DISABLE_RECT_DARK_COLOR);
        sliderColorEnabled = QColor(ENABLE_RECT_DARK_COLOR);
        bgHoverOnColor = QColor(ON_HOVER_BG_DARK_COLOR);
        bgHoverOffColor = QColor(OFF_HOVER_BG_DARK_COLOR);
        bgColorDisabled = QColor(DISABLE_DARK_COLOR);
    } else {
        bgColorOff = QColor(OFF_BG_LIGHT_COLOR);
        bgColorOn = QColor(ON_BG_LIGHT_COLOR);
        rectColorEnabled = QColor(ENABLE_RECT_LIGHT_COLOR);
        rectColorDisabled = QColor(DISABLE_RECT_LIGHT_COLOR);
        sliderColorDisabled = QColor(DISABLE_RECT_LIGHT_COLOR);
        sliderColorEnabled = QColor(ENABLE_RECT_LIGHT_COLOR);
        bgHoverOnColor = QColor(ON_HOVER_BG_LIGHT_COLOR);
        bgHoverOffColor = QColor(OFF_HOVER_BG_LIGHT_COLOR);
        bgColorDisabled = QColor(DISABLE_LIGHT_COLOR);
    }
}

//动画绘制
void SwitchButton::animation(QPainter *painter)
{
    painter->save();
    int h = height();
    int w = width();
    painter->setPen(Qt::NoPen);
    //颜色设置
    if (checked) {
        //开关在左侧时
        painter->setBrush(bgColorOn);
        rect.setRect(0, 0, h + mStartX, h);
    } else {
        painter->setBrush(bgColorOff);
        rect.setRect(mStartX, 0, w - mStartX, h);
    }
    painter->drawRoundedRect(rect, rectRadius, rectRadius);

    painter->restore();
}

//绘制背景
void SwitchButton::drawBg(QPainter *painter)
{
    int w = width();
    int h = height();
    painter->save();
    painter->setPen(Qt::NoPen);
    if (disabled) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(bgColorDisabled);
    } else {
        if (checked) {
            if (isMoving) {
                painter->setBrush(bgColorOff);
                rect.setRect(mStartX, 0, w - mStartX, h);
            } else {
                painter->setBrush(bgColorOn);
                rect.setRect(0, 0, w, h);
            }
        } else {
            if (isMoving) {
                painter->setBrush(bgColorOn);
                rect.setRect(0, 0, mStartX + h, h);
            } else {
                painter->setBrush(bgColorOff);
                rect.setRect(0, 0, w, h);
            }
        }
    }
    //半径为高度的一半
    painter->drawRoundedRect(rect, rectRadius, rectRadius);

    painter->restore();
}

//绘制滑块，也就是圆形按钮
void SwitchButton::drawSlider(QPainter *painter)
{
    painter->save();
    painter->setPen(Qt::NoPen);

    if (!disabled) {
        painter->setBrush(sliderColorEnabled);
    } else
        painter->setBrush(sliderColorDisabled);
    if (disabled) {
        if (!checked) {
            QRect smallRect(8, height() / 2 - 2, 10, 4);
            painter->drawRoundedRect(smallRect, 3, 3);
        } else {
            QRect smallRect(width() - 8 * 2, height() / 2 - 2, 10, 4);
            painter->drawRoundedRect(smallRect, 3, 3);
        }
    }

    QRect rect(0, 0, width(), height());
    int sliderWidth = rect.height() - space * 2;
    QRect sliderRect(mStartX + space, space, sliderWidth, sliderWidth);
    painter->drawEllipse(sliderRect);

    painter->restore();
}

void SwitchButton::mousePressEvent(QMouseEvent *)
{
    qDebug() << "点击后:" << isMoving << checked << disabled;
    if (isMoving) {
        return;
    }
    if (disabled) {
        mEndX = 0;
        Q_EMIT disabledClick();
        return;
    } else {
        checked = !checked;
        Q_EMIT checkedChanged(checked);

        mStep = width() / MOVING_STEPS;

        if (checked) {
            // circle out
            //        endX = width() - height() + space;
            // circle in
            mEndX = width() - height();
        } else {
            mEndX = 0;
        }
        mTimer->start();
        isMoving = true;
    }
}

void SwitchButton::resizeEvent(QResizeEvent *)
{
    //每次开始的x坐标都是跳过圆角，从直线的地方开始计算
    mStep = width() / MOVING_STEPS;

    if (checked) {
        // circle out
        //        startX = width() - height() + space;
        // circle in
        mStartX = width() - height();
    } else
        mStartX = 0;

    rectRadius = height() / 2;
    update();
}
void SwitchButton::enterEvent(QEvent *event)
{
    bgColorOn = bgHoverOnColor;
    bgColorOff = bgHoverOffColor;

    hover = true;
    update();
    return QWidget::enterEvent(event);
}

void SwitchButton::leaveEvent(QEvent *event)
{
    hover = false;

    QString style = m_qtThemeSetting->get("styleName").toString();
    changeColor(style);

    update();
    return QWidget::leaveEvent(event);
}

//根据事件向左还是向右移动
void SwitchButton::updatevalue()
{
    if (checked)
        if (mStartX < mEndX - mStep) {
            mStartX = mStartX + mStep;
        } else {
            mStartX = mEndX;
            mTimer->stop();
            isMoving = false;
        }
    else {
        if (mStartX > mEndX + mStep) {
            mStartX = mStartX - mStep;
        } else {
            mStartX = mEndX;
            mTimer->stop();
            isMoving = false;
        }
    }
    update();
}

void SwitchButton::setChecked(bool checked)
{
    if (this->checked != checked) {
        this->checked = checked;
        Q_EMIT checkedChanged(checked);
        update();
    }

    mStep = width() / MOVING_STEPS;

    if (checked) {
        // circle out
        //        endX = width() - height() + space;
        // circle in
        mEndX = width() - height();
    } else {
        mEndX = 0;
    }
    mTimer->start();
    isMoving = true;
}

bool SwitchButton::isChecked()
{
    return this->checked;
}

void SwitchButton::setDisabledFlag(bool value)
{
    disabled = value;
    update();
}

bool SwitchButton::getDisabledFlag()
{
    return disabled;
}

void SwitchButton::setAnimation(bool on)
{
    isAnimation = on;
}
