/*
    This file is part of the KDE Libraries
    SPDX-FileCopyrightText: 2006 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kpagemodel.h"
#include "kpagemodel_p.h"

KPageModelPrivate::~KPageModelPrivate()
{
}

KPageModel::KPageModel(QObject *parent)
    : QAbstractItemModel(parent)
    , d_ptr(nullptr)
{
}

KPageModel::KPageModel(KPageModelPrivate &dd, QObject *parent)
    : QAbstractItemModel(parent)
    , d_ptr(&dd)
{
    d_ptr->q_ptr = this;
}

KPageModel::~KPageModel() = default;

#include "moc_kpagemodel.cpp"
