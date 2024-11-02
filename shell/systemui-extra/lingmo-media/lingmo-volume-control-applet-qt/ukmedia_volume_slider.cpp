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
#include "ukmedia_volume_slider.h"
#include <QPalette>
#include <QPainter>
#include <QPainterPath>
#include <QWidget>
#include <QStyleOptionComplex>
#include <QStylePainter>
#include <QDebug>
#include <QGSettings>

//DisplayerMode displayMode = MINI_MODE;
//SwitchButtonState buttonState = SWITCH_BUTTON_NORMAL;
//extern double transparency;
//LingmoUIApplicationWidget::LingmoUIApplicationWidget(QWidget *parent)
//{
////    this->setAttribute(Qt::WA_TranslucentBackground);
////    this->setStyleSheet("QWiget{background:rgba(0,0,0,0);}");
//    Q_UNUSED(parent);
//}

//LingmoUIMediaSliderTipLabel::LingmoUIMediaSliderTipLabel(){
//    setAttribute(Qt::WA_TranslucentBackground);
//}

//LingmoUIMediaSliderTipLabel::~LingmoUIMediaSliderTipLabel(){
//}

//void LingmoUIApplicationWidget::paintEvent(QPaintEvent *e)
//{
//    QStyleOption opt;
//    opt.init(this);
//    QPainter p(this);
////    double transparence = transparency * 255;
////    QColor color = palette().color(QPalette::Base);
////    color.setAlpha(transparence);
//    QBrush brush = QBrush(QColor(0,0,0,0));
//    p.setBrush(brush);
////    p.setBrush(this->palette().base());
////    p.setBrush(QBrush(QColor(19, 19, 20, 0)));
//    p.setPen(Qt::NoPen);
//    QPainterPath path;
//    opt.rect.adjust(0,0,0,0);
//    path.addRoundedRect(opt.rect,0,0);
//    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
//    p.drawRoundedRect(opt.rect,0,0);
//    setProperty("blurRegion",QRegion(path.toFillPolygon().toPolygon()));
//    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

//}

//LingmoUIApplicationWidget::~LingmoUIApplicationWidget()
//{

//}

//void LingmoUIMediaSliderTipLabel::paintEvent(QPaintEvent *e)
//{
//    QStyleOptionFrame opt;
//    initStyleOption(&opt);
//    QStylePainter p(this);
////    p.setBrush(QBrush(QColor(0x1A,0x1A,0x1A,0x4C)));
//    p.setBrush(QBrush(QColor(0xFF,0xFF,0xFF,0x00)));
//    p.setPen(Qt::NoPen);
//    p.drawRoundedRect(this->rect(), 1, 1);
//    QPainterPath path;
//    path.addRoundedRect(opt.rect,6,6);
//    p.setRenderHint(QPainter::Antialiasing);
//    setProperty("blurRegion",QRegion(path.toFillPolygon().toPolygon()));
//    p.drawPrimitive(QStyle::PE_PanelTipLabel, opt);
//    this->setProperty("blurRegion", QRegion(QRect(0, 0, 1, 1)));
//    QLabel::paintEvent(e);
//}

//LingmoUIMediaButton::LingmoUIMediaButton(QWidget *parent)
//{
//    Q_UNUSED(parent);
//    this->setFixedSize(36,36);
//}

//LingmoUIMediaButton::~LingmoUIMediaButton()
//{

//}

///*!
// * \brief
// * \details
// * 绘制窗体的颜色及圆角
// */
////void LingmoUIMediaButton::paintEvent(QPaintEvent *event)
////{
////    QStyleOptionComplex opt;
////    opt.init(this);
////    QPainter p(this);
//////    double transparence = transparency * 255;
////    p.setBrush(this->palette().base());
////    p.setPen(Qt::NoPen);
////    QPainterPath path;
////    opt.rect.adjust(0,0,0,0);
////    path.addRoundedRect(opt.rect,6,6);
////    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
////    p.drawRoundedRect(opt.rect,6,6);
////    setProperty("blurRegion",QRegion(path.toFillPolygon().toPolygon()));
//////    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
////    style()->drawComplexControl(QStyle::CC_ToolButton,&opt,&p,this);
////}

//void LingmoUIMediaButton::enterEvent(QEvent *event)
//{
//    buttonState = SWITCH_BUTTON_HOVER;
//}

//void LingmoUIMediaButton::leaveEvent(QEvent *event)
//{
//    buttonState = SWITCH_BUTTON_NORMAL;
//}

//void LingmoUIMediaButton::mouseMoveEvent(QMouseEvent *e)
//{
//    buttonState = SWITCH_BUTTON_HOVER;
//    QPushButton::mouseMoveEvent(e);
//}

//void LingmoUIMediaButton::mousePressEvent(QMouseEvent *e)
//{
//    buttonState = SWITCH_BUTTON_PRESS;
//    if (displayMode == MINI_MODE) {
//        Q_EMIT moveMiniSwitchBtnSignale();
//        this->setFixedSize(34,34);
//        QSize iconSize(14,14);
//        this->setIconSize(iconSize);
//        this->setIcon(QIcon("/usr/share/lingmo-media/img/complete-module.svg"));
//    }
//    else {
//        this->setFixedSize(34,34);
//        QSize iconSize(14,14);
//        this->setIconSize(iconSize);
//        this->setIcon(QIcon("/usr/share/lingmo-media/img/mini-module.svg"));
//        Q_EMIT moveAdvanceSwitchBtnSignal();
//    }
//    QPushButton::mousePressEvent(e);
//}

//void LingmoUIMediaButton::mouseReleaseEvent(QMouseEvent *e)
//{
//    buttonState = SWITCH_BUTTON_NORMAL;
//    if (displayMode == MINI_MODE) {
//        Q_EMIT miniToAdvanceSignal();
//        this->setFixedSize(36,36);
//        QSize iconSize(16,16);
//        this->setIconSize(iconSize);
//        this->setIcon(QIcon("/usr/share/lingmo-media/img/complete-module.svg"));
//    }
//    else {
//        Q_EMIT advanceToMiniSignal();
//        this->setFixedSize(36,36);
//        QSize iconSize(16,16);
//        this->setIconSize(iconSize);
//        qDebug() << "设置图标2";
//        this->setIcon(QIcon("/usr/share/lingmo-media/img/mini-module.svg"));
//    }
//    QPushButton::mouseReleaseEvent(e);
//}

//UkmediaVolumeSlider::UkmediaVolumeSlider(QWidget *parent,bool needTip)
//{
//    Q_UNUSED(parent);
//    if (needTip) {
//        state = needTip;
//        m_pTiplabel = new LingmoUIMediaSliderTipLabel();
//        m_pTiplabel->setWindowFlags(Qt::ToolTip);
//    //    qApp->installEventFilter(new AppEventFilter(this));
//        m_pTiplabel->setFixedSize(52,30);
//        m_pTiplabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
//    }
//}

//void UkmediaVolumeSlider::mousePressEvent(QMouseEvent *ev)
//{
//    mousePress = true;
//    Q_EMIT silderPressSignal();
//    if (state) {
//        m_pTiplabel->show();
//    }
//    //注意应先调用父类的鼠标点击处理事件，这样可以不影响拖动的情况
//    QSlider::mousePressEvent(ev);
//    //获取鼠标的位置，这里并不能直接从ev中取值（因为如果是拖动的话，鼠标开始点击的位置没有意义了）
//    double pos = ev->pos().x() / (double)width();
//    setValue(pos *(maximum() - minimum()) + minimum());
//    //向父窗口发送自定义事件event type，这样就可以在父窗口中捕获这个事件进行处理
//    QEvent evEvent(static_cast<QEvent::Type>(QEvent::User + 1));
//    QCoreApplication::sendEvent(parentWidget(), &evEvent);
//    QSlider::mousePressEvent(ev);
//}

//void UkmediaVolumeSlider::mouseReleaseEvent(QMouseEvent *e)
//{
//    if(mousePress){
//        Q_EMIT silderReleaseSignal();
//    }
//    mousePress = false;
//    QSlider::mouseReleaseEvent(e);

//}

//void UkmediaVolumeSlider::initStyleOption(QStyleOptionSlider *option)
//{
//    QSlider::initStyleOption(option);
//}

//void UkmediaVolumeSlider::leaveEvent(QEvent *e)
//{
//    if (state) {
//        m_pTiplabel->hide();
//    }
//}

//void UkmediaVolumeSlider::enterEvent(QEvent *e)
//{
//    if (state) {
//        m_pTiplabel->show();
//    }
//}

//void UkmediaVolumeSlider::paintEvent(QPaintEvent *e)
//{
//    QRect rect;
//    QStyleOptionSlider m_option;
//    QSlider::paintEvent(e);
//    if (state) {

//        this->initStyleOption(&m_option);
//        rect = this->style()->subControlRect(QStyle::CC_Slider, &m_option,QStyle::SC_SliderHandle,this);
//        QPoint gPos = this->mapToGlobal(rect.topLeft());
//        QString percent;
//        percent = QString::number(this->value());
//        percent.append("%");
//        m_pTiplabel->setText(percent);
//        m_pTiplabel->move(gPos.x()-(m_pTiplabel->width()/2)+9,gPos.y()-m_pTiplabel->height()-1);
//    }


//}

//UkmediaVolumeSlider::~UkmediaVolumeSlider()
//{
//    delete m_pTiplabel;
//}

//void LingmoUIButtonDrawSvg::init(QImage img, QColor color)
//{
//    themeIcon.image = img;
//    themeIcon.color = color;
//}

//void LingmoUIButtonDrawSvg::paintEvent(QPaintEvent *event)
//{
//    QStyleOption opt;
//    opt.init(this);
//    QPainter p(this);
//    p.setBrush(QBrush(QColor(0x13,0x13,0x14,0x00)));
//    p.setPen(Qt::NoPen);
//    QPainterPath path;
//    opt.rect.adjust(0,0,0,0);
//    path.addRoundedRect(opt.rect,6,6);
//    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
//    p.drawRoundedRect(opt.rect,6,6);
//    setProperty("blurRegion",QRegion(path.toFillPolygon().toPolygon()));
//    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
//}

//QRect LingmoUIButtonDrawSvg::IconGeometry()
//{
//    QRect res = QRect(QPoint(0,0),QSize(24,24));
//    res.moveCenter(QRect(0,0,width(),height()).center());
//    return res;
//}

//void LingmoUIButtonDrawSvg::draw(QPaintEvent* e)
//{
//    Q_UNUSED(e);
//    QPainter painter(this);
//    QRect iconRect = IconGeometry();
//    if (themeIcon.image.size() != iconRect.size())
//    {
//        themeIcon.image = themeIcon.image.scaled(iconRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
//        QRect r = themeIcon.image.rect();
//        r.moveCenter(iconRect.center());
//        iconRect = r;
//    }

//    this->setProperty("fillIconSymbolicColor", true);
//    filledSymbolicColoredPixmap(themeIcon.image,themeIcon.color);
//    painter.drawImage(iconRect, themeIcon.image);
//}

//bool LingmoUIButtonDrawSvg::event(QEvent *event)
//{
//    switch (event->type())
//    {
//    case QEvent::Paint:
//        draw(static_cast<QPaintEvent*>(event));
//        break;

//    case QEvent::Move:
//    case QEvent::Resize:
//    {
//        QRect rect = IconGeometry();
//    }
//        break;

//    case QEvent::MouseButtonPress:
//    case QEvent::MouseButtonRelease:
//    case QEvent::MouseButtonDblClick:
//        event->accept();
//        break;

//    default:
//        break;
//    }

//    return QPushButton::event(event);
//}


//LingmoUIButtonDrawSvg::LingmoUIButtonDrawSvg(QWidget *parent)
//{
//    Q_UNUSED(parent);
//}
//LingmoUIButtonDrawSvg::~LingmoUIButtonDrawSvg()
//{

//}

//QPixmap LingmoUIButtonDrawSvg::filledSymbolicColoredPixmap(QImage &img, QColor &baseColor)
//{

//    for (int x = 0; x < img.width(); x++) {
//        for (int y = 0; y < img.height(); y++) {
//            auto color = img.pixelColor(x, y);
//            if (color.alpha() > 0) {
//                int hue = color.hue();
//                if (!qAbs(hue - symbolic_color.hue()) < 10) {
//                    color.setRed(baseColor.red());
//                    color.setGreen(baseColor.green());
//                    color.setBlue(baseColor.blue());
//                    img.setPixelColor(x, y, color);
//                }
//            }
//        }
//    }

//    return QPixmap::fromImage(img);
//}

//LingmoUIScrollArea::LingmoUIScrollArea(QWidget *parent){
//}

//LingmoUIScrollArea::~LingmoUIScrollArea(){
//}

//void LingmoUIScrollArea::paintEvent(QPaintEvent *event)
//{
//    QStyleOption opt;
//    opt.init(this);
//    QPainter p(this);
//    p.setBrush(this->palette().base());
//    p.setPen(Qt::NoPen);
//    QPainterPath path;
//    opt.rect.adjust(0,0,0,0);
//    path.addRoundedRect(opt.rect,6,6);
//    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
//    p.drawRoundedRect(opt.rect,6,6);
//    setProperty("blurRegion",QRegion(path.toFillPolygon().toPolygon()));
//    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
////    QWidget::paintEvent(event);
//}

//LingmoUIQMenu::LingmoUIQMenu(){
//}

//bool LingmoUIQMenu::event(QEvent *e)
//{
//    qDebug() << "事件类型" << e->type() << "show";
//    if (e->type() == QEvent::ContextMenu) {
//        this->show();
//    }
//    else if (e->type() == QEvent::WinIdChange) {
//        qDebug() << "winid changed";
//    }
//    else if(e->type() == QEvent::MouseButtonRelease) {
////        this->hide();
//        qDebug() << "事件类型" << e->type();
//    }
//     return QMenu::event(e);
//}

//void LingmoUIQMenu::hideEvent(QHideEvent *e)
//{
//    this->activateWindow();
//    this->setAttribute(Qt::WA_NoMouseReplay);
//    qDebug() << "菜单隐藏" << e->type();
//}

//LingmoUIQMenu::~LingmoUIQMenu(){
//}
