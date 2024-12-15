// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LINGMOIDWORKER_H
#define LINGMOIDWORKER_H

#include "lingmoidmodel.h"
#include "lingmoiddbusproxy.h"
#include "syncdbusproxy.h"

class LingmoWorker : public QObject
{
    Q_OBJECT
public:
    explicit LingmoWorker(LingmoidModel *model, QObject *parent = nullptr);

    void initData();


public Q_SLOTS:
    void loginUser();
    void logoutUser();
    void openWeb();

    void licenseStateChangeSlot();

private:
    QString loadCodeURL();
    void getLicenseState();

private:
    LingmoidModel *m_model;
    LingmoidDBusProxy *m_lingmoIDProxy;
    SyncDBusProxy *m_syncProxy;
    QDBusInterface *m_utcloudInter;
};

#endif // LINGMOIDWORKER_H
