// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "applet.h"
#include "dsglobal.h"

#include <QAbstractListModel>

DS_USE_NAMESPACE

namespace apps {
class AMAppItemModel;
class AppItem;
class AppsApplet : public DApplet
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel *appModel READ appModel CONSTANT FINAL)
    Q_PROPERTY(QAbstractItemModel *appGroupModel READ groupModel CONSTANT FINAL)

public:
    explicit AppsApplet(QObject *parent = nullptr);
    ~AppsApplet();

    bool load() override;

    QAbstractItemModel *appModel() const;
    QAbstractItemModel *groupModel() const;

private:
    AMAppItemModel *m_appModel;
    QAbstractItemModel *m_groupModel;
};
}
