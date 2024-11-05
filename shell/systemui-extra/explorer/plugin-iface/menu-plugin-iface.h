/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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

#ifndef MENUPLUGINIFACE_H
#define MENUPLUGINIFACE_H

#include <QPluginLoader>
#include <QtPlugin>
#include <QString>
#include <QAction>
#include "plugin-iface.h"

#define MenuPluginInterface_iid "org.lingmo.explorer-qt.plugin-iface.MenuPluginInterface"

namespace Peony {

/**
 * @brief The MenuPluginInterface class
 * @details MenuPluginInterface is a Qt Plugin Interface, the methods of this class must all be
 * pure VIRTUAL and has ZERO as return value.
 * Otherwise, the plugin will not be loaded correctly.
 * For example, if we define a virtual QString method, we must do as this:
 * <pre>
 *  virtual QString xxx() = 0;
 * </pre>
 * DO NOT FORGET to initial virtual method in class definition.
 */
class MenuPluginInterface : public PluginInterface
{
public:
    /*!
     * \brief The Type enum
     * \details
     * Type is a enum filter which could help you return different actions
     * by different menu iface type.
     * For example, if you write a desktop background wrapper plugin,
     * you should only return your actions when the passing type is
     * DesktopBackground.
     */
    enum Type {
        Invalid,
        DirectoryView,
        SideBar,
        DesktopWindow,
        Other
    };
    Q_DECLARE_FLAGS(Types, Type)

    virtual ~MenuPluginInterface() {}

    virtual QString testPlugin() = 0;

    /*!
     * \brief menuActions
     * \param types
     * \param uri
     * \param selectionUris
     * \return
     * In explorer-qt, user can install the menu extensions. This is because
     * explorer-qt's view/menu frameworks is desgined to be extensible.
     * When a explorer view context menu was request it will call the plugin's
     * menuActions method get the extension actions.
     * \note
     * A menu can be more than one type, a desktop background menu is also
     * could be a directory background menu. This is decied by explorer-qt's
     * view/menu framework itself.
     * You can only implement one type of a menu, but you should never directly
     * return the actions without a type check, this might cause actions in menu
     * duplicated.
     */
    virtual QList<QAction *> menuActions(Types types, const QString &uri, const QStringList &selectionUris) = 0;
};

}

Q_DECLARE_OPERATORS_FOR_FLAGS(Peony::MenuPluginInterface::Types)
Q_DECLARE_INTERFACE (Peony::MenuPluginInterface, MenuPluginInterface_iid)

#endif
