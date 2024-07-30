/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2013 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCOLORSCHEMEMODEL_H
#define KCOLORSCHEMEMODEL_H

#include "kcolorscheme_export.h"

#include <QAbstractListModel>
#include <QObject>
#include <memory>

class QModelIndex;

struct KColorSchemeModelPrivate;

/**
 * A model listing the KColorSchemes available in the system.
 *
 * @since 5.84
 */
class KColorSchemeModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        NameRole = Qt::DisplayRole,
        IconRole = Qt::DecorationRole,
        PathRole = Qt::UserRole,
        IdRole,
    };

    explicit KColorSchemeModel(QObject *parent = nullptr);
    ~KColorSchemeModel() override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

private:
    std::unique_ptr<KColorSchemeModelPrivate> d;
};

#endif
