// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logviewerplugin.h"
#include "../application/logexportthread.h"
#include "../application/logapplicationhelper.h"

#include <DApplication>

#include <qdatetime.h>

LogViewerPlugin::LogViewerPlugin()
{
    initConnections();
}

LogViewerPlugin::~LogViewerPlugin()
{

}

void LogViewerPlugin::clearAllFilter()
{
    m_bootFilter = {"", ""};
    m_currentSearchStr.clear();
    m_currentKwinFilter = {""};
    m_normalFilter.searchstr = "";
}

/**
 * @brief LogViewerPlugin::clearAllDatalist 清空所有获取的数据list
 */
void LogViewerPlugin::clearAllDatalist()
{
    //m_detailWgt->cleanText();
    //m_pModel->clear();
    jList.clear();
    jListOrigin.clear();
    dList.clear();
    dListOrigin.clear();
    xList.clear();
    xListOrigin.clear();
    bList.clear();
    currentBootList.clear();
    kList.clear();
    kListOrigin.clear();
    appList.clear();
    appListOrigin.clear();
    norList.clear();
    nortempList.clear();
    m_currentKwinList.clear();
    m_kwinList.clear();
    jBootList.clear();
    jBootListOrigin.clear();
    dnfList.clear();
    dnfListOrigin.clear();
    //malloc_trim(0);
}

/**
 * @brief DisplayContent::initConnections 初始化槽函数信号连接
 */
void LogViewerPlugin::initConnections()
{
    connect(&m_logFileParse, &LogFileParser::appFinished, this,
            &LogViewerPlugin::slot_appFinished);
    connect(&m_logFileParse, &LogFileParser::appData, this,
            &LogViewerPlugin::slot_appData);
}

//generateAppFile 获取系统应用日志的数据
//param path 要获取的某一个应用的日志的日志文件路径
//param id 时间筛选id 对应BUTTONID枚举,0表示全部,1是今天,2是3天内,3是筛选1周内数据,4是筛选一个月内的,5是三个月
//param lId 等级筛选id,对应PRIORITY枚举,直接传入获取系统接口,-1表示全部等级不筛选,
//param iSearchStr 搜索关键字,现阶段不用,保留参数
void LogViewerPlugin::generateAppFile(const QString &path, BUTTONID id, PRIORITY lId, const QString &iSearchStr)
{
    Q_UNUSED(iSearchStr)
    appList.clear();
    appListOrigin.clear();
    clearAllFilter();
    clearAllDatalist();
    //setLoadState(DATA_LOADING);
    m_firstLoadPageData = true;
    m_isDataLoadComplete = false;
    QDateTime dt = QDateTime::currentDateTime();
    dt.setTime(QTime()); // get zero time
    //createAppTableForm();
    APP_FILTERS appFilter;
    appFilter.path = path;
    appFilter.lvlFilter = lId;
    switch (id) {
    case ALL:
        m_appCurrentIndex = m_logFileParse.parseByApp(appFilter);
        break;
    case ONE_DAY: {
        QDateTime dtStart = dt;
        QDateTime dtEnd = dt;
        dtEnd.setTime(QTime(23, 59, 59, 999));
        appFilter.timeFilterBegin = dtStart.toMSecsSinceEpoch();
        appFilter.timeFilterEnd = dtEnd.toMSecsSinceEpoch();
        m_appCurrentIndex = m_logFileParse.parseByApp(appFilter);
    } break;
    case THREE_DAYS: {
        QDateTime dtStart = dt;
        QDateTime dtEnd = dt;
        dtEnd.setTime(QTime(23, 59, 59, 999));
        appFilter.timeFilterBegin = dtStart.addDays(-2).toMSecsSinceEpoch();
        appFilter.timeFilterEnd = dtEnd.toMSecsSinceEpoch();
        m_appCurrentIndex = m_logFileParse.parseByApp(appFilter);

    } break;
    case ONE_WEEK: {
        QDateTime dtStart = dt;
        QDateTime dtEnd = dt;
        dtEnd.setTime(QTime(23, 59, 59, 999));
        appFilter.timeFilterBegin = dtStart.addDays(-6).toMSecsSinceEpoch();
        appFilter.timeFilterEnd = dtEnd.toMSecsSinceEpoch();
        m_appCurrentIndex = m_logFileParse.parseByApp(appFilter);
    } break;
    case ONE_MONTH: {
        QDateTime dtStart = dt;
        QDateTime dtEnd = dt;
        dtEnd.setTime(QTime(23, 59, 59, 999));
        appFilter.timeFilterBegin = dtStart.addMonths(-1).toMSecsSinceEpoch();
        appFilter.timeFilterEnd = dtEnd.toMSecsSinceEpoch();
        m_appCurrentIndex = m_logFileParse.parseByApp(appFilter);
    } break;
    case THREE_MONTHS: {
        QDateTime dtStart = dt;
        QDateTime dtEnd = dt;
        dtEnd.setTime(QTime(23, 59, 59, 999));
        appFilter.timeFilterBegin = dtStart.addMonths(-3).toMSecsSinceEpoch();
        appFilter.timeFilterEnd = dtEnd.toMSecsSinceEpoch();
        m_appCurrentIndex = m_logFileParse.parseByApp(appFilter);
    } break;
    default:
        break;
    }
}

// exportAppLogFile 导出应用日志
// param: path 导出路径
// param: period 周期，枚举类型，具体值对应周期下拉框选项
// param: Level 级别，级别，具体值对应级别下拉框选项
// param: appid 自研应用id，用于导出指定应用的日志，若为空，什么都不做，返回false（例：导出相册的日志信息，传入deepin-album即可）
// return：true 导出成功， false 导出失败
bool LogViewerPlugin::exportAppLogFile(const QString &path, BUTTONID period, PRIORITY level, const QString &appid)
{
    m_curAppLog = LogApplicationHelper::instance()->getPathByAppId(appid);
    m_flag = APP;
    if (m_curAppLog.isEmpty()) {
        return false;
    }
    generateAppFile(m_curAppLog, period, level);
    connect(this, &LogViewerPlugin::sigAppFinished, this, [=](int index) {
        Q_UNUSED(index);
        exportLogFile(path);
    });

    return true;
}

void LogViewerPlugin::slot_appFinished(int index)
{
    if (m_flag != APP || index != m_appCurrentIndex)
        return;
    m_isDataLoadComplete = true;
    if (appList.isEmpty()) {
     //   setLoadState(DATA_COMPLETE);
     //   createAppTable(appList);
    }

    emit sigAppFinished(index);
}

void LogViewerPlugin::slot_appData(int index, QList<LOG_MSG_APPLICATOIN> list)
{
    if (m_flag != APP || index != m_appCurrentIndex)
        return;
    appListOrigin.append(list);
    QList<LOG_MSG_APPLICATOIN> listFiltered = filterApp(m_currentSearchStr, list);
    appList.append(listFiltered);
    //因为此槽会在同一次加载数据完成前触发数次,所以第一次收到数据需要更新界面状态,后面的话往model里塞数据就行
    if (m_firstLoadPageData) {
    //    createAppTable(appList);
        m_firstLoadPageData = false;
    }

    emit sigAppData(index, listFiltered);
}

QList<LOG_MSG_APPLICATOIN> LogViewerPlugin::filterApp(const QString &iSearchStr, const QList<LOG_MSG_APPLICATOIN> &iList)
{
    QList<LOG_MSG_APPLICATOIN> rsList;
    if (iSearchStr.isEmpty()) {
        return iList;
    }
    for (int i = 0; i < iList.size(); i++) {
        LOG_MSG_APPLICATOIN msg = iList.at(i);
        if (msg.dateTime.contains(iSearchStr, Qt::CaseInsensitive) || msg.level.contains(iSearchStr, Qt::CaseInsensitive) || msg.src.contains(iSearchStr, Qt::CaseInsensitive) || msg.msg.contains(iSearchStr, Qt::CaseInsensitive))
            rsList.append(msg);
    }
    return rsList;
}

/**
 * @brief DisplayContent::getAppName 获取当前选择的应用的日志路径对应的日志名称
 * @param filePath  当前选择的应用的日志路径
 * @return 对应的日志名称
 */
QString LogViewerPlugin::getAppName(const QString &filePath)
{
    QString ret;
    if (filePath.isEmpty())
        return ret;

    QStringList strList = filePath.split("/");
    if (strList.count() < 2) {
        if (filePath.contains("."))
            ret = filePath.section(".", 0, 0);
        else {
            ret = filePath;
        }
        return LogApplicationHelper::instance()->transName(ret);
    }

    QString desStr = filePath.section("/", -1);
    ret = desStr.mid(0, desStr.lastIndexOf("."));
    return LogApplicationHelper::instance()->transName(ret);
}

void LogViewerPlugin::exportLogFile(const QString &path)
{
    LogExportThread *exportThread = new LogExportThread(this);
    connect(exportThread, &LogExportThread::sigResult, this, &LogViewerPlugin::slot_exportResult);
//    connect(m_exportDlg, &ExportProgressDlg::sigCloseBtnClicked, exportThread, &LogExportThread::stopImmediately);
//    connect(m_exportDlg, &ExportProgressDlg::buttonClicked, exportThread, &LogExportThread::stopImmediately);
//    connect(exportThread, &LogExportThread::sigResult, this, &DisplayContent::onExportResult);
//    connect(exportThread, &LogExportThread::sigProgress, this, &DisplayContent::onExportProgress);
//    connect(exportThread, &LogExportThread::sigProcessFull, this, &DisplayContent::onExportFakeCloseDlg);

//    QString logName;
//    if (m_curListIdx.isValid())
//        logName = QString("/%1").arg(m_curListIdx.data().toString());
//    else {
//        logName = QString("/%1").arg(("New File"));
//    }

    //QString path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + logName + ".txt";
//    QString fileName = DFileDialog::getSaveFileName(
//        this, DApplication::translate("File", "Export File"),
//        path,
//        tr("TEXT (*.txt);; Doc (*.doc);; Xls (*.xls);; Html (*.html)"), &selectFilter);

    QString fileName = path;
    //限制当导出文件为空和导出doc和xls时用户改动后缀名导致导出问题，提示导出失败
    QFileInfo exportFile(fileName);
    selectFilter = exportFile.suffix();
    selectFilter = "(*." + selectFilter + ")";
    //QString exportSuffix = exportFile.suffix();
    //QString selectSuffix = selectFilter.mid(selectFilter.lastIndexOf(".") + 1, selectFilter.size() - selectFilter.lastIndexOf(".") - 2);
    //if (fileName.isEmpty())
    //    return;

    //用户修改后缀名后添加默认的后缀
//    if (selectSuffix != exportSuffix) {
//        fileName.append(".").append(selectSuffix);
//    }

    //m_exportDlg->show();
    QStringList labels;
//    for (int col = 0; col < m_pModel->columnCount(); ++col) {
//        labels.append(m_pModel->horizontalHeaderItem(col)->text());
//    }

    //暂时写死，只支持应用日志
    labels << DApplication::translate("Table", "Level")
           << DApplication::translate("Table", "Date and Time")
           << DApplication::translate("Table", "Source")
           << DApplication::translate("Table", "Info");

    //根据导出格式判断执行逻辑
    if (selectFilter.contains("(*.txt)")) {
        switch (m_flag) {
        //根据导出日志类型执行正确的导出逻辑
        case JOURNAL:
            //PERF_PRINT_BEGIN("POINT-04", QString("format=txt count=%1").arg(jList.count()));
            exportThread->exportToTxtPublic(fileName, jList, labels, m_flag);
            break;
        case BOOT_KLU:
            //PERF_PRINT_BEGIN("POINT-04", QString("format=txt count=%1").arg(jBootList.count()));
            exportThread->exportToTxtPublic(fileName, jBootList, labels, JOURNAL);
            break;
        case APP: {
            //PERF_PRINT_BEGIN("POINT-04", QString("format=txt count=%1").arg(appList.count()));
            QString appName = getAppName(m_curAppLog);
            exportThread->exportToTxtPublic(fileName, appList, labels, appName);
            break;
        }
        case DPKG:
            //PERF_PRINT_BEGIN("POINT-04", QString("format=txt count=%1").arg(dList.count()));
            exportThread->exportToTxtPublic(fileName, dList, labels);
            break;
        case BOOT:
            //PERF_PRINT_BEGIN("POINT-04", QString("format=txt count=%1").arg(currentBootList.count()));
            exportThread->exportToTxtPublic(fileName, currentBootList, labels);
            break;
        case XORG:
            //PERF_PRINT_BEGIN("POINT-04", QString("format=txt count=%1").arg(xList.count()));
            exportThread->exportToTxtPublic(fileName, xList, labels);
            break;
        case Normal:
            //PERF_PRINT_BEGIN("POINT-04", QString("format=txt count=%1").arg(nortempList.count()));
            exportThread->exportToTxtPublic(fileName, nortempList, labels);
            break;
        case KERN:
            //PERF_PRINT_BEGIN("POINT-04", QString("format=txt count=%1").arg(kList.count()));
            exportThread->exportToTxtPublic(fileName, kList, labels, m_flag);
            break;
        case Kwin:
            //PERF_PRINT_BEGIN("POINT-04", QString("format=txt count=%1").arg(m_currentKwinList.count()));
            exportThread->exportToTxtPublic(fileName, m_currentKwinList, labels);
            break;
        case Dmesg:
            //PERF_PRINT_BEGIN("POINT-04", QString("format=txt count=%1").arg(dmesgList.count()));
            exportThread->exportToTxtPublic(fileName, dmesgList, labels);
            break;
        case Dnf:
            //PERF_PRINT_BEGIN("POINT-04", QString("format=txt count=%1").arg(dnfList.count()));
            exportThread->exportToTxtPublic(fileName, dnfList, labels);
            break;
        default:
            break;
        }
        QThreadPool::globalInstance()->start(exportThread);
    } else if (selectFilter.contains("(*.html)")) {
        switch (m_flag) {
        case JOURNAL:
            //PERF_PRINT_BEGIN("POINT-04", QString("format=html count=%1").arg(jList.count()));
            exportThread->exportToHtmlPublic(fileName, jList, labels, m_flag);
            break;
        case BOOT_KLU:
            //PERF_PRINT_BEGIN("POINT-04", QString("format=html count=%1").arg(jBootList.count()));
            exportThread->exportToHtmlPublic(fileName, jBootList, labels, JOURNAL);
            break;
        case APP: {
            //PERF_PRINT_BEGIN("POINT-04", QString("format=html count=%1").arg(appList.count()));
            QString appName = getAppName(m_curAppLog);
            exportThread->exportToHtmlPublic(fileName, appList, labels, appName);
            break;
        }
        case DPKG:
            //PERF_PRINT_BEGIN("POINT-04", QString("format=html count=%1").arg(dList.count()));
            exportThread->exportToHtmlPublic(fileName, dList, labels);
            break;
        case BOOT:
            //PERF_PRINT_BEGIN("POINT-04", QString("format=html count=%1").arg(currentBootList.count()));
            exportThread->exportToHtmlPublic(fileName, currentBootList, labels);
            break;
        case XORG:
            //PERF_PRINT_BEGIN("POINT-04", QString("format=html count=%1").arg(xList.count()));
            exportThread->exportToHtmlPublic(fileName, xList, labels);
            break;
        case Normal:
            //PERF_PRINT_BEGIN("POINT-04", QString("format=html count=%1").arg(nortempList.count()));
            exportThread->exportToHtmlPublic(fileName, nortempList, labels);
            break;
        case KERN:
            //PERF_PRINT_BEGIN("POINT-04", QString("format=html count=%1").arg(kList.count()));
            exportThread->exportToHtmlPublic(fileName, kList, labels, m_flag);
            break;
        case Kwin:
            //PERF_PRINT_BEGIN("POINT-04", QString("format=html count=%1").arg(m_currentKwinList.count()));
            exportThread->exportToHtmlPublic(fileName, m_currentKwinList, labels);
            break;
        case Dmesg:
            //PERF_PRINT_BEGIN("POINT-04", QString("format=txt count=%1").arg(dmesgList.count()));
            exportThread->exportToHtmlPublic(fileName, dmesgList, labels);
            break;
        case Dnf:
            //PERF_PRINT_BEGIN("POINT-04", QString("format=txt count=%1").arg(dnfList.count()));
            exportThread->exportToHtmlPublic(fileName, dnfList, labels);
            break;
        default:
            break;
        }
        QThreadPool::globalInstance()->start(exportThread);
    } else if (selectFilter.contains("(*.doc)")) {
        switch (m_flag) {
        case JOURNAL:
            //PERF_PRINT_BEGIN("POINT-04", QString("format=doc count=%1").arg(jList.count()));
            exportThread->exportToDocPublic(fileName, jList, labels, m_flag);
            break;
        case BOOT_KLU:
            //PERF_PRINT_BEGIN("POINT-04", QString("format=doc count=%1").arg(jBootList.count()));
            exportThread->exportToDocPublic(fileName, jBootList, labels, JOURNAL);
            break;
        case APP: {
            //PERF_PRINT_BEGIN("POINT-04", QString("format=doc count=%1").arg(appList.count()));
            QString appName = getAppName(m_curAppLog);
            exportThread->exportToDocPublic(fileName, appList, labels, appName);
            break;
        }
        case DPKG:
            //PERF_PRINT_BEGIN("POINT-04", QString("format=doc count=%1").arg(dList.count()));
            exportThread->exportToDocPublic(fileName, dList, labels);
            break;
        case BOOT:
            //PERF_PRINT_BEGIN("POINT-04", QString("format=doc count=%1").arg(currentBootList.count()));
            exportThread->exportToDocPublic(fileName, currentBootList, labels);
            break;
        case XORG:
            //PERF_PRINT_BEGIN("POINT-04", QString("format=doc count=%1").arg(xList.count()));
            exportThread->exportToDocPublic(fileName, xList, labels);
            break;
        case Normal:
            //PERF_PRINT_BEGIN("POINT-04", QString("format=doc count=%1").arg(nortempList.count()));
            exportThread->exportToDocPublic(fileName, nortempList, labels);
            break;
        case KERN:
            //PERF_PRINT_BEGIN("POINT-04", QString("format=doc count=%1").arg(kList.count()));
            exportThread->exportToDocPublic(fileName, kList, labels, m_flag);
            break;
        case Kwin:
            //PERF_PRINT_BEGIN("POINT-04", QString("format=doc count=%1").arg(m_currentKwinList.count()));
            exportThread->exportToDocPublic(fileName, m_currentKwinList, labels);
            break;
        case Dmesg:
            //PERF_PRINT_BEGIN("POINT-04", QString("format=txt count=%1").arg(dmesgList.count()));
            exportThread->exportToDocPublic(fileName, dmesgList, labels);
            break;
        case Dnf:
            //PERF_PRINT_BEGIN("POINT-04", QString("format=txt count=%1").arg(dnfList.count()));
            exportThread->exportToDocPublic(fileName, dnfList, labels);
            break;
        default:
            break;
        }
        QThreadPool::globalInstance()->start(exportThread);
    } else if (selectFilter.contains("(*.xls)")) {
        switch (m_flag) {
        case JOURNAL:
            //PERF_PRINT_BEGIN("POINT-04", QString("format=xls count=%1").arg(jList.count()));
            exportThread->exportToXlsPublic(fileName, jList, labels, m_flag);
            break;
        case BOOT_KLU:
            //PERF_PRINT_BEGIN("POINT-04", QString("format=xls count=%1").arg(jBootList.count()));
            exportThread->exportToXlsPublic(fileName, jBootList, labels, JOURNAL);
            break;
        case APP: {
            //PERF_PRINT_BEGIN("POINT-04", QString("format=xls count=%1").arg(appList.count()));
            QString appName = getAppName(m_curAppLog);
            exportThread->exportToXlsPublic(fileName, appList, labels, appName);
            break;
        }
        case DPKG:
            //PERF_PRINT_BEGIN("POINT-04", QString("format=xls count=%1").arg(dList.count()));
            exportThread->exportToXlsPublic(fileName, dList, labels);
            break;
        case BOOT:
            //PERF_PRINT_BEGIN("POINT-04", QString("format=xls count=%1").arg(currentBootList.count()));
            exportThread->exportToXlsPublic(fileName, currentBootList, labels);
            break;
        case XORG:
            //PERF_PRINT_BEGIN("POINT-04", QString("format=xls count=%1").arg(xList.count()));
            exportThread->exportToXlsPublic(fileName, xList, labels);
            break;
        case Normal:
            //PERF_PRINT_BEGIN("POINT-04", QString("format=xls count=%1").arg(nortempList.count()));
            exportThread->exportToXlsPublic(fileName, nortempList, labels);
            break;
        case KERN:
            //PERF_PRINT_BEGIN("POINT-04", QString("format=xls count=%1").arg(kList.count()));
            exportThread->exportToXlsPublic(fileName, kList, labels, m_flag);
            break;
        case Kwin:
            //PERF_PRINT_BEGIN("POINT-04", QString("format=xls count=%1").arg(m_currentKwinList.count()));
            exportThread->exportToXlsPublic(fileName, m_currentKwinList, labels);
            break;
        case Dmesg:
            //PERF_PRINT_BEGIN("POINT-04", QString("format=txt count=%1").arg(dmesgList.count()));
            exportThread->exportToXlsPublic(fileName, dmesgList, labels);
            break;
        case Dnf:
            //PERF_PRINT_BEGIN("POINT-04", QString("format=txt count=%1").arg(dnfList.count()));
            exportThread->exportToXlsPublic(fileName, dnfList, labels);
            break;
        default:
            break;
        }
        QThreadPool::globalInstance()->start(exportThread);
    }
}

void LogViewerPlugin::slot_exportResult(bool isSuccess)
{
    emit sigExportResult(isSuccess);
}
