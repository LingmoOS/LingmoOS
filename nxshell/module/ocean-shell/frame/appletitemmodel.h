// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "dsglobal.h"

#include <QAbstractListModel>

DS_BEGIN_NAMESPACE

class DAppletItemModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum {
        Data = Qt::UserRole + 1
    } AppletItemRules;

    explicit DAppletItemModel(QObject *parent = nullptr);
    ~DAppletItemModel() override;

    QList<QObject *> rootObjects() const;
    void append(QObject *rootObject);
    void remove(QObject *rootObject);

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    QList<QObject *> m_rootObjects;

};

DS_END_NAMESPACE
