/*
    SPDX-FileCopyrightText: 2020 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kdescendantsproxymodel_qml.h"
#include <QDebug>

KDescendantsProxyModelQml::KDescendantsProxyModelQml(QObject *parent)
    : KDescendantsProxyModel(parent)
{
}

KDescendantsProxyModelQml::~KDescendantsProxyModelQml()
{
}

void KDescendantsProxyModelQml::expandChildren(int row)
{
    QModelIndex idx = mapToSource(index(row, 0));
    expandSourceIndex(idx);
}

void KDescendantsProxyModelQml::collapseChildren(int row)
{
    QModelIndex idx = mapToSource(index(row, 0));
    collapseSourceIndex(idx);
}

void KDescendantsProxyModelQml::toggleChildren(int row)
{
    QModelIndex sourceIndex = mapToSource(index(row, 0));

    if (!sourceModel()->hasChildren(sourceIndex)) {
        return;
    }

    if (isSourceIndexExpanded(sourceIndex)) {
        collapseSourceIndex(sourceIndex);
    } else {
        expandSourceIndex(sourceIndex);
    }
}

#include "moc_kdescendantsproxymodel_qml.cpp"
