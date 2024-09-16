// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SYSTEMMONITORDBUSADAPTOR_H
#define SYSTEMMONITORDBUSADAPTOR_H

#include <QtCore/QObject>
#include <QtDBus/QtDBus>

class SystemMonitorDBusAdaptor :  public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.SystemMonitorPluginPopup")

public:
    explicit SystemMonitorDBusAdaptor(QObject *parent = nullptr);
    virtual ~SystemMonitorDBusAdaptor();

public:
public slots:
    Q_SCRIPTABLE void slotShowOrHideSystemMonitorPluginPopupWidget();

signals:
    Q_SCRIPTABLE void sigSendShowOrHideSystemMonitorPluginPopupWidget();
    //!
    //! \brief sysMonPopVisibleChanged 系统监视器弹窗显示状态改变
    //!
    Q_SCRIPTABLE void sysMonPopVisibleChanged(bool);
};

#endif // SYSTEMMONITORDBUSADAPTOR_H
