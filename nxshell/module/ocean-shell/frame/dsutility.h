// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "dsglobal.h"

#include <DObject>
#include <QObject>
#include <QWindow>

DS_BEGIN_NAMESPACE

class UtilityPrivate;
class DS_SHARE Utility : public QObject, public DTK_CORE_NAMESPACE::DObject
{
    Q_OBJECT
    D_DECLARE_PRIVATE(Utility)
public:
    static Utility *instance();
    virtual bool grabKeyboard(QWindow *target, bool grab = true);
    virtual bool grabMouse(QWindow *target, bool grab = true);

    QList<QWindow *> allChildrenWindows(QWindow *target);
protected:
    explicit Utility(QObject *parent = nullptr);
};

DS_END_NAMESPACE
