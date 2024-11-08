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

#ifndef ANIMATORPLUGINIFACE_H
#define ANIMATORPLUGINIFACE_H

#include <QString>

class LINGMOAnimatorPluginIface
{
public:
    enum AnimatorPluginType {
        TabWidget,
        StackedWidget,
        PushButton,
        ItemView,
        ScrollBar,
        MenuBar,
        Menu,
        Other
    };

    virtual ~LINGMOAnimatorPluginIface() {}

    /*!
     * \brief id
     * \return unique plugin's id.
     * \details
     * You have to define a unique id for your animator plugin.
     */
    virtual const QString id() = 0;
    /*!
     * \brief brief
     * \return a brief of animator.
     */
    virtual const QString brief() = 0;

    /*!
     * \brief pluginType
     * \return animator type for indicating what kinds of widget to bind with.
     */
    virtual AnimatorPluginType pluginType() = 0;

    /*!
     * \brief inhertKey
     * \return
     * \details
     * When a style polish a widget, lingmo animation frameworks will bind the animator
     * to the widget. This value is the keyword for judgeing if the widget should be bound
     * with animator. For example, return this value with "QWidget", then all the widgets
     * will be bound.
     */
    virtual const QString inhertKey() = 0;

    /*!
     * \brief excludeKeys
     * \return
     * \details
     * In contrast to inhertKey(), this list is a "blacklist" of
     * widgets should not be bound with the animator.
     */
    virtual const QStringList excludeKeys() = 0;

    /*!
     * \brief isParallel
     * \return
     * \details
     * Indicate if the animator which plugin created is compatible with other
     * animators.
     *
     * \note
     * This variable has no practical effect,
     * but we hope that the animations acting on the same control
     * can be parallelized, although it is difficult to achieve.
     *
     * If you note the animator is not parallelized, other animator
     * might be invalid. For example, the default tabwidget slide animator
     * will hijack the paint event of current tabwidget's tab. This might
     * let other animator can not do a paint in current tab.
     */
    virtual bool isParallel() = 0;
};

#endif // ANIMATORPLUGINIFACE_H
