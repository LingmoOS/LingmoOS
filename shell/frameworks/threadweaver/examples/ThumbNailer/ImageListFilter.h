/* -*- C++ -*-
    This file is part of ThreadWeaver.

    SPDX-FileCopyrightText: 2005-2014 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef IMAGELISTFILTER_H
#define IMAGELISTFILTER_H

#include "Image.h"
#include <QSortFilterProxyModel>

class ImageListFilter : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit ImageListFilter(Image::Steps step, QObject *parent = nullptr);
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    Image::Steps m_step;
};
#endif // IMAGELISTFILTER_H
