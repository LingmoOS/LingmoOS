/*
 * SPDX-FileCopyrightText: 2021 David Redondo <kde@david-redondo.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#pragma once

#include <KRearrangeColumnsProxyModel>

#include <qqmlregistration.h>

/**
 * A proxy model to reverse the columns of a table. Needed because QML Tableview does not correctly
 * reverse its columns when using a RTL language.
 */
class ReverseColumnsProxyModel : public KRearrangeColumnsProxyModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit ReverseColumnsProxyModel(QObject *parent = nullptr);
    void setSourceModel(QAbstractItemModel *sourceModel) override;

private:
    void reverseColumns();
};
