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

#ifndef HIGHLIGHTEFFECT_H
#define HIGHLIGHTEFFECT_H

#include <QObject>
#include <QStyleOption>

class QAbstractItemView;
class QMenu;

class HighLightEffect : public QObject
{
    Q_OBJECT
public:
    enum HighLightMode {
        skipHighlight = 0x0,
        HighlightEffect = 0x1,
        ordinaryHighLight = 0x2,
        hoverHighLight = 0x4,
        defaultHighLight = 0x8,
        filledSymbolicColorHighLight = 0x10,
    };

    enum EffectMode {
        HighlightOnly = 0x0,
        BothDefaultAndHighlit = 0x1
    };
    Q_ENUM(EffectMode)

    /*!
     * \brief setSkipEffect
     * \param w
     * \param skip
     * \details
     * in lingmo-style, some widget such as menu will be set use highlight
     * icon effect automaticlly,
     * but we might not want to compose a special pure color image.
     * This function is use to skip the effect.
     */
    static QColor symbolic_color;
    static void setSkipEffect(QWidget *w, bool skip = true);
    static bool isPixmapPureColor(const QPixmap &pixmap);
    static bool isSymbolicColor(const QPixmap &pixmap);
    static bool setMenuIconHighlightEffect(QMenu *menu, HighLightMode hlmode = skipHighlight, EffectMode mode = HighlightOnly);
    static bool setViewItemIconHighlightEffect(QAbstractItemView *view, HighLightMode hlmode = skipHighlight, EffectMode mode = HighlightOnly);
    static HighLightMode isWidgetIconUseHighlightEffect(const QWidget *w);

    static void setSymoblicColor(const QColor &color);
    static void setWidgetIconFillSymbolicColor(QWidget *widget, bool fill);

    static const QColor getCurrentSymbolicColor();
    static const QColor defaultStyleDark(const QStyleOption *option);

    static QPixmap generatePixmap(const QPixmap &pixmap, const QStyleOption *option, const QWidget *widget = nullptr, bool force = false, EffectMode mode = HighlightOnly);
    static QPixmap ordinaryGeneratePixmap(const QPixmap &pixmap, const QStyleOption *option, const QWidget *widget = nullptr, EffectMode mode = HighlightOnly);
    static QPixmap hoverGeneratePixmap(const QPixmap &pixmap, const QStyleOption *option, const QWidget *widget = nullptr, EffectMode mode = HighlightOnly);
    static QPixmap bothOrdinaryAndHoverGeneratePixmap(const QPixmap &pixmap, const QStyleOption *option, const QWidget *widget = nullptr, EffectMode mode = HighlightOnly);
    static QPixmap filledSymbolicColoredGeneratePixmap(const QPixmap &pixmap, const QStyleOption *option, const QWidget *widget = nullptr, EffectMode mode = HighlightOnly);

private:
    explicit HighLightEffect(QObject *parent = nullptr);

    static QPixmap filledSymbolicColoredPixmap(const QPixmap &source, const QColor &baseColor);
};

#endif // HIGHLIGHTEFFECT_H
