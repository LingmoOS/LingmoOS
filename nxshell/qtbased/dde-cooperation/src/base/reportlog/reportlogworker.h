// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef REPORTLOGWORKER_H
#define REPORTLOGWORKER_H

#include "datas/reportdatainterface.h"

#include <QLibrary>
#include <QJsonObject>

namespace deepin_cross {

class ReportLogWorker : public QObject
{
    Q_OBJECT
public:
    using InitEventLog = bool (*)(const std::string &, bool);
    using WriteEventLog = void (*)(const std::string &);

    explicit ReportLogWorker(QObject *parent = nullptr);
    ~ReportLogWorker();

    bool init();

public Q_SLOTS:
    void commitLog(const QString &type, const QVariantMap &args);

private:
    bool registerLogData(const QString &type, ReportDataInterface *dataObj);
    void commit(const QVariant &args);

    QLibrary logLibrary;
    InitEventLog initEventLogFunc = nullptr;
    WriteEventLog writeEventLogFunc = nullptr;

    QJsonObject commonData;
    QHash<QString, ReportDataInterface *> logDataObj;
};
}

#endif // REPORTLOGWORKER_H
