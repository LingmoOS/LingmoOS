// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "appitemmodel.h"
#include "objectmanager1interface.h"

namespace apps
{
class AMAppItem;
class AMAppItemModel : public AppItemModel
{
    Q_OBJECT

public:
    explicit AMAppItemModel(QObject *parent = nullptr);

    AMAppItem * appItem(const QString &id);

private:
    ObjectManager *m_manager;
};
}