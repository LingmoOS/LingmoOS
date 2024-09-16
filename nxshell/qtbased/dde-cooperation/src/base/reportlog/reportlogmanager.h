// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef REPORTLOGMANAGER_H
#define REPORTLOGMANAGER_H

#include <QObject>

namespace deepin_cross {
class ReportLogWorker;
class ReportLogManager : public QObject
{
    Q_OBJECT
public:
    static ReportLogManager *instance();
    void init();

    void commit(const QString &type, const QVariantMap &args);

Q_SIGNALS:
    void requestCommitLog(const QString &type, const QVariantMap &args);

private:
    explicit ReportLogManager(QObject *parent = nullptr);
    ~ReportLogManager();

    void initConnection();

    QThread *reportWorkThread { nullptr };
    ReportLogWorker *reportWorker { nullptr };
};
}
#endif // REPORTLOGMANAGER_H
