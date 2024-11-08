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

#ifndef LINGMOTABWIDGETANIMATORPLUGINIFACE_H
#define LINGMOTABWIDGETANIMATORPLUGINIFACE_H

#include <QString>
#include "../animator-plugin-iface.h"
#include "lingmo-tabwidget-animator-iface.h"

#define LINGMOTabWidgetAnimatorPluginInterface_iid "org.lingmo.style.animatons.TabWidgetPluginInterface"

/*!
 * \brief The LINGMOTabWidgetAnimatorPluginIface class
 * \details
 * This class is used to create a tabwidget animator instace.
 *
 * LINGMO Animation's frameworks is desgined to be extensiable.
 * And this interface is an entry of plugin.
 *
 * \see LINGMOTabWidgetAnimatorIface
 */
class LINGMOTabWidgetAnimatorPluginIface : public LINGMOAnimatorPluginIface
{
public:
    virtual ~LINGMOTabWidgetAnimatorPluginIface() {}

    /*!
     * \brief key
     * \return
     * A key word of plugin, such as "slide".
     */
    virtual const QString key() = 0;

    /*!
     * \brief description
     * \return
     * A description for animator. For example, "Animator for do a horizon slide
     * for tab widget."
     */
    virtual const QString description() = 0;

    /*!
     * \brief createAnimator
     * \return
     * an animator instance, for example a LINGMO::TabWidget::DefaultSlideAnimator.
     */
    virtual LINGMOTabWidgetAnimatorIface *createAnimator() = 0;
};

Q_DECLARE_INTERFACE(LINGMOTabWidgetAnimatorPluginIface, LINGMOTabWidgetAnimatorPluginInterface_iid)

#endif // LINGMOTABWIDGETANIMATORPLUGINIFACE_H
