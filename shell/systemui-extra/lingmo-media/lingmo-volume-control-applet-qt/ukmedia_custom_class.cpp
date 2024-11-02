/*
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */
#include "ukmedia_custom_class.h"
#include <QPalette>
#include <QPainter>
#include <QPainterPath>
#include <QWidget>
#include <QStyleOptionComplex>
#include <QStylePainter>
#include <QDebug>
#include <QGSettings>
#include <QCoreApplication>
#include <QHBoxLayout>
#include <QApplication>


#define TIMER_TIMEOUT   (2*1000)
#define FOREGROUND_COLOR_NORMAL qApp->palette().text().color()

extern double transparency;

UkmediaTrayIcon::UkmediaTrayIcon(QWidget *parent)
{
    Q_UNUSED(parent);
}

UkmediaTrayIcon::~UkmediaTrayIcon()
{

}

/*!
 * \brief
 * \details
 * 处理托盘的滚动事件
 */
bool UkmediaTrayIcon::event(QEvent *event)
{
    bool value = false;
    QWheelEvent *e = static_cast<QWheelEvent *>(event);
    if (event->type() == QEvent::Wheel) {
        if (e->delta() > 0) {
            value = true;
        }
        else if (e->delta() < 0) {
            value = false;
        }
        Q_EMIT  wheelRollEventSignal(value);
    }
    return QSystemTrayIcon::event(e);

}

MyTimer::MyTimer(QObject *parent)
    :QObject(parent)
{
    m_nTimerID = this->startTimer(TIMER_TIMEOUT);
}

MyTimer::~MyTimer()
{

}

void MyTimer::timerEvent(QTimerEvent *event)
{
    if(event->timerId() == m_nTimerID){
        handleTimeout();
        Q_EMIT timeOut();
    }
}

void MyTimer::handleTimeout()
{
    killTimer(m_nTimerID);
}

LingmoUIApplicationWidget::LingmoUIApplicationWidget(QWidget *parent)
{
//    this->setAttribute(Qt::WA_TranslucentBackground);
//    this->setStyleSheet("QWiget{background:rgba(0,0,0,0);}");
    Q_UNUSED(parent);
}

LingmoUIMediaSliderTipLabel::LingmoUIMediaSliderTipLabel(){
    setAttribute(Qt::WA_TranslucentBackground);
}

LingmoUIMediaSliderTipLabel::~LingmoUIMediaSliderTipLabel(){
}

void LingmoUIApplicationWidget::paintEvent(QPaintEvent *e)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    double transparence = transparency * 255;
    QColor color = palette().color(QPalette::Base);
    color.setAlpha(transparence);
    QBrush brush = QBrush(QColor(0,0,0,0));
    p.setBrush(brush);
    p.setBrush(this->palette().base());
    p.setBrush(QBrush(QColor(19, 19, 20, 0)));
    p.setPen(Qt::NoPen);
    QPainterPath path;
    opt.rect.adjust(0,0,0,0);
    path.addRoundedRect(opt.rect,0,0);
    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    p.drawRoundedRect(opt.rect,0,0);
    setProperty("blurRegion",QRegion(path.toFillPolygon().toPolygon()));
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

}

LingmoUIApplicationWidget::~LingmoUIApplicationWidget()
{

}

void LingmoUIMediaSliderTipLabel::paintEvent(QPaintEvent *e)
{
    QStyleOptionFrame opt;
    initStyleOption(&opt);
    QStylePainter p(this);
//    p.setBrush(QBrush(QColor(0x1A,0x1A,0x1A,0x4C)));
    p.setBrush(QBrush(QColor(0xFF,0xFF,0xFF,0x00)));
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(this->rect(), 1, 1);
    QPainterPath path;
    path.addRoundedRect(opt.rect,6,6);
    p.setRenderHint(QPainter::Antialiasing);
    setProperty("blurRegion",QRegion(path.toFillPolygon().toPolygon()));
    p.drawPrimitive(QStyle::PE_PanelTipLabel, opt);
    this->setProperty("blurRegion", QRegion(QRect(0, 0, 1, 1)));
    QLabel::paintEvent(e);
}

LingmoUIMediaMuteButton::LingmoUIMediaMuteButton(QWidget *parent)
{
    Q_UNUSED(parent);
    this->setFixedSize(36,36);
    m_iconLabel = new QLabel(this);
    m_iconLabel->setFixedSize(36,36);
    m_iconLabel->setAlignment(Qt::AlignCenter);

    connect(qApp, &QApplication::paletteChanged, this, &LingmoUIMediaMuteButton::onPaletteChanged);
}

/*!
 * \brief
 * \details
 * 绘制窗体的颜色及圆角
 */
void LingmoUIMediaMuteButton::paintEvent(QPaintEvent *event)
{
    QPalette pal = this->palette();
    pal.setColor(QPalette::Base, BACKGROUND_COLOR);
    pal.setColor(QPalette::Text, m_backgroundColor);

    QPainterPath cPath;
    cPath.addRect(0, 0, this->width(), this->height());
    cPath.addEllipse(0, 0, this->width(), this->width());

    QPainterPath innerPath;
    innerPath.addEllipse(0, 0, this->width(), this->width());

    QPainter painter(this);
    painter.setRenderHint(QPainter:: Antialiasing, true);  //设置渲染,启动反锯齿
    painter.setPen(Qt::NoPen);

    painter.setBrush(pal.color(QPalette::Base));
    painter.drawPath(cPath);

    painter.fillPath(innerPath, pal.color(QPalette::Text));
}

void LingmoUIMediaMuteButton::mousePressEvent(QMouseEvent *e)
{
    if (m_isActivated) {
        m_backgroundColor = qApp->palette().highlight().color();
    } else {
        m_backgroundColor = qApp->palette().brightText().color();
        m_backgroundColor.setAlphaF(0.20);
    }
    this->update();
    QPushButton::mousePressEvent(e);
}

void LingmoUIMediaMuteButton::mouseReleaseEvent(QMouseEvent *e)
{
    if (m_isActivated) {
        m_backgroundColor = qApp->palette().highlight().color();
    } else {
        m_backgroundColor = qApp->palette().brightText().color();
        m_backgroundColor.setAlphaF(0.18);
    }
    this->update();
    QPushButton::mouseReleaseEvent(e);
}

void LingmoUIMediaMuteButton::enterEvent(QEvent *e)
{
    if (m_isActivated) {
        m_backgroundColor = qApp->palette().highlight().color();
    } else {
        m_backgroundColor = qApp->palette().brightText().color();
        m_backgroundColor.setAlphaF(0.32);
    }
    this->update();
    QPushButton::enterEvent(e);
}

void LingmoUIMediaMuteButton::leaveEvent(QEvent *e)
{
    if (m_isActivated) {
        m_backgroundColor = qApp->palette().highlight().color();
    } else {
        m_backgroundColor = qApp->palette().brightText().color();
        m_backgroundColor.setAlphaF(0.18);
    }
    this->update();
    return QPushButton::leaveEvent(e);
}

LingmoUIMediaMuteButton::~LingmoUIMediaMuteButton()
{

}

//根据连接状态更改图标颜色
void LingmoUIMediaMuteButton::setActive(const bool &isActive)
{
    m_isActivated = isActive;
    refreshButtonIcon();
}

void LingmoUIMediaMuteButton::onPaletteChanged()
{
    refreshButtonIcon();
}

void LingmoUIMediaMuteButton::setButtonIcon(const QIcon &icon)
{
    if (icon.isNull()) {
        return;
    }
    m_pixmap = icon.pixmap(18,18);
}
//显示默认图标
void LingmoUIMediaMuteButton::setDefaultPixmap()
{
    m_iconLabel->setPixmap(m_pixmap);
}

void LingmoUIMediaMuteButton::refreshButtonIcon()
{
    if (m_isActivated) {
        m_backgroundColor = qApp->palette().highlight().color();
        m_iconLabel->setPixmap(loadSvg(m_pixmap, PixmapColor::WHITE));
    } else {
        m_backgroundColor = qApp->palette().brightText().color();
        m_backgroundColor.setAlphaF(0.18);
        if (qApp->palette().base().color().red() > MIDDLE_COLOR) {
            m_iconLabel->setPixmap(m_pixmap);
        } else {
            m_iconLabel->setPixmap(loadSvg(m_pixmap, PixmapColor::WHITE));
        }
    }

    return;
}

const QPixmap LingmoUIMediaMuteButton:: loadSvg(const QPixmap &source, const PixmapColor &cgColor)
{
    QImage img = source.toImage();
    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            auto color = img.pixelColor(x, y);
            if (color.alpha() > 0) {
                switch (cgColor) {
                case PixmapColor::WHITE:
                    color.setRed(255);
                    color.setGreen(255);
                    color.setBlue(255);
                    img.setPixelColor(x, y, color);
                    break;
                case PixmapColor::BLACK:
                    color.setRed(0);
                    color.setGreen(0);
                    color.setBlue(0);
                    img.setPixelColor(x, y, color);
                    break;
                case PixmapColor::GRAY:
                    color.setRed(152);
                    color.setGreen(163);
                    color.setBlue(164);
                    img.setPixelColor(x, y, color);
                    break;
                case PixmapColor::BLUE:
                    color.setRed(61);
                    color.setGreen(107);
                    color.setBlue(229);
                    img.setPixelColor(x, y, color);
                    break;
                default:
                    return source;
                    break;
                }
            }
        }
    }
    return QPixmap::fromImage(img);
}


LingmoUIMediaButton::LingmoUIMediaButton(QWidget *parent)
{
    Q_UNUSED(parent);
    this->setFixedSize(36,36);
    m_iconLabel = new QLabel(this);
    m_iconLabel->setFixedSize(36,36);
    m_iconLabel->setAlignment(Qt::AlignCenter);

    setActive(false);
    connect(qApp, &QApplication::paletteChanged, this, &LingmoUIMediaButton::onPaletteChanged);
}

LingmoUIMediaButton::~LingmoUIMediaButton()
{

}

//根据连接状态更改图标颜色
void LingmoUIMediaButton::setActive(const bool &isActive)
{
    m_isActivated = isActive;
    refreshButtonIcon();
}

void LingmoUIMediaButton::onPaletteChanged()
{
    refreshButtonIcon();
}

void LingmoUIMediaButton::setButtonIcon(const QIcon &icon)
{
    if (icon.isNull()) {
        return;
    }
    m_pixmap = icon.pixmap(18,18);
}
//显示默认图标
void LingmoUIMediaButton::setDefaultPixmap()
{
    m_iconLabel->setPixmap(m_pixmap);
}

void LingmoUIMediaButton::refreshButtonIcon()
{
    if (m_isActivated) {
        m_backgroundColor = qApp->palette().highlight().color();
        m_iconLabel->setPixmap(loadSvg(m_pixmap, PixmapColor::WHITE));
    } else {
        m_backgroundColor = qApp->palette().brightText().color();
        m_backgroundColor.setAlphaF(0.18);
        if (qApp->palette().base().color().red() > MIDDLE_COLOR) {
            m_iconLabel->setPixmap(m_pixmap);
        } else {
            m_iconLabel->setPixmap(loadSvg(m_pixmap, PixmapColor::WHITE));
        }
    }

    return;
}

const QPixmap LingmoUIMediaButton:: loadSvg(const QPixmap &source, const PixmapColor &cgColor)
{
    QImage img = source.toImage();
    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            auto color = img.pixelColor(x, y);
            if (color.alpha() > 0) {
                switch (cgColor) {
                case PixmapColor::WHITE:
                    color.setRed(255);
                    color.setGreen(255);
                    color.setBlue(255);
                    img.setPixelColor(x, y, color);
                    break;
                case PixmapColor::BLACK:
                    color.setRed(0);
                    color.setGreen(0);
                    color.setBlue(0);
                    img.setPixelColor(x, y, color);
                    break;
                case PixmapColor::GRAY:
                    color.setRed(152);
                    color.setGreen(163);
                    color.setBlue(164);
                    img.setPixelColor(x, y, color);
                    break;
                case PixmapColor::BLUE:
                    color.setRed(61);
                    color.setGreen(107);
                    color.setBlue(229);
                    img.setPixelColor(x, y, color);
                    break;
                default:
                    return source;
                    break;
                }
            }
        }
    }
    return QPixmap::fromImage(img);
}

/*!
 * \brief
 * \details
 * 绘制窗体的颜色及圆角
 */
void LingmoUIMediaButton::paintEvent(QPaintEvent *event)
{
    QPalette pal = this->palette();
    pal.setColor(QPalette::Base, BACKGROUND_COLOR);
    pal.setColor(QPalette::Text, m_backgroundColor);

    QPainterPath cPath;
    cPath.addRect(0, 0, this->width(), this->height());
    cPath.addEllipse(0, 0, this->width(), this->width());

    QPainterPath innerPath;
    innerPath.addEllipse(0, 0, this->width(), this->width());

    QPainter painter(this);
    painter.setRenderHint(QPainter:: Antialiasing, true);  //设置渲染,启动反锯齿
    painter.setPen(Qt::NoPen);

    painter.setBrush(pal.color(QPalette::Base));
    painter.drawPath(cPath);

    painter.fillPath(innerPath, pal.color(QPalette::Text));
}

void LingmoUIMediaButton::mousePressEvent(QMouseEvent *event)
{
    if (m_isActivated) {
        m_backgroundColor = qApp->palette().highlight().color();
    } else {
        m_backgroundColor = qApp->palette().brightText().color();
        m_backgroundColor.setAlphaF(0.21);
    }
    this->update();
    return QWidget::mousePressEvent(event);
}

void LingmoUIMediaButton::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_isActivated) {
        m_backgroundColor = qApp->palette().highlight().color();
    } else {
        m_backgroundColor = qApp->palette().brightText().color();
        m_backgroundColor.setAlphaF(0.18);
    }
    this->update();
    return QWidget::mouseReleaseEvent(event);
}

void LingmoUIMediaButton::enterEvent(QEvent *event)
{
    if (m_isActivated) {
        m_backgroundColor = qApp->palette().highlight().color();
    } else {
        m_backgroundColor = qApp->palette().brightText().color();
        m_backgroundColor.setAlphaF(0.32);
    }
    this->update();
    return QWidget::enterEvent(event);
}

void LingmoUIMediaButton::leaveEvent(QEvent *event)
{
    if (m_isActivated) {
        m_backgroundColor = qApp->palette().highlight().color();
    } else {
        m_backgroundColor = qApp->palette().brightText().color();
        m_backgroundColor.setAlphaF(0.18);
    }
    this->update();
    return QWidget::leaveEvent(event);
}


static inline qreal mixQreal(qreal a, qreal b, qreal bias)
{
    return a + (b - a) * bias;
}

QColor mixColor(const QColor &c1, const QColor &c2, qreal bias)
{
    if (bias <= 0.0) {
        return c1;
    }
    if (bias >= 1.0) {
        return c2;
    }
    if (qIsNaN(bias)) {
        return c1;
    }

    qreal r = mixQreal(c1.redF(),   c2.redF(),   bias);
    qreal g = mixQreal(c1.greenF(), c2.greenF(), bias);
    qreal b = mixQreal(c1.blueF(),  c2.blueF(),  bias);
    qreal a = mixQreal(c1.alphaF(), c2.alphaF(), bias);

    return QColor::fromRgbF(r, g, b, a);
}

void LingmoUISettingButton::onPaletteChanged()
{
    m_foregroundColor = FOREGROUND_COLOR_NORMAL;
    this->repaint();
}

void LingmoUISettingButton::setPressColor()
{
    QColor hightlight = this->palette().color(QPalette::Active,QPalette::Highlight);
    QColor mix = this->palette().color(QPalette::Active,QPalette::BrightText);
    m_foregroundColor = mixColor(hightlight, mix, 0.05);
}

void LingmoUISettingButton::setHoverColor()
{
    QColor hightlight = this->palette().color(QPalette::Active,QPalette::Highlight);
    QColor mix = this->palette().color(QPalette::Active,QPalette::BrightText);
    m_foregroundColor = mixColor(hightlight, mix, 0.2);
}

void LingmoUISettingButton::setNormalColor()
{
    m_foregroundColor = FOREGROUND_COLOR_NORMAL;
}

LingmoUISettingButton::LingmoUISettingButton(QWidget *parent)
{
    connect(qApp, &QApplication::paletteChanged, this, &LingmoUISettingButton::onPaletteChanged);
    onPaletteChanged();
}

LingmoUISettingButton::~LingmoUISettingButton()
{

}

void LingmoUISettingButton::paintEvent(QPaintEvent *event)
{
    QPalette pal = this->palette();
    pal.setColor(QPalette::WindowText, m_foregroundColor);
    this->setPalette(pal);
    return QLabel::paintEvent(event);
}

void LingmoUISettingButton::enterEvent(QEvent *event)
{
    setHoverColor();
    this->update();
}

void LingmoUISettingButton::leaveEvent(QEvent *event)
{
    setNormalColor();
    this->update();
}

void LingmoUISettingButton::mousePressEvent(QMouseEvent *event)
{
    setPressColor();
    this->update();
    return QLabel::mousePressEvent(event);
}

void LingmoUISettingButton::mouseReleaseEvent(QMouseEvent *event)
{
    setHoverColor();
    this->update();
    Q_EMIT clicked();
    return QLabel::mouseReleaseEvent(event);
}

AudioSlider::AudioSlider(QWidget *parent)
    : KSlider(parent)
{
    Q_UNUSED(parent);
    setRange(0,100);
    setSliderType(SmoothSlider);
    setOrientation(Qt::Horizontal);
    setFocusPolicy(Qt::StrongFocus);
    setNodeVisible(false);
}

void AudioSlider::wheelEvent(QWheelEvent *e)
{
    if (this->value() - blueValue >= 10 || blueValue - this->value() >= 10) {
        blueValue = this->value();
        Q_EMIT blueValueChanged(blueValue);
    }

    return KSlider::wheelEvent(e);
}

void AudioSlider::keyReleaseEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_VolumeMute) {
        qDebug() << "AudioSlider keyReleaseEvent " << e->key();
        // 过滤掉快捷键的操作
        e->ignore(); // 忽略该事件
        return;
    }

    //start 长按左键或者下键调低音量
    if (e->isAutoRepeat())
    {
        if (this->value() - blueValue >= 10 || blueValue - this->value() >= 10) {
            blueValue = this->value();
            Q_EMIT blueValueChanged(blueValue);
        }
        return;
    }
    //end

    if (this->value() != blueValue)
    {
        blueValue = this->value();
        Q_EMIT blueValueChanged(blueValue);
    }

    return KSlider::keyReleaseEvent(e);
}

void AudioSlider::mouseMoveEvent(QMouseEvent *e)
{
    isMouseWheel = true;

    if (this->value() - blueValue >= 10 || blueValue - this->value() >= 10) {
        blueValue = this->value();
        Q_EMIT blueValueChanged(blueValue);
    }

    return KSlider::mouseMoveEvent(e);
}

void AudioSlider::mouseReleaseEvent(QMouseEvent *e)
{
    isMouseWheel = false;
    if (this->value() != blueValue)
    {
        blueValue = this->value();
        Q_EMIT blueValueChanged(blueValue);
    }

    return KSlider::mouseReleaseEvent(e);
}

AudioSlider::~AudioSlider()
{

}


UkmediaVolumeSlider::UkmediaVolumeSlider(QWidget *parent,bool needTip)
{
    Q_UNUSED(parent);
    if (needTip) {
        state = needTip;
        m_pTiplabel = new LingmoUIMediaSliderTipLabel();
        m_pTiplabel->setWindowFlags(Qt::ToolTip);
    //    qApp->installEventFilter(new AppEventFilter(this));
        m_pTiplabel->setFixedSize(52,30);
        m_pTiplabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    }
    this->setFixedSize(276,20);
}

void UkmediaVolumeSlider::updateValue(QMouseEvent *e)
{
    int value = 0;
    int currentX = e->pos().x();
    if (strstr(getenv("LANGUAGE"),"kk_KZ")) {
        currentX = this->width() - currentX;
    }
    double per = currentX * 1.0 / this->width();
    if ((this->maximum() - this->minimum()) >= 50) { //减小鼠标点击像素的影响
        value = qRound(per*(this->maximum() - this->minimum())) + this->minimum();
        if (value <= (this->maximum() / 2 - this->maximum() / 10 + this->minimum() / 10)) {
            value = qRound(per*(this->maximum() - this->minimum() - 1)) + this->minimum();
        } else if (value > (this->maximum() / 2 + this->maximum() / 10 + this->minimum() / 10)) {
            value = qRound(per*(this->maximum() - this->minimum() + 1)) + this->minimum();
        } else {
            value = qRound(per*(this->maximum() - this->minimum())) + this->minimum();
        }
    } else {
        value = qRound(per*(this->maximum() - this->minimum())) + this->minimum();
    }

    this->setValue(value);
}

void UkmediaVolumeSlider::mousePressEvent(QMouseEvent *e)
{
    if (state) {
        m_pTiplabel->show();
    }

    updateValue(e);
    mousePress = true;
    Q_EMIT silderPressSignal();

    //向父窗口发送自定义事件event type，这样就可以在父窗口中捕获这个事件进行处理
    QEvent evEvent(static_cast<QEvent::Type>(QEvent::User + 1));
    QCoreApplication::sendEvent(parentWidget(), &evEvent);
}

void UkmediaVolumeSlider::mouseReleaseEvent(QMouseEvent *e)
{
    if (mousePress) {
        Q_EMIT silderReleaseSignal();
    }

    mousePress   = false;
    isMouseWheel = false;
    if (this->value() != blueValue)
    {
        blueValue = this->value();
        Q_EMIT blueValueChanged(blueValue);
    }

    QSlider::mouseReleaseEvent(e);
}

void UkmediaVolumeSlider::mouseMoveEvent(QMouseEvent *e)
{
    isMouseWheel = true;

    if (mousePress) {
        updateValue(e);
    }

    if (this->value() - blueValue >= 10 || blueValue - this->value() >= 10) {
        blueValue = this->value();
        Q_EMIT blueValueChanged(blueValue);
    }

    QSlider::mouseMoveEvent(e);
}

void UkmediaVolumeSlider::wheelEvent(QWheelEvent *e)
{
    if (this->value() - blueValue >= 10 || blueValue - this->value() >= 10) {
        blueValue = this->value();
        Q_EMIT blueValueChanged(blueValue);
    }

    QSlider::wheelEvent(e);
}

void UkmediaVolumeSlider::keyReleaseEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_VolumeMute) {
        qDebug() << "UkmediaVolumeSlider keyReleaseEvent " << e->key();
        // 过滤掉快捷键的操作
        e->ignore(); // 忽略该事件
        return;
    }

    //start 长按左键或者下键调低音量
    if (e->isAutoRepeat())
    {
        if (this->value() - blueValue >= 10 || blueValue - this->value() >= 10) {
            blueValue = this->value();
            Q_EMIT blueValueChanged(blueValue);
        }
        return;
    }
    //end

    if (this->value() != blueValue)
    {
        blueValue = this->value();
        Q_EMIT blueValueChanged(blueValue);
    }

    QSlider::keyReleaseEvent(e);
}

void UkmediaVolumeSlider::initStyleOption(QStyleOptionSlider *option)
{
    QSlider::initStyleOption(option);
}

void UkmediaVolumeSlider::leaveEvent(QEvent *e)
{
    if (state) {
        m_pTiplabel->hide();
    }
    return QSlider::leaveEvent(e);
}

void UkmediaVolumeSlider::enterEvent(QEvent *e)
{
    if (state) {
        m_pTiplabel->show();
    }
    return QSlider::enterEvent(e);
}

void UkmediaVolumeSlider::paintEvent(QPaintEvent *e)
{
    QRect rect;
    QStyleOptionSlider m_option;
    QSlider::paintEvent(e);
    if (state) {
        this->initStyleOption(&m_option);
        rect = this->style()->subControlRect(QStyle::CC_Slider, &m_option,QStyle::SC_SliderHandle,this);
        QPoint gPos = this->mapToGlobal(rect.topLeft());
        QString percent;
        percent = QString::number(this->value());
        percent.append("%");
        m_pTiplabel->setText(percent);
        m_pTiplabel->move(gPos.x()-(m_pTiplabel->width()/2)+9,gPos.y()-m_pTiplabel->height()-1);
    }
}

UkmediaVolumeSlider::~UkmediaVolumeSlider()
{
//    delete m_pTiplabel;
}


void LingmoUIButtonDrawSvg::init(QImage img, QColor color)
{
    themeIcon.image = img;
    themeIcon.color = color;
}

void LingmoUIButtonDrawSvg::paintEvent(QPaintEvent *event)
{
    QStyleOptionComplex opt;
    opt.init(this);
    QPainter p(this);

    QColor color;
    switch (buttonState) {
    case PUSH_BUTTON_NORMAL:
        color = QColor(0x25,0x25,0x25,0xFF);
        break;
    case PUSH_BUTTON_CLICK:
        color = QColor(0x37,0x90,0xFA,0xFF);
        break;
    case PUSH_BUTTON_PRESS:
        color = QColor(0x24,0x6D,0xD4,0xFF);
        break;
    default:
        break;
    }

    p.setBrush(QBrush(color));
    p.setPen(Qt::NoPen);
    QPainterPath path;
    opt.rect.adjust(0,0,0,0);
    path.addRoundedRect(opt.rect,18,18);
    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    p.drawRoundedRect(opt.rect,18,18);
    setProperty("blurRegion",QRegion(path.toFillPolygon().toPolygon()));
    style()->drawComplexControl(QStyle::CC_ToolButton,&opt,&p,this);

//    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
//    QStyleOption opt;
//    opt.init(this);
//    QPainter p(this);
//    p.setBrush(QBrush(QColor(0x13,0x13,0x14,0x00)));
//    p.setPen(Qt::NoPen);
//    QPainterPath path;
//    opt.rect.adjust(0,0,0,0);
//    path.addRoundedRect(opt.rect,18,18);
//    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
//    p.drawRoundedRect(opt.rect,18,18);
//    setProperty("blurRegion",QRegion(path.toFillPolygon().toPolygon()));
//    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

QRect LingmoUIButtonDrawSvg::IconGeometry()
{
    QRect res = QRect(QPoint(0,0),QSize(24,24));
    res.moveCenter(QRect(0,0,width(),height()).center());
    return res;
}

void LingmoUIButtonDrawSvg::draw(QPaintEvent* e)
{
    Q_UNUSED(e);
    QPainter painter(this);
    QRect iconRect = IconGeometry();
    if (themeIcon.image.size() != iconRect.size())
    {
        themeIcon.image = themeIcon.image.scaled(iconRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QRect r = themeIcon.image.rect();
        r.moveCenter(iconRect.center());
        iconRect = r;
    }

    this->setProperty("fillIconSymbolicColor", true);
    filledSymbolicColoredPixmap(themeIcon.image,themeIcon.color);
    painter.drawImage(iconRect, themeIcon.image);
}

bool LingmoUIButtonDrawSvg::event(QEvent *event)
{
    switch (event->type())
    {
    case QEvent::Paint:
        draw(static_cast<QPaintEvent*>(event));
        break;

    case QEvent::Move:
    case QEvent::Resize:
    {
        QRect rect = IconGeometry();
    }
        break;

    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
        event->accept();
        break;

    default:
        break;
    }

    return QPushButton::event(event);
}


LingmoUIButtonDrawSvg::LingmoUIButtonDrawSvg(QWidget *parent)
{
    Q_UNUSED(parent);
}
LingmoUIButtonDrawSvg::~LingmoUIButtonDrawSvg()
{

}

QPixmap LingmoUIButtonDrawSvg::filledSymbolicColoredPixmap(QImage &img, QColor &baseColor)
{

    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            auto color = img.pixelColor(x, y);
            if (color.alpha() > 0) {
                int hue = color.hue();
                if (!qAbs(hue - symbolic_color.hue()) < 10) {
                    color.setRed(baseColor.red());
                    color.setGreen(baseColor.green());
                    color.setBlue(baseColor.blue());
                    img.setPixelColor(x, y, color);
                }
            }
        }
    }

    return QPixmap::fromImage(img);
}

FixLabel::FixLabel(QWidget *parent):
    QLabel(parent)
{

}

FixLabel::FixLabel(QString text , QWidget *parent):
    QLabel(parent)
{
    this->setText(text);
}

FixLabel::~FixLabel()
{

}

void FixLabel::paintEvent(QPaintEvent *event)
{
    QFontMetrics fontMetrics(this->font());
    int fontSize = fontMetrics.width(fullText);
    if (fontSize > this->width()) {
        this->setText(fontMetrics.elidedText(fullText, Qt::ElideRight, this->width()), false);
        this->setToolTip(fullText);
    } else {
        this->setText(fullText, false);
        this->setToolTip("");
    }
    QLabel::paintEvent(event);
}

void FixLabel::setText(const QString & text, bool saveTextFlag)
{
    if (saveTextFlag)
        fullText = text;
    QLabel::setText(text);
}

Divider::Divider(QWidget * parent) : QFrame(parent)
{
    this->setFixedSize(420,1);
}

void Divider::paintEvent(QPaintEvent * e)
{
    QPainter p(this);
    QColor color = qApp->palette().color(QPalette::BrightText);
    color.setAlphaF(0.08);
    p.save();
    p.setBrush(color);
    p.setPen(Qt::transparent);
    p.drawRoundedRect(this->rect(), 6, 6);
    p.restore();
    return QFrame::paintEvent(e);
}
