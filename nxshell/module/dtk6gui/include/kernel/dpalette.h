// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DPALETTE_H
#define DPALETTE_H

#include <dtkgui_global.h>

#include <QDebug>
#include <QPalette>

DGUI_BEGIN_NAMESPACE

class DPalettePrivate;
class DPalette : public QPalette
{
    Q_GADGET
public:
    enum ColorType {
        NoType,
        ItemBackground,     //列表项的背景色
        TextTitle,          //标题型文本的颜色
        TextTips,           //提示性文本的颜色
        TextWarning,        //警告类型的文本颜色
        TextLively,         //活跃式文本颜色（不受活动色影响）
        LightLively,        //活跃式按钮（recommend button）背景色中的亮色（不受活跃色影响）
        DarkLively,         //活跃式按钮（recommend button）背景色中的暗色，会从亮色渐变到暗色（不受活跃色影响）
        FrameBorder,        //控件边框颜色
        PlaceholderText,    //占位类型的文本颜色，可用于输入框占位内容等提示性文字
        FrameShadowBorder,  //用于跟阴影叠加的边框颜色
        ObviousBackground,  //明显的背景色
        NColorTypes
    };
    Q_ENUM(ColorType)

    DPalette();
    DPalette(const QPalette &palette);
    DPalette(const DPalette &palette);
    ~DPalette();

    DPalette &operator=(const DPalette &palette);

    inline const QColor &color(ColorGroup cg, ColorType ct) const
    { return brush(cg, ct).color(); }
    const QBrush &brush(ColorGroup cg, ColorType ct) const;
    inline void setColor(ColorGroup cg, ColorType ct, const QColor &color)
    { setBrush(cg, ct, color); }
    inline void setColor(ColorType ct, const QColor &color)
    { setColor(All, ct, color); }
    inline void setBrush(ColorType ct, const QBrush &brush)
    { setBrush(All, ct, brush); }
    void setBrush(ColorGroup cg, ColorType ct, const QBrush &brush);

    inline const QColor &color(ColorType ct) const { return color(Current, ct); }
    inline const QBrush &brush(ColorType ct) const { return brush(Current, ct); }
    inline const QBrush &itemBackground() const { return brush(ItemBackground); }
    inline const QBrush &textTitle() const { return brush(TextTitle); }
#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
    D_DECL_DEPRECATED inline const QBrush &textTiele() const { return textTitle();}
#endif
    inline const QBrush &textTips() const { return brush(TextTips); }
    inline const QBrush &textWarning() const { return brush(TextWarning); }
    inline const QBrush &textLively() const { return brush(TextLively); }
    inline const QBrush &lightLively() const { return brush(LightLively); }
    inline const QBrush &darkLively() const { return brush(DarkLively); }
    inline const QBrush &frameBorder() const { return brush(FrameBorder); }
    inline const QBrush &placeholderText() const { return brush(PlaceholderText); }
    inline const QBrush &frameShadowBorder() const { return brush(FrameShadowBorder); }

    using QPalette::color;
    using QPalette::brush;
    using QPalette::setBrush;
    using QPalette::setColor;

protected:
    QScopedPointer<DPalettePrivate> d;

    friend Q_GUI_EXPORT QDataStream &operator<<(QDataStream &s, const DPalette &p);
};

DGUI_END_NAMESPACE

QT_BEGIN_NAMESPACE
/*****************************************************************************
  DPalette stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
Q_GUI_EXPORT QDataStream &operator<<(QDataStream &ds, const DTK_GUI_NAMESPACE::DPalette &p);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &ds, DTK_GUI_NAMESPACE::DPalette &p);
#endif // QT_NO_DATASTREAM

#ifndef QT_NO_DEBUG_STREAM
Q_GUI_EXPORT QDebug operator<<(QDebug, const DTK_GUI_NAMESPACE::DPalette &);
#endif

QT_END_NAMESPACE

#endif // DPALETTE_H
