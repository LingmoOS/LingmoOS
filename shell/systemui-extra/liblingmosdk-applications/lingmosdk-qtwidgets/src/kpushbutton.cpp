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
 * Authors: Xiangwei Liu <liuxiangwei@kylinos.cn>
 *
 */

#include "kpushbutton.h"
#include "themeController.h"
#include <QColor>
#include <QPainter>
#include <QStyleOptionButton>
#include <QPainterPath>
#include <QDebug>

namespace kdk {

class KPushButtonPrivate:public QObject,public ThemeController
{
    Q_DECLARE_PUBLIC(KPushButton)
    Q_OBJECT
public:
    KPushButtonPrivate(KPushButton *parent):q_ptr(parent)
    {setParent(parent);}

    void changeTheme();
private:
    KPushButton* q_ptr;
    int m_radius;
    int m_bottomLeft;
    int m_topLeft;
    int m_topRight;
    int m_bottomRight;
    QColor m_backgroundColor;
    bool m_useCustomColor;
    bool m_hasOneParam;
    bool m_isTranslucent;
    bool m_isIconHighlight;
    QColor m_iconColor;
    bool m_useCustomIconColor;
    bool m_isBackgroundColorHighlight;
    bool m_isTranslucentFlag;
    KPushButton::ButtonType m_buttonType;
};

KPushButton::KPushButton(QWidget *parent):QPushButton(parent),d_ptr(new KPushButtonPrivate(this))
{
    Q_D(KPushButton);
    d->m_radius = 6;
    d->m_bottomLeft = 6;
    d->m_topLeft = 6;
    d->m_topRight = 6;
    d->m_bottomRight = 6;
    d->m_backgroundColor = palette().color(QPalette::Button);
    d->m_useCustomColor = false;
    d->m_hasOneParam = true;
    d->m_buttonType = NormalType;
    d->m_isTranslucent = false;
    d->m_isIconHighlight = false;
    d->m_iconColor = palette().color(QPalette::ButtonText);
    d->m_useCustomIconColor = false;
    d->m_isBackgroundColorHighlight = false;
    d->m_isTranslucentFlag = false;
    connect(d->m_gsetting,&QGSettings::changed, d,&KPushButtonPrivate::changeTheme);
}

KPushButton::~KPushButton()
{

}

void KPushButton::setBorderRadius(int radius)
{
    Q_D(KPushButton);
    d->m_hasOneParam = true;
    d->m_radius = radius;
    update();
}

void KPushButton::setBorderRadius(int bottomLeft,int topLeft,int topRight,int bottomRight)
{
    Q_D(KPushButton);
    d->m_hasOneParam = false;
    d->m_bottomLeft = bottomLeft;
    d->m_topLeft = topLeft;
    d->m_topRight = topRight;
    d->m_bottomRight = bottomRight;
    update();
}

int KPushButton::borderRadius()
{
    Q_D(KPushButton);
    return d->m_radius;
}

void KPushButton::setBackgroundColor(QColor color)
{
    Q_D(KPushButton);
    if(d->m_isBackgroundColorHighlight)
        d->m_isBackgroundColorHighlight = false;
    d->m_useCustomColor = true;
    d->m_backgroundColor = color;
    update();
}

QColor KPushButton::backgroundColor()
{
    Q_D(KPushButton);
    return d->m_backgroundColor;
}

void KPushButton::setButtonType(ButtonType type)
{
    Q_D(KPushButton);
    d->m_buttonType = type;
}

KPushButton::ButtonType KPushButton::buttonType()
{
    Q_D(KPushButton);
    return d->m_buttonType;
}

void KPushButton::setTranslucent(bool flag)
{
    Q_D(KPushButton);
    d->m_isTranslucentFlag = flag;
    d->m_isTranslucent = flag;
    d->changeTheme();
}

bool KPushButton::isTranslucent()
{
    Q_D(KPushButton);
    return d->m_isTranslucent;
}

void KPushButton::setIconHighlight(bool flag)
{
    Q_D(KPushButton);
    d->m_isIconHighlight = flag;
}

bool KPushButton::isIconHighlight()
{
    Q_D(KPushButton);
    return d->m_isIconHighlight;
}

void KPushButton::setIconColor(QColor color)
{
    Q_D(KPushButton);
    d->m_useCustomIconColor = true;
    d->m_iconColor = color;
    update();
}

QColor KPushButton::IconColor()
{
    Q_D(KPushButton);
    return d->m_iconColor;
}

void KPushButton::setBackgroundColorHighlight(bool flag)
{
    Q_D(KPushButton);
    if(d->m_useCustomColor)
        d->m_useCustomColor = false;
    d->m_isBackgroundColorHighlight = flag;
}

bool KPushButton::isBackgroundColorHighlight()
{
    Q_D(KPushButton);
    return d->m_isBackgroundColorHighlight;
}

bool KPushButton::eventFilter(QObject *watched, QEvent *event)
{
    Q_D(KPushButton);
    return QPushButton::eventFilter(watched,event);
}

void KPushButton::paintEvent(QPaintEvent *event)
{
    Q_D(KPushButton);
    QStyleOptionButton option;
    initStyleOption(&option);

    QPainter p(this);
    QColor fontColor = option.palette.color(QPalette::Active,QPalette::BrightText);
    QColor mix = option.palette.brightText().color();
    QColor highlight = option.palette.highlight().color();
    QColor backgroundColor;
    if(ThemeController::widgetTheme() == ClassicTheme )
    {
        if(p.pen().width() == 1)
            p.translate(0.5,0.5);
    }
    if(d->m_isTranslucent){
        /*判断使用用户设置的背景色、跟随系统高亮色或是使用默认的背景色*/
        if(d->m_useCustomColor){
            backgroundColor = d->m_backgroundColor;
        }else if(d->m_isBackgroundColorHighlight){
            backgroundColor = highlight;
        }else{
            backgroundColor = option.palette.brightText().color();
        }

        if(ThemeController::themeMode() == LightTheme){
            /*按钮处于不可用（Disabled）状态*/
            if(!option.state.testFlag(QStyle::State_Enabled))
            {
                fontColor = option.palette.color(QPalette::Disabled,QPalette::BrightText);
                backgroundColor.setAlphaF(0.1);
            }
            else{
                /*按钮处于可用（Enabled）状态*/
                if(option.state.testFlag(QStyle::State_MouseOver))      /*鼠标在按钮上(hover状态)*/
                {
                    if(option.state.testFlag(QStyle::State_Sunken))     /*按钮被选中（clicked）*/
                        backgroundColor.setAlphaF(0.21);
                    else
                        backgroundColor.setAlphaF(0.16);
                }
                else{
                    backgroundColor.setAlphaF(0.1);
                }
            }
        }
        else{
            /*按钮处于不可用（Disabled）状态*/
            if(!option.state.testFlag(QStyle::State_Enabled))
            {
                fontColor = option.palette.color(QPalette::Disabled,QPalette::BrightText);
                backgroundColor.setAlphaF(0.1);
            }
            else
            {
                /*按钮处于可用（Enabled）状态*/
                if(option.state.testFlag(QStyle::State_MouseOver))      /*鼠标在按钮上(hover状态)*/
                {
                    if(option.state.testFlag(QStyle::State_Sunken))     /*按钮被选中（clicked）*/
                        backgroundColor.setAlphaF(0.3);
                    else
                        backgroundColor.setAlphaF(0.2);
                }
                else{
                    backgroundColor.setAlphaF(0.1);
                }
            }
        }
    }
    else{
        if(ThemeController::widgetTheme() == ClassicTheme)
        {
            /*判断使用用户设置的背景色或是使用默认的背景色*/
            if(d->m_useCustomColor){
                backgroundColor = d->m_backgroundColor;
            }else if(d->m_isBackgroundColorHighlight){
                backgroundColor=highlight;
                if(option.state.testFlag(QStyle::State_MouseOver))      /*鼠标在按钮上(hover状态)*/
                {
                    if(option.state.testFlag(QStyle::State_Sunken))     /*按钮被选中（clicked）*/
                    {
                        if(ThemeController::themeMode() == LightTheme)
                        {
                            backgroundColor =ThemeController::highlightClick(false,palette());

                            backgroundColor = ThemeController::adjustH(backgroundColor,-1);
                            backgroundColor = ThemeController::adjustS(backgroundColor,-34);
                            backgroundColor = ThemeController::adjustL(backgroundColor,-30);
                        }
                        else
                        {
                            backgroundColor =ThemeController::highlightClick(true,palette());

                            backgroundColor = ThemeController::adjustH(backgroundColor,-2);
                            backgroundColor = ThemeController::adjustS(backgroundColor,2);
                            backgroundColor = ThemeController::adjustL(backgroundColor,45);
                        }
                    }
                    else
                    {
                        if(ThemeController::themeMode() == LightTheme)
                        {
                            backgroundColor =ThemeController::highlightHover(false,palette());

                            backgroundColor = ThemeController::adjustH(backgroundColor,-3);
                            backgroundColor = ThemeController::adjustS(backgroundColor,-24);
                            backgroundColor = ThemeController::adjustL(backgroundColor,-24);
                        }
                        else
                        {
                            backgroundColor =ThemeController::highlightHover(true,palette());

                            backgroundColor = ThemeController::adjustH(backgroundColor,-2);
                            backgroundColor = ThemeController::adjustS(backgroundColor,-3);
                            backgroundColor = ThemeController::adjustL(backgroundColor,36);
                        }
                    }
                }
            }else{
                backgroundColor = palette().color(QPalette::Button);
            }

            /*按钮处于不可用（Disabled）状态*/
            if(!option.state.testFlag(QStyle::State_Enabled))
            {
                fontColor = option.palette.color(QPalette::Disabled,QPalette::BrightText);
            }
            else
            {
                if(d->m_useCustomColor){
                    if(option.state.testFlag(QStyle::State_MouseOver))      /*鼠标在按钮上(hover状态)*/
                    {
                        if(option.state.testFlag(QStyle::State_Sunken))     /*按钮被选中（clicked）*/
                            backgroundColor = ThemeController::mixColor(backgroundColor,mix,0.2);
                        else
                            backgroundColor = ThemeController::mixColor(backgroundColor,mix,0.05);
                        fontColor = option.palette.color(QPalette::HighlightedText);
                    }
                }
                else if( d->m_isBackgroundColorHighlight)
                {
                    backgroundColor = backgroundColor;
                }
                else{
                /*按钮处于可用（Enabled）状态*/
                    if(option.state.testFlag(QStyle::State_MouseOver))      /*鼠标在按钮上(hover状态)*/
                    {
                        if(option.state.testFlag(QStyle::State_Sunken))     /*按钮被选中（clicked）*/
                            backgroundColor = ThemeController::mixColor(highlight,mix,0.2);
                        else
                            backgroundColor = ThemeController::mixColor(highlight,mix,0.05);
                        fontColor = option.palette.color(QPalette::HighlightedText);
                    }
                    else
                    {
                        backgroundColor = palette().color(QPalette::Button);
                    }
                }
            }
        }
        else
        {
            /*判断使用用户设置的背景色或是使用默认的背景色*/
            if(d->m_useCustomColor){
                backgroundColor = d->m_backgroundColor;
            }else if(d->m_isBackgroundColorHighlight){
                backgroundColor = highlight;
            }else{
                backgroundColor = palette().color(QPalette::Button);
            }

            /*按钮处于不可用（Disabled）状态*/
            if(!option.state.testFlag(QStyle::State_Enabled))
            {
                fontColor = option.palette.color(QPalette::Disabled,QPalette::BrightText);
            }
            else
            {
                if(d->m_useCustomColor || d->m_isBackgroundColorHighlight){
                    if(option.state.testFlag(QStyle::State_MouseOver))      /*鼠标在按钮上(hover状态)*/
                    {
                        if(option.state.testFlag(QStyle::State_Sunken))     /*按钮被选中（clicked）*/
                            backgroundColor = ThemeController::mixColor(backgroundColor,mix,0.2);
                        else
                            backgroundColor = ThemeController::mixColor(backgroundColor,mix,0.05);
                        fontColor = option.palette.color(QPalette::HighlightedText);
                    }
                }
                else{
                /*按钮处于可用（Enabled）状态*/
                    if(option.state.testFlag(QStyle::State_MouseOver))      /*鼠标在按钮上(hover状态)*/
                    {
                        if(option.state.testFlag(QStyle::State_Sunken))     /*按钮被选中（clicked）*/
                        {
                            if(ThemeController::widgetTheme() != ClassicTheme)
                                backgroundColor = ThemeController::mixColor(highlight,mix,0.2);
                        }
                        else
                        {
                            if(ThemeController::widgetTheme() != ClassicTheme)
                                backgroundColor = ThemeController::mixColor(highlight,mix,0.05);
                        }
                        fontColor = option.palette.color(QPalette::HighlightedText);
                    }
                    else
                    {
                        backgroundColor = palette().color(QPalette::Button);
                    }
                }
            }
        }

        if(isChecked())
            backgroundColor = highlight;
    }

    p.setRenderHint(QPainter::HighQualityAntialiasing);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    /*绘制背景色和rect*/
    p.save();
    p.setBrush(backgroundColor);
    QColor borderColor;
    if(ThemeController::widgetTheme() == ClassicTheme )
    {
        if(ThemeController::themeMode() == LightTheme)
            borderColor=ThemeController::lanhuHSLToQtHsl(0, 0, 73);
        else
            borderColor=ThemeController::lanhuHSLToQtHsl(0, 0, 35);
        p.setPen(borderColor);
    }
    else
        p.setPen(Qt::NoPen);

    switch(d->m_buttonType)
    {
    case NormalType:
    {
        if(d->m_hasOneParam)
        {
            if(ThemeController::widgetTheme() == ClassicTheme )
                p.drawRoundedRect(option.rect.adjusted(0,0,-1,-1),0,0);
            else
                p.drawRoundedRect(option.rect.adjusted(0,0,-1,-1),d->m_radius,d->m_radius);
        }
        else
        {
            QPainterPath path;
            if(ThemeController::widgetTheme() == ClassicTheme )
                path.addRect(option.rect.adjusted(0,0,-1,-1));
            else
            {
                path.moveTo(option.rect.topLeft() + QPointF(0,d->m_topLeft));
                path.lineTo(option.rect.bottomLeft() - QPointF(0,d->m_bottomLeft));
                path.quadTo(option.rect.bottomLeft(),option.rect.bottomLeft() + QPointF(d->m_bottomLeft,0));
                path.lineTo(option.rect.bottomRight() - QPointF(d->m_bottomRight,0));
                path.quadTo(option.rect.bottomRight(),option.rect.bottomRight() - QPointF(0,d->m_bottomRight));
                path.lineTo(option.rect.topRight() + QPointF(0,d->m_topRight));
                path.quadTo(option.rect.topRight(),option.rect.topRight() - QPointF(d->m_topRight,0));
                path.lineTo(option.rect.topLeft() + QPointF(d->m_topLeft,0));
                path.quadTo(option.rect.topLeft(),option.rect.topLeft() + QPointF(0,d->m_topLeft));
            }
            p.drawPath(path);
        }
        p.restore();
        break;
    }
    case CircleType:
        p.drawEllipse(option.rect.adjusted(0,0,-1,-1));
        p.restore();
        break;
    default:
        break;
    }

    /*绘制图标和文字*/
    QPen pen;
    pen.setColor(fontColor);
    p.setBrush(Qt::NoBrush);
    p.setPen(pen);
    QPoint point;
    QRect ir = option.rect;
    uint tf = Qt::AlignVCenter;

    if (!option.icon.isNull()) {
        QIcon::Mode mode = option.state & QStyle::State_Enabled ? QIcon::Normal : QIcon::Disabled;
        if (mode == QIcon::Normal && option.state & QStyle::State_HasFocus)
            mode = QIcon::Active;
        QIcon::State state = QIcon::Off;
        if (option.state & QStyle::State_On)
            state = QIcon::On;

        QPixmap pixmap = option.icon.pixmap(option.iconSize, mode, state);
        QImage image = pixmap.toImage();
        if(mode != QIcon::Disabled){
            if(d->m_isIconHighlight){
                pixmap = ThemeController::drawColoredPixmap(this->icon().pixmap(iconSize()),highlight);
            }else if(d->m_useCustomIconColor){
                pixmap = ThemeController::drawColoredPixmap(this->icon().pixmap(iconSize()),d->m_iconColor);
            }else{
                pixmap = ThemeController::drawColoredPixmap(this->icon().pixmap(iconSize()),fontColor);
            }
        }

        int w = pixmap.width() / pixmap.devicePixelRatio();
        int h = pixmap.height() / pixmap.devicePixelRatio();

        if (!option.text.isEmpty())
            w += option.fontMetrics.boundingRect(option.rect, tf, option.text).width() + 2;

        point = QPoint(ir.x() + ir.width() / 2 - w / 2,
                       ir.y() + ir.height() / 2 - h / 2);

        w = pixmap.width() / pixmap.devicePixelRatio();

        if (option.direction == Qt::RightToLeft)
            point.rx() += w;
        if(mode == QIcon::Disabled){
            qreal opacity = 0.35;
            QPainter painter(&image);
            painter.setOpacity(opacity);
            painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
            painter.fillRect(image.rect(),fontColor);
            painter.end();
            p.drawPixmap(this->style()->visualPos(option.direction, option.rect, point), QPixmap::fromImage(image));
        }
        else
            p.drawPixmap(this->style()->visualPos(option.direction, option.rect, point), pixmap);

        if (option.direction == Qt::RightToLeft)
            ir.translate(-point.x() - 2, 0);
        else
            ir.translate(point.x() + w + 4, 0);

        // left-align text if there is
        if (!option.text.isEmpty())
            tf |= Qt::AlignLeft;
    }
    else
    {
        tf |= Qt::AlignHCenter;
    }

    QFontMetrics fm = option.fontMetrics;

    if(!this->isEnabled()){
        qreal opacity = 0.35;
        fontColor.setAlphaF(opacity);
        QPen pen ;
        pen.setColor(fontColor);
        p.setPen(pen);
        if(option.icon.isNull() && (fm.width(option.text) > option.rect.width() - 10))
        {
            QString elidedText = fm.elidedText(option.text,Qt::ElideRight,option.rect.width() - 10);
            p.drawText(option.rect.adjusted(5,0,-5,0),tf,elidedText);
            setToolTip(option.text);
        }
        else
            p.drawText(ir,tf,option.text);

    }
    else
    {
        if(option.icon.isNull() && (fm.width(option.text) > option.rect.width() - 10)){
            QString elidedText = fm.elidedText(option.text,Qt::ElideRight,option.rect.width() - 10);
            p.drawText(option.rect.adjusted(5,0,-5,0),tf,elidedText);
            setToolTip(option.text);
        }
        else
            p.drawText(ir,tf,option.text);
    }
}

void KPushButtonPrivate::changeTheme()
{
    Q_Q(KPushButton);
    if(m_isTranslucentFlag)
    {
        if(ThemeController::widgetTheme() == ClassicTheme)
            m_isTranslucent = false;
        else
            m_isTranslucent = true;
    }
    else
    {
        m_isTranslucent = false;
    }
    initThemeStyle();
}

}
#include "kpushbutton.moc"
#include "moc_kpushbutton.cpp"
