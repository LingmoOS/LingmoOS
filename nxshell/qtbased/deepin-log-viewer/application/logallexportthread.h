// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOGALLEXPORTTHREAD_H
#define LOGALLEXPORTTHREAD_H

#include "structdef.h"

#include <QObject>
#include <QRunnable>

class LogAllExportThread : public QObject
    , public QRunnable
{
    Q_OBJECT
public:
    explicit LogAllExportThread(const QStringList &types, const QString &outfile, QObject *parent = nullptr);
public slots:
    void slot_cancelExport() { m_cancel = true; }

protected:
    void run() override;
signals:
    void updatecurrentProcess(int current);
    void updateTolProcess(int tol);
    void exportFinsh(bool success = true);

private:
    QStringList m_types;
    QString m_outfile {""};

    bool m_cancel {false};
};

#endif // LOGALLEXPORTTHREAD_H
