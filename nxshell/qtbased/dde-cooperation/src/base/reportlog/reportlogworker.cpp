// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "reportlogworker.h"
#include "datas/cooperationreportdata.h"
#include "datas/reportdatainterface.h"

#include <QDebug>
#include <QApplication>
#include <QJsonDocument>

using namespace deepin_cross;

ReportLogWorker::ReportLogWorker(QObject *parent)
    : QObject(parent)
{
}

ReportLogWorker::~ReportLogWorker()
{
    qDeleteAll(logDataObj.begin(), logDataObj.end());
    logDataObj.clear();

    if (logLibrary.isLoaded())
        logLibrary.unload();
}

bool ReportLogWorker::init()
{
    QList<ReportDataInterface *> datas {
        new StatusReportData,
        new FileDeliveryReportData,
        new ConnectionReportData
    };

    std::for_each(datas.cbegin(), datas.cend(), [this](ReportDataInterface *dat) { registerLogData(dat->type(), dat); });

    logLibrary.setFileName("deepin-event-log");
    if (!logLibrary.load()) {
        qWarning() << "Report log load log library failed!";
        return false;
    } else {
        qInfo() << "Report log load log library success.";
    }

    initEventLogFunc = reinterpret_cast<InitEventLog>(logLibrary.resolve("Initialize"));
    writeEventLogFunc = reinterpret_cast<WriteEventLog>(logLibrary.resolve("WriteEventLog"));

    if (!initEventLogFunc || !writeEventLogFunc) {
        qWarning() << "Log library init failed!";
        return false;
    }

    if (!initEventLogFunc(QApplication::applicationName().toStdString(), false)) {
        qWarning() << "Log library init function call failed!";
        return false;
    }

    return true;
}

void ReportLogWorker::commitLog(const QString &type, const QVariantMap &args)
{
    ReportDataInterface *interface = logDataObj.value(type, nullptr);
    if (!interface) {
        qInfo() << "Error: Log data object is not registed.";
        return;
    }
    QJsonObject jsonObject = interface->prepareData(args);

    const QStringList &keys = commonData.keys();
    foreach (const QString &key, keys) {
        jsonObject.insert(key, commonData.value(key));   //add common data for each log commit
    }

    commit(jsonObject.toVariantHash());
}

bool ReportLogWorker::registerLogData(const QString &type, ReportDataInterface *dataObj)
{
    if (logDataObj.contains(type))
        return false;

    logDataObj.insert(type, dataObj);
    return true;
}

void ReportLogWorker::commit(const QVariant &args)
{
    if (args.isNull() || !args.isValid())
        return;
    const QJsonObject &dataObj = QJsonObject::fromVariantHash(args.toHash());
    QJsonDocument doc(dataObj);
    const QByteArray &sendData = doc.toJson(QJsonDocument::Compact);
    writeEventLogFunc(sendData.data());
}
