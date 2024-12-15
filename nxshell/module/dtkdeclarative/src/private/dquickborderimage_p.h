// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DQUICKBORDERIMAGE_P_H
#define DQUICKBORDERIMAGE_P_H

#include <dtkdeclarative_global.h>

#include <private/qquickborderimage_p.h>

DQUICK_BEGIN_NAMESPACE

class DQuickBorderImage : public QQuickBorderImage
{
    Q_OBJECT
    QML_NAMED_ELEMENT(DBorderImage)
public:
    explicit DQuickBorderImage(QQuickItem *parent=nullptr);

protected:
    void itemChange(ItemChange change, const ItemChangeData &value) override;
};

DQUICK_END_NAMESPACE

#endif // DQUICKBORDERIMAGE_P_H
