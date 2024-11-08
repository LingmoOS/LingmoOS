/*
 * liblingmosdk-qtwidgets's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#include "ktabbar.h"
#include "themeController.h"
#include <QStyleOptionTab>
#include <QPainter>
#include <QPainterPath>
#include <QStylePainter>
#include <QPainterPath>
#include <QFontMetrics>
#include <QDebug>
#include <QEvent>
#include <QApplication>
#include <QVariantAnimation>
#include <QMouseEvent>
#include "parmscontroller.h"

namespace kdk
{
class KTabBarPrivate:public QObject,public ThemeController
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KTabBar)

public:
    KTabBarPrivate(KTabBar*parent)
        :q_ptr(parent),
          m_animation(new QVariantAnimation(this)),
          m_nextTabWidth(0),
          m_animationStarted(false)
    {
        Q_Q(KTabBar);
        setParent(parent);
        parent->installEventFilter(this);
        m_animation->setDuration(300);
        m_animation->setEasingCurve(QEasingCurve::InOutQuad);
        connect(m_animation, &QVariantAnimation::valueChanged, q, [=](){
            q->update();
        });
        connect(m_animation, &QVariantAnimation::finished, this, [=](){
            m_animationStarted = false;
        });
    }
    void changeTheme();
    int getIndexAtPos(const QPoint &p);

protected:
    bool eventFilter(QObject *watched, QEvent *event);

private:
    KTabBar *q_ptr;
    KTabBarStyle m_kTabBarStyle;
    int m_borderRadius = 6;
    int m_horizontalMargin = 1;
    int m_topMargin = 0;
    QColor m_bkgrdColor;
    bool m_useCustomColor;
    QVariantAnimation *m_animation;
    int m_nextTabWidth;
    bool m_animationStarted;
};

KTabBar::KTabBar(KTabBarStyle barStyle,QWidget* parent):
    QTabBar(parent),
    d_ptr(new KTabBarPrivate(this))
{
    Q_D(KTabBar);
    d->m_borderRadius = 6;
    d->m_kTabBarStyle = barStyle;
    d->m_useCustomColor = false;

    //this->setObjectName("KTabbar");
    d->changeTheme();
    connect(d->m_gsetting,&QGSettings::changed, d,&KTabBarPrivate::changeTheme);
    connect(Parmscontroller::self(),&Parmscontroller::modeChanged,this,[=](bool flag){
        updateGeometry();
    });
}

KTabBar::~KTabBar()
{

}

void KTabBar::setTabBarStyle(KTabBarStyle barStyle)
{
    Q_D(KTabBar);
    d->m_kTabBarStyle = barStyle;
    d->changeTheme();
}
KTabBarStyle KTabBar::barStyle()
{
    Q_D(KTabBar);
    return d->m_kTabBarStyle;
}

void KTabBar::setBorderRadius(int radius)
{
    Q_D(KTabBar);
    if(radius < 0 || radius > 20)
        return;
    d->m_borderRadius = radius;
    update();
}

int KTabBar::borderRadius()
{
    Q_D(KTabBar);
    if(d->m_kTabBarStyle == Sliding)
        return 0;
    else
        return d->m_borderRadius;
}

void KTabBar::setBackgroundColor(const QColor &color)
{
    Q_D(KTabBar);
    d->m_useCustomColor = true;
    d->m_bkgrdColor = color;
}

QSize KTabBar::sizeHint() const
{
    auto size = QTabBar::sizeHint();
    size.setHeight(Parmscontroller::parm(Parmscontroller::Parm::PM_TabBarHeight));
    return size;
}

QSize KTabBar::minimumTabSizeHint(int index) const
{
    Q_UNUSED(index)
    QSize size(100,Parmscontroller::parm(Parmscontroller::Parm::PM_PushButtonHeight));
    return size;
}

QSize KTabBar::tabSizeHint(int index) const
{
    Q_UNUSED(index)
    //判断外部是否设置了固定宽度或者固定高度，如果设置，走默认的tabSizeHint
    if((this->maximumHeight() == this->minimumHeight()) ||this->maximumWidth()==this->minimumWidth())
        return QTabBar::tabSizeHint(index);
    auto size = QTabBar::tabSizeHint(index);
    size.setHeight(Parmscontroller::parm(Parmscontroller::Parm::PM_TabBarHeight));
    return size;
}

void KTabBar::paintEvent(QPaintEvent *event)
{
    Q_D(KTabBar);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::HighQualityAntialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    if(ThemeController::widgetTheme() == ClassicTheme)
    {
        if(p.pen().width() == 1)
            p.translate(0.5,0.5);
    }

    QColor fontColor;
    QColor mix;
    QColor borderColor;
    QFontMetrics fm = p.fontMetrics();
    for (int i = 0; i < count(); ++i)
    {
        QStyleOptionTab option;
        initStyleOption(&option, i);
        QRect rect = option.rect.adjusted(d->m_horizontalMargin,0,0,-d->m_topMargin);
        rect.setHeight(Parmscontroller::parm(Parmscontroller::Parm::PM_PushButtonHeight));
        switch (d->m_kTabBarStyle)
        {
        case SegmentDark:
        {
            mix = option.palette.brightText().color();
            fontColor = option.palette.buttonText().color();
            QColor bkgrdColor = d->m_bkgrdColor;
            if(ThemeController::widgetTheme() == ClassicTheme)
            {
                if(ThemeController::themeMode() == LightTheme)
                {
                    if(option.state.testFlag(QStyle::State_Selected))
                    {
                        bkgrdColor = ThemeController::highlightClick(false,palette());
                        bkgrdColor = ThemeController::adjustH(bkgrdColor,-3);
                        bkgrdColor = ThemeController::adjustS(bkgrdColor,-3);
                        bkgrdColor = ThemeController::adjustL(bkgrdColor,-3);
//                        fontColor = QColor("#FFFFFF");

                        borderColor = ThemeController::highlightClick(false,palette());
                        borderColor = ThemeController::adjustH(borderColor,-1);
                        borderColor = ThemeController::adjustS(borderColor,-34);
                        borderColor = ThemeController::adjustL(borderColor,-30);
                    }
                    else if(option.state.testFlag(QStyle::State_MouseOver))
                    {
                         bkgrdColor = ThemeController::highlightHover(false,palette());
                         bkgrdColor = ThemeController::adjustH(bkgrdColor,-3);
                         bkgrdColor = ThemeController::adjustS(bkgrdColor,-8);
                         bkgrdColor = ThemeController::adjustL(bkgrdColor,-3);

                         borderColor = ThemeController::highlightHover(false,palette());
                         borderColor = ThemeController::adjustH(borderColor,-3);
                         borderColor = ThemeController::adjustS(borderColor,-24);
                         borderColor = ThemeController::adjustL(borderColor,-24);
                    }
                }
                else
                {
                    if(option.state.testFlag(QStyle::State_Selected))
                    {
                        bkgrdColor = ThemeController::highlightClick(true,palette());
                        bkgrdColor = ThemeController::adjustH(bkgrdColor,-2);
                        bkgrdColor = ThemeController::adjustS(bkgrdColor,-23);
                        bkgrdColor = ThemeController::adjustL(bkgrdColor,11);
                        fontColor = QColor("#FFFFFF");

                        borderColor = ThemeController::highlightClick(true,palette());
                        borderColor = ThemeController::adjustH(borderColor,-2);
                        borderColor = ThemeController::adjustS(borderColor,2);
                        borderColor = ThemeController::adjustL(borderColor,46);
                    }
                    else if(option.state.testFlag(QStyle::State_MouseOver))
                    {
                         bkgrdColor = ThemeController::highlightHover(true,palette());
                         bkgrdColor = ThemeController::adjustH(bkgrdColor,-2);
                         bkgrdColor = ThemeController::adjustS(bkgrdColor,-23);
                         bkgrdColor = ThemeController::adjustL(bkgrdColor,11);

                         borderColor = ThemeController::highlightHover(true,palette());
                         borderColor = ThemeController::adjustH(borderColor,-2);
                         borderColor = ThemeController::adjustS(borderColor,-2);
                         borderColor = ThemeController::adjustL(borderColor,36);
                    }
                }
            }
            else
            {
                if(option.state.testFlag(QStyle::State_Selected))
                {
                    bkgrdColor = option.palette.highlight().color();
                    fontColor = QColor("#FFFFFF");
                }
                else if(option.state.testFlag(QStyle::State_MouseOver))
                {
                     bkgrdColor = ThemeController::mixColor(bkgrdColor,mix,0.05);
                }
            }
            if(option.position == QStyleOptionTab::Middle)
            {
                p.save();
                p.setBrush(bkgrdColor);
                if(ThemeController::widgetTheme() == ClassicTheme &&(option.state.testFlag(QStyle::State_MouseOver) || option.state.testFlag(QStyle::State_Selected) ))
                    p.setPen(borderColor);
                else
                    p.setPen(Qt::NoPen);
                //利用quadto绘制圆角矩形会出现一个像素的偏差，修正一下QRect底部高度
                p.drawRect(rect.adjusted(0,1,-1,-1));  //调整启典 上描边
                p.restore();
                p.setBrush(Qt::NoBrush);
                p.setPen(fontColor);
                QPoint point;
                uint tf = Qt::AlignVCenter;
                if (!option.icon.isNull()) {
                    QIcon::Mode mode = option.state & QStyle::State_Enabled ? QIcon::Normal
                                                                     : QIcon::Disabled;
                    if (mode == QIcon::Normal && option.state & QStyle::State_HasFocus)
                        mode = QIcon::Active;
                    QIcon::State state = QIcon::Off;
                    if (option.state & QStyle::State_On)
                        state = QIcon::On;

                    QPixmap pixmap = option.icon.pixmap(option.iconSize, mode, state);

                    int w = pixmap.width() / pixmap.devicePixelRatio();
                    int h = pixmap.height() / pixmap.devicePixelRatio();

                    if (!tabText(i).isEmpty())
                        w += option.fontMetrics.boundingRect(option.rect, tf, tabText(i)).width() + 2;

                    point = QPoint(rect.x() + rect.width() / 2 - w / 2,
                                   rect.y() + rect.height() / 2 - h / 2);

                    w = pixmap.width() / pixmap.devicePixelRatio();

                    if (option.direction == Qt::RightToLeft)
                        point.rx() += w;

                    if(fm.width(tabText(i)) >= option.rect.width()-option.iconSize.width()-7)
                        p.drawPixmap(option.rect.x()+4,this->style()->visualPos(option.direction, option.rect, point).y(), pixmap);
                    else
                        p.drawPixmap(this->style()->visualPos(option.direction, option.rect, point), pixmap);

                    if (!tabText(i).isEmpty()){
                        int subH = std::max(option.iconSize.height(),option.fontMetrics.height());
                        int icon_Y = (rect.height() - subH) / 2;
                        int text_X = point.x() + option.iconSize.width() + 4;
                        int text_Y = icon_Y;

                        QRect textRect;
                        if(fm.width(tabText(i)) >= option.rect.width()-option.iconSize.width()-7)
                        {
                            textRect = QRect(option.rect.x()+option.iconSize.width()+8, text_Y, option.rect.width()-option.iconSize.width()-7, option.fontMetrics.height());
                            setTabToolTip(i,tabText(i));
                            QString elidedText = fm.elidedText(tabText(i),Qt::ElideRight,option.rect.width()-option.iconSize.width()-7);
                            p.drawText(textRect,tf,elidedText);
                        }
                        else
                        {
                            textRect=QRect(text_X, text_Y, option.fontMetrics.width(tabText(i)), option.fontMetrics.height());
                            setTabToolTip(i,"");
                            p.drawText(textRect,tf,tabText(i));
                        }
                    }
                }
                else
                {
                    tf |= Qt::AlignHCenter;
                    if(fm.width(tabText(i)) >= option.rect.width())
                    {
                        QString elidedText = fm.elidedText(tabText(i),Qt::ElideRight,option.rect.width());
                        p.drawText(rect,tf,elidedText);
                        setTabToolTip(i,tabText(i));
                    }
                    else
                    {
                        setTabToolTip(i,"");
                        p.drawText(rect,tf,tabText(i));
                    }

                }
            }
            else if(option.position == QStyleOptionTab::Beginning)
            {
                p.save();
                p.setBrush(bkgrdColor);
                if(ThemeController::widgetTheme() == ClassicTheme &&(option.state.testFlag(QStyle::State_MouseOver) || option.state.testFlag(QStyle::State_Selected) ))
                    p.setPen(borderColor);
                else
                    p.setPen(Qt::NoPen);
                QPainterPath path;
                auto tempRect = rect.adjusted(0,1,0,0); //调整启典 上描边
                if(ThemeController::widgetTheme() == ClassicTheme)
                {
                    path.moveTo(tempRect.topLeft());
                    path.lineTo(tempRect.bottomLeft());
                    path.lineTo(tempRect.bottomRight());
                    path.lineTo(tempRect.topRight());
                    path.lineTo(tempRect.topLeft());
                }
                else
                {
                    if(layoutDirection() == Qt::LeftToRight)
                    {
                        path.moveTo(tempRect.topLeft() + QPointF(0, d->m_borderRadius));
                        path.lineTo(tempRect.bottomLeft() - QPointF(0, d->m_borderRadius));
                        path.quadTo(tempRect.bottomLeft(), tempRect.bottomLeft() + QPointF(d->m_borderRadius, 0));
                        path.lineTo(tempRect.bottomRight());
                        path.lineTo(tempRect.topRight());
                        path.lineTo(tempRect.topLeft() + QPointF(d->m_borderRadius, 0));
                        path.quadTo(tempRect.topLeft(), tempRect.topLeft() + QPointF(0, d->m_borderRadius));
                    }
                    else
                    {
                        path.moveTo(tempRect.topLeft());
                        path.lineTo(tempRect.bottomLeft());
                        path.lineTo(tempRect.bottomRight() - QPointF(d->m_borderRadius,0));
                        path.quadTo(tempRect.bottomRight(), tempRect.bottomRight() - QPointF(0,d->m_borderRadius));
                        path.lineTo(tempRect.topRight() + QPointF(0,d->m_borderRadius));
                        path.quadTo(tempRect.topRight(), tempRect.topRight() - QPointF(d->m_borderRadius,0));
                        path.lineTo(tempRect.topLeft());
                    }
                }
                p.drawPath(path);
                p.restore();
                p.setBrush(Qt::NoBrush);
                p.setPen(fontColor);
                QPoint point;
                uint tf = Qt::AlignVCenter;
                if (!option.icon.isNull()) {
                    QIcon::Mode mode = option.state & QStyle::State_Enabled ? QIcon::Normal
                                                                     : QIcon::Disabled;
                    if (mode == QIcon::Normal && option.state & QStyle::State_HasFocus)
                        mode = QIcon::Active;
                    QIcon::State state = QIcon::Off;
                    if (option.state & QStyle::State_On)
                        state = QIcon::On;

                    QPixmap pixmap = option.icon.pixmap(option.iconSize, mode, state);

                    int w = pixmap.width() / pixmap.devicePixelRatio();
                    int h = pixmap.height() / pixmap.devicePixelRatio();

                    if (!tabText(i).isEmpty())
                        w += option.fontMetrics.boundingRect(option.rect, tf, tabText(i)).width() + 2;

                    point = QPoint(rect.x() + rect.width() / 2 - w / 2,
                                   rect.y() + rect.height() / 2 - h / 2);

                    w = pixmap.width() / pixmap.devicePixelRatio();

                    if (option.direction == Qt::RightToLeft)
                        point.rx() += w;

                    p.setBrush(Qt::red);
                    p.drawRect(option.rect.x()+4,this->style()->visualPos(option.direction, option.rect, point).y(),pixmap.width(),pixmap.height());
                    p.setBrush(Qt::NoBrush);

                    if(fm.width(tabText(i)) >= option.rect.width()-option.iconSize.width()-7)
                        p.drawPixmap(option.rect.x()+4,this->style()->visualPos(option.direction, option.rect, point).y(), pixmap);
                    else
                        p.drawPixmap(this->style()->visualPos(option.direction, option.rect, point), pixmap);

                    if (option.direction == Qt::RightToLeft)
                        rect.translate(-point.x() - 2, 0);
                    else
                        rect.translate(point.x() + w + 4, 0);

                    if (!tabText(i).isEmpty()){
                         tf |= Qt::AlignLeft;
                         int subH = std::max(option.iconSize.height(),option.fontMetrics.height());
                         int icon_Y = (rect.height() - subH) / 2;
                         int text_X = point.x() + option.iconSize.width() + 4;
                         int text_Y = icon_Y;
                         QRect textRect;
                         if(fm.width(tabText(i)) >= option.rect.width()-option.iconSize.width()-7)
                         {
                             textRect = QRect(option.rect.x()+option.iconSize.width()+8, text_Y, option.rect.width()-option.iconSize.width()-8, option.fontMetrics.height());
                             setTabToolTip(i,tabText(i));
                             QString elidedText = fm.elidedText(tabText(i),Qt::ElideRight,option.rect.width()-option.iconSize.width()-8);
                             p.drawText(textRect,tf,elidedText);
                         }
                         else
                         {
                             textRect=QRect(text_X, text_Y, option.fontMetrics.width(tabText(i)), option.fontMetrics.height());
                             setTabToolTip(i,"");
                             p.drawText(textRect,tf,tabText(i));
                         }
                     }
                }
                else
                {
                    tf |= Qt::AlignHCenter;
                    if(fm.width(tabText(i)) >= option.rect.width())
                    {
                        QString elidedText = fm.elidedText(tabText(i),Qt::ElideRight,option.rect.width());
                        p.drawText(rect,tf,elidedText);
                        setTabToolTip(i,tabText(i));
                    }
                    else
                    {
                        p.drawText(rect,tf,tabText(i));
                        setTabToolTip(i,"");
                    }
                }
            }
            else
            {
                p.save();
                p.setBrush(bkgrdColor);
                if(ThemeController::widgetTheme() == ClassicTheme &&(option.state.testFlag(QStyle::State_MouseOver) || option.state.testFlag(QStyle::State_Selected) ))
                    p.setPen(borderColor);
                else
                    p.setPen(Qt::NoPen);
                QPainterPath path;
                if(ThemeController::widgetTheme() == ClassicTheme)
                {
                    rect=rect.adjusted(0,1,0,0); //调整启典 上描边
                    path.moveTo(rect.topLeft());
                    path.lineTo(rect.bottomLeft());
                    path.lineTo(rect.bottomRight());
                    path.lineTo(rect.topRight());
                    path.lineTo(rect.topLeft());
                }
                else
                {
                    if(layoutDirection() == Qt::LeftToRight)
                    {
                        path.moveTo(rect.topLeft());
                        path.lineTo(rect.bottomLeft());
                        path.lineTo(rect.bottomRight() - QPointF(d->m_borderRadius,0));
                        path.quadTo(rect.bottomRight(), rect.bottomRight() - QPointF(0, d->m_borderRadius));
                        path.lineTo(rect.topRight() + QPointF(0, d->m_borderRadius));
                        path.quadTo(rect.topRight(), rect.topRight() - QPointF(d->m_borderRadius,0));
                        path.lineTo(rect.topLeft());
                    }
                    else
                    {
                        path.moveTo(rect.topLeft() + QPointF(0, d->m_borderRadius));
                        path.lineTo(rect.bottomLeft() - QPointF(0, d->m_borderRadius));
                        path.quadTo(rect.bottomLeft(), rect.bottomLeft() + QPointF(d->m_borderRadius, 0));
                        path.lineTo(rect.bottomRight());
                        path.lineTo(rect.topRight());
                        path.lineTo(rect.topLeft() + QPointF(d->m_borderRadius, 0));
                        path.quadTo(rect.topLeft(), rect.topLeft() + QPointF(0, d->m_borderRadius));
                    }
                }
                p.drawPath(path);
                p.restore();
                p.setBrush(Qt::NoBrush);
                p.setPen(fontColor);
                QPoint point;
                uint tf = Qt::AlignVCenter;
                if (!option.icon.isNull()) {

                    QIcon::Mode mode = option.state & QStyle::State_Enabled ? QIcon::Normal
                                                                     : QIcon::Disabled;
                    if (mode == QIcon::Normal && option.state & QStyle::State_HasFocus)
                        mode = QIcon::Active;
                    QIcon::State state = QIcon::Off;
                    if (option.state & QStyle::State_On)
                        state = QIcon::On;

                    QPixmap pixmap = option.icon.pixmap(option.iconSize, mode, state);

                    int w = pixmap.width() / pixmap.devicePixelRatio();
                    int h = pixmap.height() / pixmap.devicePixelRatio();

                    if (!tabText(i).isEmpty())
                        w += option.fontMetrics.boundingRect(option.rect, tf, tabText(i)).width() + 4;

                    point = QPoint(rect.x() + rect.width() / 2 - w / 2,
                                   rect.y() + rect.height() / 2 - h / 2);

                    w = pixmap.width() / pixmap.devicePixelRatio();

                    if (option.direction == Qt::RightToLeft)
                        point.rx() += w;

                    if(fm.width(tabText(i)) >= option.rect.width()-option.iconSize.width()-7)
                        p.drawPixmap(option.rect.x()+4,this->style()->visualPos(option.direction, option.rect, point).y(), pixmap);
                    else
                        p.drawPixmap(this->style()->visualPos(option.direction, option.rect, point), pixmap);

                    if (!tabText(i).isEmpty()){
                        int subH = std::max(option.iconSize.height(),option.fontMetrics.height());
                        int icon_Y = (rect.height() - subH) / 2;
                        int text_X = point.x() + option.iconSize.width() + 4;
                        int text_Y = icon_Y;
                        QRect textRect;
                        if(fm.width(tabText(i)) >= option.rect.width()-option.iconSize.width()-7)
                        {
                            textRect = QRect(option.rect.x()+option.iconSize.width()+8, text_Y, option.rect.width()-option.iconSize.width()-8, option.fontMetrics.height());
                            setTabToolTip(i,tabText(i));
                            QString elidedText = fm.elidedText(tabText(i),Qt::ElideRight,option.rect.width()-option.iconSize.width()-8);
                            p.drawText(textRect,elidedText);
                        }
                        else
                        {
                            textRect=QRect(text_X, text_Y, option.fontMetrics.width(tabText(i)), option.fontMetrics.height());
                            setTabToolTip(i,"");
                            p.drawText(textRect,tabText(i));
                        }
                    }
                }
                else
                {
                    tf |= Qt::AlignHCenter;
                    if(fm.width(tabText(i)) >= option.rect.width())
                    {
                        QString elidedText = fm.elidedText(tabText(i),Qt::ElideRight,option.rect.width());
                        p.drawText(rect,tf,elidedText);
                        setTabToolTip(i,tabText(i));
                    }
                    else
                    {
                        p.drawText(rect,tf,tabText(i));
                        setTabToolTip(i,"");
                    }
                }
            }
            break;
        }
        case SegmentLight:
        {
            mix = option.palette.brightText().color();
            fontColor = option.palette.buttonText().color();
            QColor bkgrdColor = d->m_bkgrdColor;
            if(ThemeController::widgetTheme() == ClassicTheme)
            {
                if(ThemeController::themeMode() == LightTheme)
                {
                    if(option.state.testFlag(QStyle::State_Selected))
                    {
                        bkgrdColor = ThemeController::highlightClick(false,palette());
                        bkgrdColor = ThemeController::adjustH(bkgrdColor,-3);
                        bkgrdColor = ThemeController::adjustS(bkgrdColor,-3);
                        bkgrdColor = ThemeController::adjustL(bkgrdColor,-3);

                        borderColor = ThemeController::highlightClick(false,palette());
                        borderColor = ThemeController::adjustH(borderColor,-1);
                        borderColor = ThemeController::adjustS(borderColor,-34);
                        borderColor = ThemeController::adjustL(borderColor,-30);
                    }
                    else if(option.state.testFlag(QStyle::State_MouseOver))
                    {
                         bkgrdColor = ThemeController::highlightHover(false,palette());
                         bkgrdColor = ThemeController::adjustH(bkgrdColor,-3);
                         bkgrdColor = ThemeController::adjustS(bkgrdColor,-8);
                         bkgrdColor = ThemeController::adjustL(bkgrdColor,-3);

                         borderColor = ThemeController::highlightHover(false,palette());
                         borderColor = ThemeController::adjustH(borderColor,-3);
                         borderColor = ThemeController::adjustS(borderColor,-24);
                         borderColor = ThemeController::adjustL(borderColor,-24);
                    }
                }
                else
                {
                    if(option.state.testFlag(QStyle::State_Selected))
                    {
                        bkgrdColor = ThemeController::highlightClick(true,palette());
                        bkgrdColor = ThemeController::adjustH(bkgrdColor,-2);
                        bkgrdColor = ThemeController::adjustS(bkgrdColor,-23);
                        bkgrdColor = ThemeController::adjustL(bkgrdColor,11);

                        fontColor = QColor("#FFFFFF");

                        borderColor = ThemeController::highlightClick(true,palette());
                        borderColor = ThemeController::adjustH(borderColor,-2);
                        borderColor = ThemeController::adjustS(borderColor,2);
                        borderColor = ThemeController::adjustL(borderColor,46);
                    }
                    else if(option.state.testFlag(QStyle::State_MouseOver))
                    {
                         bkgrdColor = ThemeController::highlightHover(true,palette());
                         bkgrdColor = ThemeController::adjustH(bkgrdColor,-2);
                         bkgrdColor = ThemeController::adjustS(bkgrdColor,-23);
                         bkgrdColor = ThemeController::adjustL(bkgrdColor,11);

                         borderColor = ThemeController::highlightHover(true,palette());
                         borderColor = ThemeController::adjustH(borderColor,-2);
                         borderColor = ThemeController::adjustS(borderColor,-2);
                         borderColor = ThemeController::adjustL(borderColor,36);
                    }
                }
            }
            else
            {
                if(option.state.testFlag(QStyle::State_Selected))
                {
                    bkgrdColor = option.palette.highlight().color();
                    fontColor = QColor("#FFFFFF");
                }
                else if(option.state.testFlag(QStyle::State_MouseOver))
                {
                     bkgrdColor = ThemeController::mixColor(bkgrdColor,mix,0.05);
                }
            }
            p.save();
            p.setBrush(bkgrdColor);
            if(ThemeController::widgetTheme() == ClassicTheme &&(option.state.testFlag(QStyle::State_MouseOver) || option.state.testFlag(QStyle::State_Selected) ))
                p.setPen(borderColor);
            else
                p.setPen(Qt::NoPen);
            if(ThemeController::widgetTheme() == ClassicTheme)
                p.drawRoundedRect(rect.adjusted(0,1,-1,-1),0,0);
            else
                p.drawRoundedRect(/*option.rect.adjusted*/rect.adjusted(d->m_horizontalMargin,0,0,-d->m_topMargin),
                              d->m_borderRadius,d->m_borderRadius);
            p.restore();
            p.setBrush(Qt::NoBrush);
            p.setPen(fontColor);
            /*QRect*/  rect = /*option.*/rect.adjusted(d->m_horizontalMargin,0,0,-d->m_topMargin);
            QPoint point;
            uint tf = Qt::AlignVCenter;
            if (!option.icon.isNull()) {
                QIcon::Mode mode = option.state & QStyle::State_Enabled ? QIcon::Normal
                                                                 : QIcon::Disabled;
                if (mode == QIcon::Normal && option.state & QStyle::State_HasFocus)
                    mode = QIcon::Active;
                QIcon::State state = QIcon::Off;
                if (option.state & QStyle::State_On)
                    state = QIcon::On;

                QPixmap pixmap = option.icon.pixmap(option.iconSize, mode, state);

                int w = pixmap.width() / pixmap.devicePixelRatio();
                int h = pixmap.height() / pixmap.devicePixelRatio();

                if (!tabText(i).isEmpty())
                    w += option.fontMetrics.boundingRect(option.rect, tf, tabText(i)).width() + 2;

                point = QPoint(rect.x() + rect.width() / 2 - w / 2,
                               rect.y() + rect.height() / 2 - h / 2);

                w = pixmap.width() / pixmap.devicePixelRatio();

                if (option.direction == Qt::RightToLeft)
                    point.rx() += w;

                if(fm.width(tabText(i)) >= option.rect.width()-option.iconSize.width()-7)
                    p.drawPixmap(option.rect.x()+4,this->style()->visualPos(option.direction, option.rect, point).y(), pixmap);
                else
                    p.drawPixmap(this->style()->visualPos(option.direction, option.rect, point), pixmap);

                if (!tabText(i).isEmpty()){
                    int subH = std::max(option.iconSize.height(),option.fontMetrics.height());
                    int icon_Y = (rect.height() - subH) / 2;
                    int text_X = point.x() + option.iconSize.width() + 4;
                    int text_Y = icon_Y;
                    QRect textRect;
                    if(fm.width(tabText(i)) >= option.rect.width()-option.iconSize.width()-7)
                    {
                        textRect = QRect(option.rect.x()+option.iconSize.width()+8, text_Y, option.rect.width()-option.iconSize.width()-8, option.fontMetrics.height());
                        setTabToolTip(i,tabText(i));
                        QString elidedText = fm.elidedText(tabText(i),Qt::ElideRight,option.rect.width()-option.iconSize.width()-8);
                        p.drawText(textRect,elidedText);
                    }
                    else
                    {
                        textRect=QRect(text_X, text_Y, option.fontMetrics.width(tabText(i)), option.fontMetrics.height());
                        setTabToolTip(i,"");
                        p.drawText(textRect,tabText(i));
                    }
                }
            }
            else
            {
                tf |= Qt::AlignHCenter;
                if(fm.width(tabText(i)) >= option.rect.width())
                {
                    QString elidedText = fm.elidedText(tabText(i),Qt::ElideRight,option.rect.width());
                    p.drawText(rect,tf,elidedText);
                    setTabToolTip(i,tabText(i));
                }
                else
                {
                    p.drawText(rect,tf,tabText(i));
                    setTabToolTip(i,"");
                }
            }
            break;
        }
        case Sliding:
        {
            mix = option.palette.brightText().color();
            fontColor = option.palette.buttonText().color();
            QColor bkgrdColor = d->m_bkgrdColor;
            if(ThemeController::widgetTheme() == ClassicTheme)
            {
                if(ThemeController::themeMode() == LightTheme)
                {
                    if(option.state.testFlag(QStyle::State_Selected))
                    {
                        bkgrdColor = ThemeController::highlightClick(false,palette());
                        bkgrdColor = ThemeController::adjustH(bkgrdColor,0);
                        bkgrdColor = ThemeController::adjustS(bkgrdColor,-29);
                        bkgrdColor = ThemeController::adjustL(bkgrdColor,-42);

                        fontColor = bkgrdColor;
                    }
                    else if(option.state.testFlag(QStyle::State_MouseOver))
                    {
                        bkgrdColor = ThemeController::highlightHover(false,palette());
                        bkgrdColor = ThemeController::adjustH(bkgrdColor,-6);
                        bkgrdColor = ThemeController::adjustS(bkgrdColor,4);
                        bkgrdColor = ThemeController::adjustL(bkgrdColor,-27);
                    }
                }
                else
                {
                    if(option.state.testFlag(QStyle::State_Selected))
                    {
                        bkgrdColor = ThemeController::highlightClick(true,palette());
                        bkgrdColor = ThemeController::adjustH(bkgrdColor,1);
                        bkgrdColor = ThemeController::adjustS(bkgrdColor,25);
                        bkgrdColor = ThemeController::adjustL(bkgrdColor,35);

                        fontColor = bkgrdColor;
                    }
                    else if(option.state.testFlag(QStyle::State_MouseOver))
                    {
                        bkgrdColor = ThemeController::highlightHover(true,palette());
                        bkgrdColor = ThemeController::adjustH(bkgrdColor,3);
                        bkgrdColor = ThemeController::adjustS(bkgrdColor,2);
                        bkgrdColor = ThemeController::adjustL(bkgrdColor,29);
                    }
                }
            }
            else
            {
                if(option.state.testFlag(QStyle::State_Selected))
                {
                    bkgrdColor = option.palette.highlight().color();
                    fontColor = option.palette.highlight().color();
                }
                else if(option.state.testFlag(QStyle::State_MouseOver))
                {
                     bkgrdColor = ThemeController::mixColor(bkgrdColor,mix,0.05);
                }
            }

            p.save();
            QPen pen;
            if(i != currentIndex())
                pen.setColor(bkgrdColor);
            else
                pen.setColor(d->m_bkgrdColor);
            pen.setWidth(2);
            p.setPen(pen);
            p.drawLine(rect.bottomLeft(),rect.bottomRight());
            p.restore();

            if(d->m_animationStarted)
            {
                int left_border = d->m_animation->currentValue().toInt();
                p.save();
                QPen pen;
                pen.setWidth(2);
                pen.setColor(option.palette.highlight().color());
                p.setPen(pen);
                p.drawLine(QPoint(left_border, rect.height()-1),QPoint(left_border + d->m_nextTabWidth, rect.height()-1));
                p.restore();
            }
            else
            {
                p.save();
                QPen pen;
                pen.setColor(bkgrdColor);
                pen.setWidth(2);
                p.setPen(pen);
                p.drawLine(rect.bottomLeft(),rect.bottomRight());
                p.restore();
            }


            p.setBrush(Qt::NoBrush);
            p.setPen(fontColor);
            QPoint point;
            uint tf = Qt::AlignVCenter;
            if (!option.icon.isNull()) {
                QIcon::Mode mode = option.state & QStyle::State_Enabled ? QIcon::Normal
                                                                 : QIcon::Disabled;
                if (mode == QIcon::Normal && option.state & QStyle::State_HasFocus)
                    mode = QIcon::Active;
                QIcon::State state = QIcon::Off;
                if (option.state & QStyle::State_On)
                    state = QIcon::On;

                QPixmap pixmap = option.icon.pixmap(option.iconSize, mode, state);

                int w = pixmap.width() / pixmap.devicePixelRatio();
                int h = pixmap.height() / pixmap.devicePixelRatio();

                if (!tabText(i).isEmpty())
                    w += option.fontMetrics.boundingRect(option.rect, tf, tabText(i)).width() + 2;

                point = QPoint(rect.x() + rect.width() / 2 - w / 2,
                               rect.y() + rect.height() / 2 - h / 2);

                w = pixmap.width() / pixmap.devicePixelRatio();

                if (option.direction == Qt::RightToLeft)
                    point.rx() += w;

                if(fm.width(tabText(i)) >= option.rect.width()-option.iconSize.width()-7)
                     p.drawPixmap(option.rect.x()+4,this->style()->visualPos(option.direction, option.rect, point).y(), pixmap);
                else
                    p.drawPixmap(this->style()->visualPos(option.direction, option.rect, point), pixmap);

                if (!tabText(i).isEmpty()){
                    int subH = std::max(option.iconSize.height(),option.fontMetrics.height());
                    int icon_Y = (rect.height() - subH) / 2;
                    int text_X = point.x() + option.iconSize.width() + 4;
                    int text_Y = icon_Y;
                    QRect textRect;
                    if(fm.width(tabText(i)) >= option.rect.width()-option.iconSize.width()-7)
                    {
                        textRect = QRect(option.rect.x()+option.iconSize.width()+8, text_Y, option.rect.width()-option.iconSize.width()-8, option.fontMetrics.height());
                        setTabToolTip(i,tabText(i));
                        QString elidedText = fm.elidedText(tabText(i),Qt::ElideRight,option.rect.width()-option.iconSize.width()-8);
                        p.drawText(textRect,elidedText);
                    }
                    else
                    {
                        textRect=QRect(text_X, text_Y, option.fontMetrics.width(tabText(i)), option.fontMetrics.height());
                        setTabToolTip(i,"");
                        p.drawText(textRect,tabText(i));
                    }
                }
            }
            else
            {
                tf |= Qt::AlignHCenter;
                if(fm.width(tabText(i)) >= option.rect.width())
                {
                    QString elidedText = fm.elidedText(tabText(i),Qt::ElideRight,option.rect.width());
                    p.drawText(rect,tf,elidedText);
                    setTabToolTip(i,tabText(i));
                }
                else
                {
                    p.drawText(rect,tf,tabText(i));
                    setTabToolTip(i,"");
                }
            }
            break;
        }
        default:
            break;
        }
    }
}

void KTabBarPrivate::changeTheme()
{
    Q_Q(KTabBar);
    switch (m_kTabBarStyle) {
    case SegmentDark:
    {
        if(m_useCustomColor)
            return;
        else
            m_bkgrdColor = q->palette().button().color();
        break;
    }
    case SegmentLight:
    {
        if(m_useCustomColor)
           return;
        else
            m_bkgrdColor = QColor(0,0,0,0);
        break;
    }
    case Sliding:
        if(m_useCustomColor)
            return;
        else
            m_bkgrdColor = q->palette().button().color();
        break;
    default:
        break;
    } ;
    q->update();
}

int KTabBarPrivate::getIndexAtPos(const QPoint &p)
{
    Q_Q(KTabBar);
    if (q->tabRect(q->currentIndex()).contains(p))
        return q->currentIndex();
    for (int i = 0; i < q->count(); ++i)
        if (q->tabRect(i).contains(p))
            return i;
    return -1;
}

bool KTabBarPrivate::eventFilter(QObject *watched, QEvent *event)
{
    Q_Q(KTabBar);
    if(event->type() == QEvent::MouseButtonPress)
    {
        if(q->isEnabled())
        {
            QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent*>(event);
            if(mouseEvent && mouseEvent->button() == Qt::MouseButton::LeftButton)
            {
                int nextIndex = getIndexAtPos(mouseEvent->pos());
                int currentIndex = q->currentIndex();
                if(nextIndex != currentIndex)
                {
                    int cur_left_border = q->tabRect(currentIndex).left();
                    int next_left_border = q->tabRect(nextIndex).left();
                    m_nextTabWidth = q->tabRect(nextIndex).width();
                    m_animation->setStartValue(cur_left_border);
                    m_animation->setEndValue(next_left_border);
                    m_animation->start();
                    m_animationStarted = true;
                }
            }
        }
    }
    return QObject::eventFilter(watched, event);
}

}
#include "ktabbar.moc"
#include "moc_ktabbar.cpp"
