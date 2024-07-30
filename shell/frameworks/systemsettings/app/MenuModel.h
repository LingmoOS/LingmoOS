/*
 * SPDX-FileCopyrightText: 2009 Ben Cooksley <bcooksley@kde.org>
 * SPDX-FileCopyrightText: 2007 Will Stephenson <wstephenson@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef MENUMODEL_H
#define MENUMODEL_H

#include <QAbstractItemModel>

class MenuItem;

/**
 * @brief Provides a menu of the MenuItem objects
 *
 * Provides a standardised model to be used with views to display the list of modules in a variety of ways.\n
 * It is recommended to also use this with the MenuProxyModel to provide searching
 * and correct ordering of modules.
 *
 * @author Ben Cooksley <bcooksley@kde.org>
 * @author Will Stephenson <wstephenson@kde.org>
 */
class MenuModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum Roles {
        MenuItemRole = Qt::UserRole,

        /**
         * Role used to request the keywords to filter the items when searching.
         */
        UserFilterRole,

        /**
         * Role used to request the weight of a module, used to sort the items.
         */
        UserSortRole,

        DepthRole,

        IsCategoryRole,

        IsKCMRole,

        DefaultIndicatorRole,

        IconNameRole,
    };

    /**
     * Creates a MenuModel using the MenuItem specified. The MenuItem must always be valid
     * throughout the life of the MenuModel, otherwise it will cause crashes.
     *
     * @param menuRoot The MenuItem to use as the basis for providing information.
     * @param parent The QObject to use as a parent of the MenuModel.
     */
    explicit MenuModel(MenuItem *menuRoot, QObject *parent = nullptr);

    /**
     * Destroys the MenuModel. The menuRoot will not be destroyed.
     */
    ~MenuModel() override;

    QHash<int, QByteArray> roleNames() const override;

    /**
     * Please see Qt QAbstractItemModel documentation for more details.\n
     * Provides the name, tooltip, icon, category, keywords and the internal MenuItem to views.
     *
     * @param index The QModelIndex you want information about.
     * @param role The information role you want information about.
     * @returns The data requested for the role provided from the QModelIndex provided.
     */
    QVariant data(const QModelIndex &index, int role) const override;

    /**
     * Please see Qt QAbstractItemModel documentation for more details.\n
     * Provides the status flags for the QModelIndex specified.
     * The item always has selectable and enabled for its status unless invalid.
     *
     * @returns The flags for the QModelIndex provided.
     */
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    /**
     * Please see Qt QAbstractItemModel documentation for more details.\n
     * Provides a QModelIndex at the row and column specified who belongs to the parent specified.
     *
     * @param row Vertical position in the grid of children.
     * @param column Horizontal position in the grid of children.
     * @param parent The parent of the requested child.
     * @returns The QModelIndex for the item requested.
     */
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

    /**
     * Please see Qt QAbstractItemModel documentation for more details.\n
     * Provides the parent QModelIndex for the child specified.
     *
     * @param index The child of the parent.
     * @returns A QModelIndex for the parent.
     */
    QModelIndex parent(const QModelIndex &index) const override;

    /**
     * Please see Qt QAbstractItemModel documentation for more details.\n
     * Provides the number of MenuItems ( categories or modules ) that the specified parent has.
     *
     * @param parent The QModelIndex the count is performed on.
     * @returns The number of rows ( children ) in the parent.
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * Please see Qt QAbstractItemModel documentation for more details.\n
     * Returns 1, the number of columns of information about a row.
     *
     * @param parent This is ignored, as the count is always 1.
     * @returns The number of columns ( 1 ) in the parent.
     */
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * Makes the MenuItem specified be hidden from the list, while still showing its children.\n
     * Children of exceptions consider their grand parents as their parent.
     * Parents of exceptions consider the exceptions children as theirs.
     *
     * @param exception The MenuItem to give an exception to.
     */
    void addException(MenuItem *exception);

    /**
     * Revokes the exception granted above. After this, the MenuItem's parents will return their children
     * as normal and the grand children will return their true parents, restoring normal operation.
     * It has no effect if the MenuItem does not have an exception.
     *
     * @param exception The MenuItem to revoke an exception from.
     */
    void removeException(MenuItem *exception);

    QModelIndex indexForItem(MenuItem *item) const;

protected:
    /**
     * Provides the MenuItem which is used internally to provide information.
     *
     * @returns The MenuItem used internally.
     */
    MenuItem *rootItem() const;

    /**
     * Provides a list of children of an item which has been altered by the exceptions list
     *
     * @param parent The parent of the children desired
     * @returns The list of children for the item specified
     */
    QList<MenuItem *> childrenList(MenuItem *parent) const;

    /**
     * Provides the parent of the child specified altered by the exceptions list
     *
     * @param child The child of the parent
     * @returns The exceptions list affected parent of the child
     */
    MenuItem *parentItem(MenuItem *child) const;

private:
    class Private;
    Private *const d;
};

#endif
