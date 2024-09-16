// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DRIVERBACKUPTHREAD_H
#define DRIVERBACKUPTHREAD_H

#include "MacroDefinition.h"

#include <QThread>


class DriverBackupThread : public QThread
{
    Q_OBJECT
public:
    enum BackupStatus {
        Waiting = 0,
        Failed,
        Success
    };

    explicit DriverBackupThread(QObject *parent = nullptr);

    void run();

    /**
     * @brief setBackupDriverInfo 设置需要备份的驱动信息
     */
    void setBackupDriverInfo(DriverInfo *info);

    /**
     * @brief undoBackup 取消备份
     */
    void undoBackup();
    void setStatus(BackupStatus status);

signals:
    void backupProgressChanged(int progress);
    void backupProgressFinished(bool bsuccess);

public slots:

private:
    DriverInfo *mp_driverInfo;
    bool m_isStop = true;
    BackupStatus m_status = Waiting;
};

#endif // DRIVERBACKUPTHREAD_H
