// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COOPERATIONCOREPLUGIN_H
#define COOPERATIONCOREPLUGIN_H

#include "common/exportglobal.h"
#include "gui/mainwindow.h"

#include <QObject>
#include <QSharedPointer>

namespace cooperation_core {

class EXPORT_API CooperaionCorePlugin : public QObject
{
    Q_OBJECT

public:
    explicit CooperaionCorePlugin(QObject *parent = nullptr);
    ~CooperaionCorePlugin();

    bool start();
    void stop();

private:
    void initialize();
    bool isMinilize();
#ifdef ENABLE_PHONE
    void initMobileModule();
#endif
    QSharedPointer<MainWindow> dMain { nullptr };
    bool onlyTransfer { false };
};

}   // namespace cooperation_core

#endif   // COOPERATIONCOREPLUGIN_H
