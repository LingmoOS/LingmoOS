/*
 * Qt5-LINGMO's Library
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "highlight-effect.h"

#include <QAbstractItemView>
#include <QMenu>
#include <QStyleOption>

#include <QPixmap>
#include <QPainter>

#include <QImage>
#include <QtMath>

#include <QApplication>

#include <QDebug>

#define ColorDifference 10

QColor HighLightEffect::symbolic_color = QColor(38, 38, 38);

void HighLightEffect::setSkipEffect(QWidget *w, bool skip)
{
    w->setProperty("skipHighlightIconEffect", skip);
}



bool HighLightEffect::isPixmapPureColor(const QPixmap &pixmap)
{
    if (pixmap.isNull()) {
        qWarning("pixmap is null!");
        return false;
    }
    QImage image = pixmap.toImage();

    QVector<QColor> vector;
    int total_red = 0;
    int total_green = 0;
    int total_blue = 0;
    bool pure = true;
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            if (image.pixelColor(x, y).alphaF() > 0.3) {
                QColor color = image.pixelColor(x, y);
                vector << color;
                total_red += color.red();
                total_green += color.green();
                total_blue += color.blue();
                int dr = qAbs(color.red() - symbolic_color.red());
                int dg = qAbs(color.green() - symbolic_color.green());
                int db = qAbs(color.blue() - symbolic_color.blue());
                if (dr > ColorDifference || dg > ColorDifference || db > ColorDifference)
                    pure = false;
            }
        }
    }

    if (pure)
        return true;

    qreal squareRoot_red = 0;
    qreal squareRoot_green = 0;
    qreal squareRoot_blue = 0;
    qreal average_red = total_red / vector.count();
    qreal average_green = total_green / vector.count();
    qreal average_blue = total_blue / vector.count();
    for (QColor color : vector) {
        squareRoot_red += (color.red() - average_red) * (color.red() - average_red);
        squareRoot_green += (color.green() - average_green) * (color.green() - average_green);
        squareRoot_blue += (color.blue() - average_blue) * (color.blue() - average_blue);
    }

    qreal arithmeticSquareRoot_red = qSqrt(squareRoot_red / vector.count());
    qreal arithmeticSquareRoot_green = qSqrt(squareRoot_green / vector.count());
    qreal arithmeticSquareRoot_blue = qSqrt(squareRoot_blue / vector.count());
    return arithmeticSquareRoot_red < 2.0 && arithmeticSquareRoot_green < 2.0 && arithmeticSquareRoot_blue < 2.0;
}


bool HighLightEffect::isSymbolicColor(const QPixmap &pixmap)
{
    if (pixmap.isNull()) {
        qWarning("pixmap is null!");
        return false;
    }
    QImage image = pixmap.toImage();

    bool pure = true;
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            if (image.pixelColor(x, y).alphaF() > 0.3) {
                QColor color = image.pixelColor(x, y);

                //default symbolic color (38, 38, 38)
                int dr = qAbs(color.red() - symbolic_color.red());
                int dg = qAbs(color.green() - symbolic_color.green());
                int db = qAbs(color.blue() - symbolic_color.blue());
                if (dr > ColorDifference || dg > ColorDifference || db > ColorDifference)
                    pure = false;
            }
        }
    }

    return pure;
}


bool HighLightEffect::setMenuIconHighlightEffect(QMenu *menu, HighLightMode hlmode, EffectMode mode)
{
    if (!menu)
        return false;

    menu->setProperty("useIconHighlightEffect", hlmode);
    menu->setProperty("iconHighlightEffectMode", mode);
    return true;
}

bool HighLightEffect::setViewItemIconHighlightEffect(QAbstractItemView *view, HighLightMode hlmode, EffectMode mode)
{
    if (!view)
        return false;

    view->viewport()->setProperty("useIconHighlightEffect", hlmode);
    view->viewport()->setProperty("iconHighlightEffectMode", mode);
    return true;
}

HighLightEffect::HighLightMode HighLightEffect::isWidgetIconUseHighlightEffect(const QWidget *w)
{
    if (w) {
        if (w->property("useIconHighlightEffect").isValid())
            return HighLightMode(w->property("useIconHighlightEffect").toInt());
    }
    return HighLightEffect::skipHighlight;
}

void HighLightEffect::setSymoblicColor(const QColor &color)
{
    qApp->setProperty("symbolicColor", color);
    symbolic_color = color;
}

void HighLightEffect::setWidgetIconFillSymbolicColor(QWidget *widget, bool fill)
{
    widget->setProperty("fillIconSymbolicColor", fill);
}

const QColor HighLightEffect::getCurrentSymbolicColor()
{
    QIcon symbolic = QIcon::fromTheme("window-new-symbolic");
    QPixmap pix = symbolic.pixmap(QSize(16, 16));
    QImage img = pix.toImage();
    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            QColor color = img.pixelColor(x, y);
            if (color.alpha() > 0) {
                symbolic_color = color;
                return color;
            }
        }
    }
    return symbolic_color;
}

const QColor HighLightEffect::defaultStyleDark(const QStyleOption *option)
{
    auto windowText = qApp->palette().color(QPalette::Active, QPalette::WindowText);
    qreal h, s, v;

    if (option) {
        windowText = option->palette.color(QPalette::Active, QPalette::WindowText);
    }

    windowText.getHsvF(&h, &s, &v);
    return QColor::fromHsvF(h, s, v);
}

QPixmap HighLightEffect::generatePixmap(const QPixmap &pixmap, const QStyleOption *option, const QWidget *widget, bool force, EffectMode mode)
{
    if (pixmap.isNull())
        return pixmap;
    if (!(option->state & QStyle::State_Enabled))
        return pixmap;
    if (widget && !widget->isEnabled())
        return pixmap;

    QPixmap target = pixmap;
    bool isPurePixmap = isPixmapPureColor(pixmap);
    if (force) {
        if (!isPurePixmap)
            return pixmap;

        QPainter p(&target);
        p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        if (option->state & QStyle::State_MouseOver ||
                option->state & QStyle::State_Selected ||
                option->state & QStyle::State_On ||
                option->state & QStyle::State_Sunken) {
            p.fillRect(target.rect(), option->palette.highlightedText());
        } else {
                p.fillRect(target.rect(), mode ? option->palette.text() : defaultStyleDark(option));
        }
        return target;
    }



    if (!widget) {
        return pixmap;
    }

    if (widget->property("skipHighlightIconEffect").isValid()) {
        bool skipEffect = widget->property("skipHighlightIconEffect").toBool();
        if (skipEffect)
            return pixmap;
    }

    if (widget->property("iconHighlightEffectMode").isValid()) {
        mode = EffectMode(widget->property("iconHighlightEffectMode").toBool());
    }

    HighLightMode hlmode = isWidgetIconUseHighlightEffect(widget);

    if (hlmode == skipHighlight) {
        return pixmap;
    } else if (hlmode == HighlightEffect) {
        bool fillIconSymbolicColor = false;
        if (widget->property("fillIconSymbolicColor").isValid()) {
            fillIconSymbolicColor = widget->property("fillIconSymbolicColor").toBool();
        }

        bool isEnable = option->state.testFlag(QStyle::State_Enabled);
        bool overOrDown =  option->state.testFlag(QStyle::State_MouseOver) ||
                option->state.testFlag(QStyle::State_Sunken) ||
                option->state.testFlag(QStyle::State_On) ||
                option->state.testFlag(QStyle::State_Selected);

        if (qobject_cast<const QAbstractItemView *>(widget)) {
            if (!option->state.testFlag(QStyle::State_Selected))
                overOrDown = false;
        }

        if (isEnable && overOrDown) {
            if (fillIconSymbolicColor) {
                target = filledSymbolicColoredPixmap(pixmap, option->palette.highlightedText().color());
            }

            if (!isPurePixmap)
                return target;

            QPainter p(&target);
            p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
            p.setCompositionMode(QPainter::CompositionMode_SourceIn);
            p.fillRect(target.rect(), option->palette.highlightedText());
            return target;
        } else {
            QPixmap target = pixmap;
            if (fillIconSymbolicColor) {
                target = filledSymbolicColoredPixmap(pixmap, option->palette.highlightedText().color());
            }
            if (!isPurePixmap)
                return target;

            QPainter p(&target);
            p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
            p.setCompositionMode(QPainter::CompositionMode_SourceIn);
            p.fillRect(target.rect(), mode ? option->palette.text() : defaultStyleDark(option));
            return target;
        }
    } else if (hlmode == ordinaryHighLight) {
        return ordinaryGeneratePixmap(pixmap, option, widget, mode);
    } else if (hlmode == hoverHighLight) {
        return hoverGeneratePixmap(pixmap, option, widget);
    } else if (hlmode == defaultHighLight) {
        return bothOrdinaryAndHoverGeneratePixmap(pixmap, option, widget, mode);
    } else if (hlmode == filledSymbolicColorHighLight) {
        if (isPurePixmap) {
            return bothOrdinaryAndHoverGeneratePixmap(pixmap, option, widget, mode);
        } else {
            return filledSymbolicColoredGeneratePixmap(pixmap, option, widget, mode);
        }
    } else {
        return pixmap;
    }

    return pixmap;
}

QPixmap HighLightEffect::ordinaryGeneratePixmap(const QPixmap &pixmap, const QStyleOption *option, const QWidget *widget, EffectMode mode)
{
    if (pixmap.isNull())
        return pixmap;
    if (!isPixmapPureColor(pixmap) || !(option->state & QStyle::State_Enabled))
        return pixmap;
    if (!isSymbolicColor(pixmap))
        return pixmap;

    QPixmap target = pixmap;
    QColor color;
    if (widget && widget->property("setIconHighlightEffectDefaultColor").isValid() && widget->property("setIconHighlightEffectDefaultColor").canConvert<QColor>()) {
        color = widget->property("setIconHighlightEffectDefaultColor").value<QColor>();
    }

    if (widget && widget->property("iconHighlightEffectMode").isValid()) {
        mode = EffectMode(widget->property("iconHighlightEffectMode").toBool());
    }

    QPainter p(&target);
    p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(target.rect(), color.isValid() ? color : (mode ? option->palette.text() : defaultStyleDark(option)));
    return target;
}

QPixmap HighLightEffect::hoverGeneratePixmap(const QPixmap &pixmap, const QStyleOption *option, const QWidget *widget, EffectMode mode)
{
    if (pixmap.isNull())
        return pixmap;
    if (!isPixmapPureColor(pixmap) || !(option->state & QStyle::State_Enabled))
        return pixmap;
    if (!isSymbolicColor(pixmap))
        return pixmap;

    QPixmap target = pixmap;
    QColor color;
    if (widget && widget->property("setIconHighlightEffectHoverColor").isValid() && widget->property("setIconHighlightEffectHoverColor").canConvert<QColor>()) {
        color = widget->property("setIconHighlightEffectHoverColor").value<QColor>();
    }

    if (widget && widget->property("iconHighlightEffectMode").isValid()) {
        mode = EffectMode(widget->property("iconHighlightEffectMode").toBool());
    }

    bool overOrDown =  option->state.testFlag(QStyle::State_MouseOver) ||
            option->state.testFlag(QStyle::State_Sunken) ||
            option->state.testFlag(QStyle::State_On) ||
            option->state.testFlag(QStyle::State_Selected);
    if (qobject_cast<const QAbstractItemView *>(widget)) {
        if (!option->state.testFlag(QStyle::State_Selected))
            overOrDown = false;
    }

    QPainter p(&target);
    if (overOrDown) {
        p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        p.fillRect(target.rect(), color.isValid() ? color : option->palette.highlightedText());
    }
    return target;
}

QPixmap HighLightEffect::bothOrdinaryAndHoverGeneratePixmap(const QPixmap &pixmap, const QStyleOption *option, const QWidget *widget, EffectMode mode)
{
    if (pixmap.isNull())
        return pixmap;
    if (!isPixmapPureColor(pixmap) || !(option->state & QStyle::State_Enabled))
        return pixmap;
    if (!isSymbolicColor(pixmap))
        return pixmap;

    QPixmap target = pixmap;
    QColor defaultColor, hoverColor;
    if (widget && widget->property("setIconHighlightEffectDefaultColor").isValid() && widget->property("setIconHighlightEffectDefaultColor").canConvert<QColor>()) {
        defaultColor = widget->property("setIconHighlightEffectDefaultColor").value<QColor>();
    }
    if (widget && widget->property("setIconHighlightEffectHoverColor").isValid() && widget->property("setIconHighlightEffectHoverColor").canConvert<QColor>()) {
        hoverColor = widget->property("setIconHighlightEffectHoverColor").value<QColor>();
    }

    if (widget && widget->property("iconHighlightEffectMode").isValid()) {
        mode = EffectMode(widget->property("iconHighlightEffectMode").toBool());
    }

    bool overOrDown =  option->state.testFlag(QStyle::State_MouseOver) ||
            option->state.testFlag(QStyle::State_Sunken) ||
            option->state.testFlag(QStyle::State_On) ||
            option->state.testFlag(QStyle::State_Selected);

    if (qobject_cast<const QAbstractItemView *>(widget)) {
        if (!option->state.testFlag(QStyle::State_Selected))
            overOrDown = false;
    } else if (option->styleObject && option->styleObject->inherits("QQuickStyleItem1")) {
        // 修复qml菜单高亮状态异常问题
        overOrDown = option->state.testFlag(QStyle::State_MouseOver) ||
                option->state.testFlag(QStyle::State_Sunken) ||
                option->state.testFlag(QStyle::State_Selected);
    }

    QPainter p(&target);
    if (overOrDown) {
        p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        p.fillRect(target.rect(), hoverColor.isValid() ? hoverColor : option->palette.highlightedText());
    } else {
        p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        p.fillRect(target.rect(), defaultColor.isValid() ? defaultColor : (mode ? option->palette.text() : defaultStyleDark(option)));
    }
    return target;
}

QPixmap HighLightEffect::filledSymbolicColoredGeneratePixmap(const QPixmap &pixmap, const QStyleOption *option, const QWidget *widget, EffectMode mode)
{
    if (pixmap.isNull())
        return pixmap;
    if (isPixmapPureColor(pixmap))
        return bothOrdinaryAndHoverGeneratePixmap(pixmap, option, widget, mode);

    QPixmap target = pixmap;
    QColor defaultColor, hoverColor;
    if (widget && widget->property("setIconHighlightEffectDefaultColor").isValid() && widget->property("setIconHighlightEffectDefaultColor").canConvert<QColor>()) {
        defaultColor = widget->property("setIconHighlightEffectDefaultColor").value<QColor>();
    }
    if (widget && widget->property("setIconHighlightEffectHoverColor").isValid() && widget->property("setIconHighlightEffectHoverColor").canConvert<QColor>()) {
        hoverColor = widget->property("setIconHighlightEffectHoverColor").value<QColor>();
    }

    if (widget && widget->property("iconHighlightEffectMode").isValid()) {
        mode = EffectMode(widget->property("iconHighlightEffectMode").toBool());
    }
    bool isEnable = option->state.testFlag(QStyle::State_Enabled);
    bool overOrDown =  option->state.testFlag(QStyle::State_MouseOver) ||
            option->state.testFlag(QStyle::State_Sunken) ||
            option->state.testFlag(QStyle::State_On) ||
            option->state.testFlag(QStyle::State_Selected);
    if (qobject_cast<const QAbstractItemView *>(widget)) {
        if (!option->state.testFlag(QStyle::State_Selected))
            overOrDown = false;
    }

    if (isEnable && overOrDown) {
        return filledSymbolicColoredPixmap(target, hoverColor.isValid() ? hoverColor : option->palette.color(QPalette::HighlightedText));
    } else {
        return filledSymbolicColoredPixmap(target, defaultColor.isValid() ? defaultColor : (mode ? option->palette.color(QPalette::Text) : defaultStyleDark(option)));
    }

    return target;
}

HighLightEffect::HighLightEffect(QObject *parent) : QObject(parent)
{

}

QPixmap HighLightEffect::filledSymbolicColoredPixmap(const QPixmap &source, const QColor &baseColor)
{
    if (source.isNull())
        return source;
    QImage img = source.toImage();
    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            auto color = img.pixelColor(x, y);
            if (color.alpha() > 0.3) {
                if (qAbs(color.red() - symbolic_color.red()) < 10 && qAbs(color.green() - symbolic_color.green()) < 10
                        && qAbs(color.blue() - symbolic_color.blue()) < 10) {
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
