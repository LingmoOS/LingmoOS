/* -*- C++ -*-
    This file is part of ThreadWeaver.

    SPDX-FileCopyrightText: 2005-2014 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ImageListFilter.h"

#include "Model.h"

ImageListFilter::ImageListFilter(Image::Steps step, QObject *parent)
    : QSortFilterProxyModel(parent)
    , m_step(step)
{
    setSortRole(Model::Role_SortRole);
    setDynamicSortFilter(true);
    sort(0, Qt::AscendingOrder);
}

bool ImageListFilter::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    const QModelIndex index(sourceModel()->index(source_row, 0, source_parent));
    const int step = index.data(Model::Role_StepRole).value<int>();
    return step == m_step;
}

#include "moc_ImageListFilter.cpp"
