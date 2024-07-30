/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef MODELSOURCE_H
#define MODELSOURCE_H

#include <QAbstractItemModel>
#include <QPointer>

#include "ChartDataSource.h"

/**
 * A data source that reads data from a QAbstractItemModel.
 *
 *
 */
class QUICKCHARTS_EXPORT ModelSource : public ChartDataSource
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit ModelSource(QObject *parent = nullptr);

    Q_PROPERTY(int role READ role WRITE setRole NOTIFY roleChanged)
    int role() const;
    void setRole(int role);
    Q_SIGNAL void roleChanged();

    Q_PROPERTY(QString roleName READ roleName WRITE setRoleName NOTIFY roleNameChanged)
    QString roleName() const;
    void setRoleName(const QString &name);
    Q_SIGNAL void roleNameChanged();

    Q_PROPERTY(int column READ column WRITE setColumn NOTIFY columnChanged)
    int column() const;
    void setColumn(int column);
    Q_SIGNAL void columnChanged();

    Q_PROPERTY(QAbstractItemModel *model READ model WRITE setModel NOTIFY modelChanged)
    QAbstractItemModel *model() const;
    void setModel(QAbstractItemModel *model);
    Q_SIGNAL void modelChanged();

    Q_PROPERTY(bool indexColumns READ indexColumns WRITE setIndexColumns NOTIFY indexColumnsChanged)
    bool indexColumns() const;
    void setIndexColumns(bool index);
    Q_SIGNAL void indexColumnsChanged();

    int itemCount() const override;
    QVariant item(int index) const override;
    QVariant minimum() const override;
    QVariant maximum() const override;

private:
    Q_SLOT void onMinimumChanged();
    Q_SLOT void onMaximumChanged();

    mutable int m_role = -1;
    QString m_roleName;
    int m_column = 0;
    bool m_indexColumns = false;
    QAbstractItemModel *m_model = nullptr;

    QVariant m_minimum;
    QVariant m_maximum;
};

#endif // MODELSOURCE_H
