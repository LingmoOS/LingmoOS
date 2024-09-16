// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "applet.h"
#include "workspacemodel.h"
#include "dsglobal.h"

namespace dock {

class WorkspaceItem : public DS_NAMESPACE::DApplet
{
    Q_OBJECT
    Q_PROPERTY(WorkspaceModel* dataModel READ dataModel NOTIFY modelChanged FINAL)
public:
    explicit WorkspaceItem(QObject *parent = nullptr);

    WorkspaceModel *dataModel() const;
Q_SIGNALS:
    void modelChanged();
private:
    WorkspaceModel *m_dataModel = nullptr;
};

}
