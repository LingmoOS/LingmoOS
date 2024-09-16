// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONNECTEDTASK_H
#define CONNECTEDTASK_H

#include "ztroubleshoot_p.h"

#include <QThread>

class ConnectedTask : public QThread
{
    Q_OBJECT
public:
    explicit ConnectedTask(const QString &name, QObject *parent = nullptr);
    virtual ~ConnectedTask() override;

    void setSuspended(bool suspended);

protected:
    void run() override;

private:
    CheckConnected *m_pCheckCon;
    QString m_printerName;

signals:
    void signalResult(bool connected, const QString &printerName);
};
#endif // CONNECTEDTASK_H
