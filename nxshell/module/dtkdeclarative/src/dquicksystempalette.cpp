// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dquicksystempalette.h"
#include "private/dquicksystempalette_p.h"

#include <DGuiApplicationHelper>
#include <QDebug>

DGUI_USE_NAMESPACE
DQUICK_BEGIN_NAMESPACE

#define RETURNCOLOR_BYROLE(_role) \
    { \
        D_DC(DQuickSystemPalette); \
        return d->palette.color(d->colorGroup, (_role)); \
    }

DQuickSystemPalettePrivate::DQuickSystemPalettePrivate(DQuickSystemPalette *qq)
    : DObjectPrivate(qq)
{
    // 初始化调色板
    palette = DGuiApplicationHelper::instance()->applicationPalette();
}

DQuickSystemPalette::DQuickSystemPalette(QObject *parent)
    : QObject(parent)
    , DObject(*new DQuickSystemPalettePrivate(this))
{
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::applicationPaletteChanged, this, [this] {
        // 重置调色板
        D_D(DQuickSystemPalette);
        d->palette = DGuiApplicationHelper::instance()->applicationPalette();
        Q_EMIT paletteChanged();
    });
}

/*!
 * \~chinese \class DQuickSystemPalette
 *
 * \~chinese \brief DQuickSystemPalette 是 DTk 自定义 QML 类型，用于在 QML 中使用调色板相关功能的类。
 * \~chinese 继承自 QObject,DObject,QQmlParserStatus，需要在 QML 中配合　Item 一起使用,通过
 * \~chinese 属性 sourceItem 可以绑定一个 Item 控件，根据控件状态获取相应调色板调色板的值。
 */
DQuickSystemPalette::~DQuickSystemPalette()
{

}

const DPalette &DQuickSystemPalette::palette() const
{
    D_DC(DQuickSystemPalette);
    return d->palette;
}

DQuickSystemPalette::ColorGroup DQuickSystemPalette::colorGroup() const
{
    D_DC(DQuickSystemPalette);
    return static_cast<DQuickSystemPalette::ColorGroup>(d->colorGroup);
}

/*!
 * \~chinese \brief DQuickSystemPalette::window　window　用于获取　DPalette 的　window 类型的颜色值。
 * \~chinese \return window 返回　DPalette　的 window 类型的颜色值。
 */
QColor DQuickSystemPalette::window() const
{
    RETURNCOLOR_BYROLE(QPalette::Window);
}

/*!
 * \~chinese \brief DQuickSystemPalette::window　windowText　用于获取　DPalette 的　windowText 类型的颜色值。
 * \~chinese \return windowText 返回　DPalette　的 windowText 类型的颜色值。
 */
QColor DQuickSystemPalette::windowText() const
{
    RETURNCOLOR_BYROLE(QPalette::WindowText);
}

/*!
 * \~chinese \brief DQuickSystemPalette::window　base　用于获取　DPalette 的　base 类型的颜色值。
 * \~chinese \return base 返回　DPalette　的 base 类型的颜色值。
 */
QColor DQuickSystemPalette::base() const
{
    RETURNCOLOR_BYROLE(QPalette::Base);
}

/*!
 * \~chinese \brief DQuickSystemPalette::window　text　用于获取　DPalette 的　text 类型的颜色值。
 * \~chinese \return text 返回　DPalette　的 text 类型的颜色值。
 */
QColor DQuickSystemPalette::text() const
{
    RETURNCOLOR_BYROLE(QPalette::Text);
}

/*!
 * \~chinese \brief DQuickSystemPalette::window　alternateBase　用于获取　DPalette 的　alternateBase 类型的颜色值。
 * \~chinese \return alternateBase 返回　DPalette　的 alternateBase 类型的颜色值。
 */
QColor DQuickSystemPalette::alternateBase() const
{
    RETURNCOLOR_BYROLE(QPalette::AlternateBase);
}

/*!
 * \~chinese \brief DQuickSystemPalette::window　button　用于获取　DPalette 的　button 类型的颜色值。
 * \~chinese \return button 返回　DPalette　的 button 类型的颜色值。
 */
QColor DQuickSystemPalette::button() const
{
    RETURNCOLOR_BYROLE(QPalette::Button);
}

/*!
 * \~chinese \brief DQuickSystemPalette::window　buttonText　用于获取　DPalette 的　buttonText 类型的颜色值。
 * \~chinese \return buttonText 返回　DPalette　的 buttonText 类型的颜色值。
 */
QColor DQuickSystemPalette::buttonText() const
{
    RETURNCOLOR_BYROLE(QPalette::ButtonText);
}

/*!
 * \~chinese \brief DQuickSystemPalette::light　light　用于获取　DPalette 的　light 类型的颜色值。
 * \~chinese \return light 返回　DPalette　的 light 类型的颜色值。
 */
QColor DQuickSystemPalette::light() const
{
    RETURNCOLOR_BYROLE(QPalette::Light);
}

/*!
 * \~chinese \brief DQuickSystemPalette::midlight　midlight　用于获取　DPalette 的　midlight 类型的颜色值。
 * \~chinese \return midlight 返回　DPalette　的 midlight 类型的颜色值。
 */
QColor DQuickSystemPalette::midlight() const
{
    RETURNCOLOR_BYROLE(QPalette::Midlight);
}

/*!
 * \~chinese \brief DQuickSystemPalette::dark　dark　用于获取　DPalette 的　dark 类型的颜色值。
 * \~chinese \return dark 返回　DPalette　的 dark 类型的颜色值。
 */
QColor DQuickSystemPalette::dark() const
{
    RETURNCOLOR_BYROLE(QPalette::Dark);
}

/*!
 * \~chinese \brief DQuickSystemPalette::mid　mid　用于获取　DPalette 的　mid 类型的颜色值。
 * \~chinese \return mid 返回　DPalette　的 mid 类型的颜色值。
 */
QColor DQuickSystemPalette::mid() const
{
    RETURNCOLOR_BYROLE(QPalette::Mid);
}

/*!
 * \~chinese \brief DQuickSystemPalette::shadow　shadow　用于获取　DPalette 的　shadow 类型的颜色值。
 * \~chinese \return shadow 返回　DPalette　的 shadow 类型的颜色值。
 */
QColor DQuickSystemPalette::shadow() const
{
    RETURNCOLOR_BYROLE(QPalette::Shadow);
}

/*!
 * \~chinese \brief DQuickSystemPalette::highlight　highlight　用于获取　DPalette 的　highlight 类型的颜色值。
 * \~chinese \return highlight 返回　DPalette　的 highlight 类型的颜色值。
 */
QColor DQuickSystemPalette::highlight() const
{
    RETURNCOLOR_BYROLE(QPalette::Highlight);
}

/*!
 * \~chinese \brief DQuickSystemPalette::highlightedText　highlightedText　用于获取　DPalette 的　highlightedText 类型的颜色值。
 * \~chinese \return highlightedText 返回　DPalette　的 highlightedText 类型的颜色值。
 */
QColor DQuickSystemPalette::highlightedText() const
{
    RETURNCOLOR_BYROLE(QPalette::HighlightedText);
}

/*!
 * \~chinese \brief DQuickSystemPalette::itemBackground　itemBackground　用于获取　DPalette 的　itemBackground 类型的颜色值。
 * \~chinese \return itemBackground 返回　DPalette　的 itemBackground 类型的颜色值。
 */
QColor DQuickSystemPalette::itemBackground() const
{
    RETURNCOLOR_BYROLE(DPalette::ItemBackground);
}

/*!
 * \~chinese \brief DQuickSystemPalette::textTitle　textTitle　用于获取　DPalette 的　textTitle 类型的颜色值。
 * \~chinese \return textTitle 返回　DPalette　的 textTitle 类型的颜色值。
 */
QColor DQuickSystemPalette::textTitle() const
{
    RETURNCOLOR_BYROLE(DPalette::TextTitle);
}

/*!
 * \~chinese \brief DQuickSystemPalette::textTips　textTips　用于获取　DPalette 的　textTips 类型的颜色值。
 * \~chinese \return textTips 返回　DPalette　的 textTips 类型的颜色值。
 */
QColor DQuickSystemPalette::textTips() const
{
    RETURNCOLOR_BYROLE(DPalette::TextTips);
}

/*!
 * \~chinese \brief DQuickSystemPalette::textWarning　textWarning　用于获取　DPalette 的　textWarning 类型的颜色值。
 * \~chinese \return textWarning 返回　DPalette　的 textWarning 类型的颜色值。
 */
QColor DQuickSystemPalette::textWarning() const
{
    RETURNCOLOR_BYROLE(DPalette::TextWarning);
}

/*!
 * \~chinese \brief DQuickSystemPalette::textLively　textLively　用于获取　DPalette 的　textLively 类型的颜色值。
 * \~chinese \return textLively 返回　DPalette　的 textLively 类型的颜色值。
 */
QColor DQuickSystemPalette::textLively() const
{
    RETURNCOLOR_BYROLE(DPalette::TextLively);
}

/*!
 * \~chinese \brief DQuickSystemPalette::lightLively　lightLively　用于获取　DPalette 的　lightLively 类型的颜色值。
 * \~chinese \return lightLively 返回　DPalette　的 lightLively 类型的颜色值。
 */
QColor DQuickSystemPalette::lightLively() const
{
    RETURNCOLOR_BYROLE(DPalette::LightLively);
}

/*!
 * \~chinese \brief DQuickSystemPalette::darkLively　darkLively　用于获取　DPalette 的　darkLively 类型的颜色值。
 * \~chinese \return darkLively 返回　DPalette　的 darkLively 类型的颜色值。
 */
QColor DQuickSystemPalette::darkLively() const
{
    RETURNCOLOR_BYROLE(DPalette::DarkLively);
}

/*!
 * \~chinese \brief DQuickSystemPalette::frameBorder　frameBorder　用于获取　DPalette 的　frameBorder 类型的颜色值。
 * \~chinese \return frameBorder 返回　DPalette　的 frameBorder 类型的颜色值。
 */
QColor DQuickSystemPalette::frameBorder() const
{
    RETURNCOLOR_BYROLE(DPalette::FrameBorder);
}

/*!
 * \~chinese \brief DQuickSystemPalette::placeholderText　placeholderText　用于获取　DPalette 的　placeholderText 类型的颜色值。
 * \~chinese \return placeholderText 返回　DPalette　的 placeholderText 类型的颜色值。
 */
QColor DQuickSystemPalette::placeholderText() const
{
    RETURNCOLOR_BYROLE(DPalette::PlaceholderText);
}

/*!
 * \~chinese \brief DQuickSystemPalette::frameShadowBorder　frameShadowBorder　用于获取　DPalette 的　frameShadowBorder 类型的颜色值。
 * \~chinese \return frameShadowBorder 返回　DPalette　的 frameShadowBorder 类型的颜色值。
 */
QColor DQuickSystemPalette::frameShadowBorder() const
{
    RETURNCOLOR_BYROLE(DPalette::FrameShadowBorder);
}

/*!
 * \~chinese \brief DQuickSystemPalette::obviousBackground　obviousBackground　用于获取　DPalette 的　obviousBackground 类型的颜色值。
 * \~chinese \return obviousBackground 返回　DPalette　的 obviousBackground 类型的颜色值。
 */
QColor DQuickSystemPalette::obviousBackground() const
{
    RETURNCOLOR_BYROLE(DPalette::ObviousBackground);
}

void DQuickSystemPalette::setColorGroup(DQuickSystemPalette::ColorGroup colorGroup)
{
    D_D(DQuickSystemPalette);
    if (d->colorGroup == static_cast<QPalette::ColorGroup>(colorGroup))
        return;

    d->colorGroup = static_cast<QPalette::ColorGroup>(colorGroup);
    Q_EMIT paletteChanged();
}

DQUICK_END_NAMESPACE
