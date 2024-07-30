/*
 * SPDX-FileCopyrightText: 2009 Ben Cooksley <bcooksley@kde.org>
 * SPDX-FileCopyrightText: 2007 Will Stephenson <wstephenson@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "MenuModel.h"

#include "MenuItem.h"

#include <KCategorizedSortFilterProxyModel>

#include <QIcon>

using namespace Qt::StringLiterals;

class MenuModel::Private
{
public:
    Private()
    {
    }

    MenuItem *rootItem = nullptr;
    QList<MenuItem *> exceptions;
};

MenuModel::MenuModel(MenuItem *menuRoot, QObject *parent)
    : QAbstractItemModel(parent)
    , d(new Private())
{
    d->rootItem = menuRoot;
}

MenuModel::~MenuModel()
{
    d->exceptions.clear();
    delete d;
}

QHash<int, QByteArray> MenuModel::roleNames() const
{
    QHash<int, QByteArray> names = QAbstractItemModel::roleNames();
    names[DepthRole] = "depth";
    names[IsCategoryRole] = "isCategory";
    names[IsKCMRole] = "isKCM";
    names[DefaultIndicatorRole] = "showDefaultIndicator";
    names[IconNameRole] = "iconName";
    return names;
}

int MenuModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 1;
}

int MenuModel::rowCount(const QModelIndex &parent) const
{
    MenuItem *mi;
    if (parent.isValid()) {
        mi = static_cast<MenuItem *>(parent.internalPointer());
    } else {
        mi = d->rootItem;
    }
    return childrenList(mi).count();
}

QVariant MenuModel::data(const QModelIndex &index, int role) const
{
    QVariant theData;
    if (!index.isValid()) {
        return {};
    }

    auto mi = static_cast<MenuItem *>(index.internalPointer());
    switch (role) {
    case Qt::DisplayRole:
        theData.setValue(mi->name());
        break;
    case Qt::DecorationRole:
        theData = QVariant(QIcon::fromTheme(mi->iconName()));
        break;
    case KCategorizedSortFilterProxyModel::CategorySortRole:
        if (mi->parent()) {
            theData.setValue(QStringLiteral("%1%2").arg(QString::number(mi->parent()->weight()), 5, QLatin1Char('0')).arg(mi->parent()->name()));
        }
        break;
    case KCategorizedSortFilterProxyModel::CategoryDisplayRole: {
        MenuItem *candidate = mi->parent();
        // The model has an invisible single root item.
        // So to get the "root category" we don't go up all the way
        // To the actual root, but to the list of the first childs.
        // That's why we check for candidate->parent()->parent()
        while (candidate && candidate->parent() && candidate->parent()->parent()) {
            candidate = candidate->parent();
        }
        if (candidate) {
            // Children of this special root category don't have an user visible category
            if (!candidate->isSystemsettingsRootCategory()) {
                theData.setValue(candidate->name());
            }
        }
        break;
    }
    case MenuModel::MenuItemRole:
        theData.setValue(mi);
        break;
    case MenuModel::UserFilterRole:
        // We join by ZERO WIDTH SPACE to avoid awkward word merging in search terms
        // e.g. ['keys', 'slow'] should match 'keys' and 'slow' but not 'ssl'.
        // https://bugs.kde.org/show_bug.cgi?id=487855
        theData.setValue(mi->keywords().join(u"\u200B"_s));
        break;
    case MenuModel::UserSortRole:
        // Category owners are always before everything else, regardless of weight
        if (mi->isCategoryOwner()) {
            theData.setValue(QStringLiteral("%1").arg(QString::number(mi->weight()), 5, QLatin1Char('0')));
        } else {
            theData.setValue(QStringLiteral("1%1").arg(QString::number(mi->weight()), 5, QLatin1Char('0')));
        }
        break;
    case MenuModel::DepthRole: {
        MenuItem *candidate = mi;
        // -1 excludes the invisible root, having main categories at depth 0
        int depth = -1;
        while (candidate && candidate->parent()) {
            candidate = candidate->parent();
            ++depth;
        }

        MenuItem *parent = mi->parent();
        // Items that are in a category with an owner are one level deeper,
        // except the owner
        if (parent && parent->menu() && parent->isLibrary() && !mi->isCategoryOwner()) {
            ++depth;
        }
        theData.setValue(depth);
        break;
    }
    case MenuModel::IsCategoryRole:
        theData.setValue(mi->menu());
        break;
    case MenuModel::IsKCMRole:
        theData.setValue(mi->isLibrary());
        break;
    case MenuModel::DefaultIndicatorRole:
        theData.setValue(mi->showDefaultIndicator());
        break;
    case MenuModel::IconNameRole:
        theData.setValue(mi->iconName());
        break;
    default:
        break;
    }
    return theData;
}

Qt::ItemFlags MenuModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QModelIndex MenuModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent)) {
        return {};
    }

    MenuItem *parentItem;
    if (!parent.isValid()) {
        parentItem = d->rootItem;
    } else {
        parentItem = static_cast<MenuItem *>(parent.internalPointer());
    }

    MenuItem *childItem = childrenList(parentItem).value(row);
    if (childItem) {
        return createIndex(row, column, childItem);
    } else {
        return {};
    }
}

QModelIndex MenuModel::parent(const QModelIndex &index) const
{
    auto childItem = static_cast<MenuItem *>(index.internalPointer());
    if (!childItem) {
        return {};
    }

    MenuItem *parent = parentItem(childItem);
    MenuItem *grandParent = parentItem(parent);

    int childRow = 0;
    if (grandParent) {
        childRow = childrenList(grandParent).indexOf(parent);
    }

    if (parent == d->rootItem) {
        return {};
    }
    return createIndex(childRow, 0, parent);
}

QList<MenuItem *> MenuModel::childrenList(MenuItem *parent) const
{
    QList<MenuItem *> children = parent->children();
    foreach (MenuItem *child, children) {
        if (d->exceptions.contains(child)) {
            children.removeOne(child);
            children.append(child->children());
        }
    }
    return children;
}

MenuItem *MenuModel::parentItem(MenuItem *child) const
{
    MenuItem *parent = child->parent();
    if (d->exceptions.contains(parent)) {
        parent = parentItem(parent);
    }
    return parent;
}

QModelIndex MenuModel::indexForItem(MenuItem *item) const
{
    MenuItem *parent = parentItem(item);

    if (!parent) {
        return {};
    }

    const int row = childrenList(parent).indexOf(item);

    if (row < 0) {
        return {};
    }

    return createIndex(row, 0, item);
}

MenuItem *MenuModel::rootItem() const
{
    return d->rootItem;
}

void MenuModel::addException(MenuItem *exception)
{
    if (exception == d->rootItem) {
        return;
    }
    d->exceptions.append(exception);
}

void MenuModel::removeException(MenuItem *exception)
{
    d->exceptions.removeAll(exception);
}

#include "moc_MenuModel.cpp"
