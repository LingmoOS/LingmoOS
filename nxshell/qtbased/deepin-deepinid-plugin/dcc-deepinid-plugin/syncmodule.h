// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "interface/moduleobject.h"

#include "operation/syncmodel.h"
#include "operation/syncworker.h"

#include <QObject>

namespace dcc {
namespace cloudsync {
class SyncWorker;
class SyncModel;
}
}

using namespace DCC_NAMESPACE;

class SyncModule : public ModuleObject
{
    Q_OBJECT
public:
    explicit SyncModule(QObject *parent = nullptr);
    ~SyncModule() Q_DECL_OVERRIDE;

    //active module
    void active() Q_DECL_OVERRIDE;
    //deactive module
    void deactive() override;
    //always construct new page
    QWidget *page() override;
private:
    void InitModuleData();
private:
    SyncModel  *m_model;
    SyncWorker *m_worker;
};
