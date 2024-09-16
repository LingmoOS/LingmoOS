// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dquickdciicon_p.h"

DQUICK_BEGIN_NAMESPACE

class DQuickDciIconPrivate : public QSharedData
{
public:
    int width = -1;
    int height = -1;
    QString name;
    QUrl source;
    DQMLGlobalObject::ControlState mode = DQMLGlobalObject::NormalState;
    DGuiApplicationHelper::ColorType theme = DGuiApplicationHelper::ColorType::LightType;
    DDciIconPalette palette;
    bool fallbackToQIcon = true;
};

/*!
    \class Dtk::Quick::DQuickDciIcon
    \inmodule dtkdeclarative

    \brief A dci icon type for qml.

    This is a DCI icon type that can be added to QML.
    Since controls in QML Quick cannot be inherited
    individually, we can only inherit some internal
    controls and internalize them.

    It may not show up directly in your help documentation,
    but you can still use it normally. For example, you can
    use it in a Button control, such as the code below:
    \code
    import org.deepin.dtk.controls 1.0 as D
    Button {
        id: btn
        text: "A Button"

        icon.name: "button_add"
        icon.width: 32
        D.DciIcon.type: D.DciIcon.TextType
    }
    \endcode

    You may need to know about DCI icons. The DCI icon can automatically
    return the icon you need to display according to the DCI file you
    provide and the current icon status and icon mode.
    For example, You might need to highlight a selected icon, All you need to do
    is provide a DCI file that holds various schema icons and choose to use the
    Text or Action type, so we can help you with the task of highlighting icons.

    \note If your DCI file does not contain icons under the corresponding Mode,
    we will render the icons in the Normal Mode you provided to complete the state you want.

    \warning Icon type icons will not be rendered in different modes.
 */

DQuickDciIcon::DQuickDciIcon()
    : d(new DQuickDciIconPrivate)
{
}

DQuickDciIcon::DQuickDciIcon(const DQuickDciIcon &other)
    : d(other.d)
{
}

DQuickDciIcon::~DQuickDciIcon()
{
}

DQuickDciIcon &DQuickDciIcon::operator=(const DQuickDciIcon &other)
{
    d = other.d;
    return *this;
}

bool DQuickDciIcon::operator==(const DQuickDciIcon &other) const
{
    return d == other.d || (d->name == other.name()
                            && d->width == other.width()
                            && d->height == other.height()
                            && d->mode == other.mode()
                            && d->theme == other.theme()
                            && d->palette == other.palette());
}

bool DQuickDciIcon::operator!=(const DQuickDciIcon &other) const
{
    return !(*this == other);
}

/*!
    \brief Determine whether the icon data is empty.

    \return Return true if is empty, otherwise return false.
 */
bool DQuickDciIcon::isEmpty() const
{
    return d->name.isEmpty() && d->source.isEmpty();
}

/*!
    \property Dtk::Quick::DQuickDciIcon::name

    \brief Represents the path of the icon.

    Typically, \return the path to the DCI file.
 */

QString DQuickDciIcon::name() const
{
    return d->name;
}

void DQuickDciIcon::setName(const QString &source)
{
    d->name = source;
}

void DQuickDciIcon::resetName()
{
    d->name = QString();
}

/*!
    \property Dtk::Quick::DQuickDciIcon::width

    \brief Represents the width of the icon.
 */

int DQuickDciIcon::width() const
{
    return d->width;
}

void DQuickDciIcon::setWidth(int width)
{
    d->width = width;
}

void DQuickDciIcon::resetWidth()
{
    d->width = 0;
}

/*!
    \property Dtk::Quick::DQuickDciIcon::height

    \brief Represents the height of the icon.
 */

int DQuickDciIcon::height() const
{
    return d->height;
}

void DQuickDciIcon::setHeight(int height)
{
    d->height = height;
}

void DQuickDciIcon::resetHeight()
{
    d->height = 0;
}

/*!
    \property Dtk::Quick::DQuickDciIcon::mode

    \brief Represents the mode of the icon.
 */

DQMLGlobalObject::ControlState DQuickDciIcon::mode() const
{
    return d->mode;
}

void DQuickDciIcon::setMode(DQMLGlobalObject::ControlState mode)
{
    d->mode = mode;
}

void DQuickDciIcon::resetMode()
{
    d->mode = DQMLGlobalObject::NormalState;
}

/*!
    \property Dtk::Quick::DQuickDciIcon::theme

    \brief Represents the theme of the icon.
 */

DGuiApplicationHelper::ColorType DQuickDciIcon::theme() const
{
    return d->theme;
}

void DQuickDciIcon::setTheme(DGuiApplicationHelper::ColorType theme)
{
    d->theme  = theme;
}

void DQuickDciIcon::resetTheme()
{
    d->theme = DGuiApplicationHelper::ColorType::LightType;
}

/*!
    \property Dtk::Quick::DQuickDciIcon::palette

    \brief Represents the palette of the icon.
 */

DDciIconPalette DQuickDciIcon::palette() const
{
    return d->palette;
}

void DQuickDciIcon::setPalette(const DDciIconPalette &palette)
{
    d->palette = palette;
}

void DQuickDciIcon::resetPalette()
{
    d->palette = DDciIconPalette();
}

const QUrl &DQuickDciIcon::source() const
{
    return d->source;
}

void DQuickDciIcon::setSource(const QUrl &newSource)
{
    d->source = newSource;
}

void DQuickDciIcon::resetSource()
{
    d->source.clear();
}

bool DQuickDciIcon::fallbackToQIcon() const
{
    return d->fallbackToQIcon;
}

void DQuickDciIcon::setFallbackToQIcon(bool newFallbackToQIcon)
{
    d->fallbackToQIcon = newFallbackToQIcon;
}

void DQuickDciIcon::resetFallbackToQIcon()
{
    d->fallbackToQIcon = true;
}

DQUICK_END_NAMESPACE
