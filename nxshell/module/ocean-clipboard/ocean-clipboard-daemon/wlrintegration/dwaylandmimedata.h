// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QMimeData>

class DWaylandMimeData : public QMimeData
{
    Q_OBJECT
public:
    DWaylandMimeData();
    ~DWaylandMimeData() override;
    QStringList formats() const override;
    QVariant retrieveData(const QString &mimeType, QMetaType preferredType) const override;
};
