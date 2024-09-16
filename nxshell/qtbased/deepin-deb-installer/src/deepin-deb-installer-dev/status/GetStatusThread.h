// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GETSTATUSTHREAD_H
#define GETSTATUSTHREAD_H

#include "PackageStatus.h"

#include <QObject>
#include <QThread>

class Package;
class GetStatusThread : public QThread
{
    Q_OBJECT
public:
    explicit GetStatusThread(PackageStatus *);

    void run();

    void setPackage(int index, const QString &packagePath);

signals:

    void signal_dependsStatus(int, DependsStatus);
    void signal_installStatus(int, InstallStatus);

private:
    int m_index = -1;
    PackageStatus *m_pPackageStatus = nullptr;
    QString m_packagePath = "";
};

#endif  // GETSTATUSTHREAD_H
