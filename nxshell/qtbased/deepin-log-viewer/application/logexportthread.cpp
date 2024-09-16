// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logexportthread.h"
#include "utils.h"
#include "xlsxwriter.h"
#include "WordProcessingMerger.h"
#include "WordProcessingCompiler.h"
#include "dbusproxy/dldbushandler.h"

#include <DApplication>

#include <QDebug>
#include <QFile>
#include <QTextCodec>
#include <QTextStream>
#include <QTextDocument>
#include <QTextDocumentWriter>
#include <QElapsedTimer>
#include <QStandardPaths>
#include <QProcess>
#include <QLoggingCategory>

#include <malloc.h>
DWIDGET_USE_NAMESPACE

#ifdef QT_DEBUG
Q_LOGGING_CATEGORY(logExport, "org.deepin.log.viewer.export.work")
#else
Q_LOGGING_CATEGORY(logExport, "org.deepin.log.viewer.export.work", QtInfoMsg)
#endif

/**
 * @brief LogExportThread::LogExportThread 导出日志线程类构造函数
 * @param parent 父对象
 */
LogExportThread::LogExportThread(QObject *parent)
    :  QObject(parent),
       QRunnable(),
       m_appendExport(false)
{
    setAutoDelete(true);
    initMap();
}
/**
 * @brief LogExportThread::~LogExportThread 析构函数
 */
LogExportThread::~LogExportThread()
{
    //释放空闲内存
    malloc_trim(0);
}

/**
 * @brief LogExportThread::enableAppendExport 是否追加导出
 */
void LogExportThread::enableAppendExport(bool bEnable/* = true*/)
{
    m_appendExport = bEnable;
}

/**
 * @brief LogExportThread::exportToTxtPublic 导出到日志txt格式配置函数对QStandardItemModel数据类型的重载
 * @param fileName 导出文件路径全称
 * @param pModel 要导出的数据源，为QStandardItemModel
 * @param flag 导出的日志类型
 */
void LogExportThread::exportToTxtPublic(const QString &fileName, QStandardItemModel *pModel, LOG_FLAG flag)
{
    m_fileName = fileName;
    m_pModel = pModel;
    m_flag = flag;
    m_runMode = TxtModel;
    m_canRunning = true;
}

/**
 * @brief LogExportThread::exportToTxtPublic 导出到日志txt格式配置函数，对Json数据类型的重载（内核日志、窗管日志等）
 * @param fileName 导出文件路径全称
 * @param jList 要导出的数据源 数据类型为QString
 * @param labels 表头字符串
 * @param flag  导出的日志类型
 */
void LogExportThread::exportToTxtPublic(const QString &fileName, const QList<QString> &jList, const QStringList &labels, LOG_FLAG flag)
{
    m_fileName = fileName;
    m_logDataList = jList;
    m_runMode = TxtJOURNAL;
    m_labels = labels;
    m_flag = flag;
    m_canRunning = true;
}

/**
 * @brief LogExportThread::exportToTxtPublic 导出到日志txt格式配置函数，对LOG_MSG_JOURNAL数据类型的重载（指系统日志和内核日志）
 * @param fileName 导出文件路径全称
 * @param jList 要导出的数据源 数据类型为LOG_MSG_JOURNAL
 * @param labels 表头字符串
 * @param flag  导出的日志类型
 */
void LogExportThread::exportToTxtPublic(const QString &fileName, const QList<LOG_MSG_JOURNAL> &jList,  const QStringList &labels, LOG_FLAG flag)
{
    m_fileName = fileName;
    m_jList = jList;
    m_runMode = TxtJOURNAL;
    m_labels = labels;
    m_flag = flag;
    m_canRunning = true;

}

/**
 * @brief LogExportThread::exportToTxtPublic  导出到日志txt格式配置函数，对LOG_MSG_APPLICATOIN数据类型的重载（指应用日志）
 * @param fileName 导出文件路径全称
 * @param jList 要导出的数据源 数据类型为LOG_MSG_APPLICATOIN
 * @param labels 表头字符串
 * @param iAppName 导出的应用日志的应用名称
 */
void LogExportThread::exportToTxtPublic(const QString &fileName, const QList<LOG_MSG_APPLICATOIN> &jList, const QStringList &labels, const QString &iAppName)
{
    m_fileName = fileName;
    m_appList = jList;
    m_labels = labels;
    m_runMode = TxtAPP;
    m_canRunning = true;
    m_appName = iAppName;
}

/**
 * @brief LogExportThread::exportToTxtPublic 导出到日志txt格式配置函数，对LOG_MSG_DPKG数据类型的重载（指dpkg日志）
 * @param fileName 导出文件路径全称
 * @param jList  要导出的数据源 LOG_MSG_DPKG
 * @param labels 表头字符串
 */
void LogExportThread::exportToTxtPublic(const QString &fileName, const QList<LOG_MSG_DPKG> &jList, const QStringList &labels)
{
    m_fileName = fileName;
    m_dpkgList = jList;
    m_labels = labels;
    m_runMode = TxtDPKG;
    m_canRunning = true;
}

/**
 * @brief LogExportThread::exportToTxtPublic 导出到日志txt格式配置函数，对LOG_MSG_BOOT数据类型的重载（指启动日志）
 * @param fileName 导出文件路径全称
 * @param jList  要导出的数据源 LOG_MSG_BOOT
 * @param labels 表头字符串
 */
void LogExportThread::exportToTxtPublic(const QString &fileName, const QList<LOG_MSG_BOOT> &jList, const QStringList &labels)
{
    m_fileName = fileName;
    m_bootList = jList;
    m_labels = labels;
    m_runMode = TxtBOOT;
    m_canRunning = true;
}

/**
 * @brief LogExportThread::exportToTxtPublic 导出到日志txt格式配置函数，对LOG_MSG_XORG数据类型的重载（指xorg日志）
 * @param fileName 导出文件路径全称
 * @param jList 要导出的数据源 LOG_MSG_XORG
 * @param labels 表头字符串
 */
void LogExportThread::exportToTxtPublic(const QString &fileName, const QList<LOG_MSG_XORG> &jList, const QStringList &labels)
{
    m_fileName = fileName;
    m_xorgList = jList;
    m_labels = labels;
    m_runMode = TxtXORG;
    m_canRunning = true;
}


/**
 * @brief LogExportThread::exportToTxtPublic 导出到日志txt格式配置函数，对LOG_MSG_NORMAL数据类型的重载（指开关机日志）
 * @param fileName 导出文件路径全称
 * @param jList 要导出的数据源 LOG_MSG_NORMAL
 * @param labels 表头字符串
 */
void LogExportThread::exportToTxtPublic(const QString &fileName, const QList<LOG_MSG_NORMAL> &jList, const QStringList &labels)
{
    m_fileName = fileName;
    m_normalList = jList;
    m_labels = labels;
    m_runMode = TxtNORMAL;
    m_canRunning = true;
}

/**
 * @brief LogExportThread::exportToTxtPublic导出到日志txt格式配置函数，对LOG_MSG_KWIN数据类型的重载（指klu上的kwin日志）
 * @param fileName 导出文件路径全称
 * @param jList 要导出的数据源 LOG_MSG_KWIN
 * @param labels 表头字符串
 */
void LogExportThread::exportToTxtPublic(const QString &fileName, const QList<LOG_MSG_KWIN> &jList, const QStringList &labels)
{
    m_fileName = fileName;
    m_kwinList = jList;
    m_labels = labels;
    m_runMode = TxtKWIN;
    m_canRunning = true;
}

void LogExportThread::exportToTxtPublic(const QString &fileName, const QList<LOG_MSG_DNF> &jList, const QStringList &labels)
{
    m_fileName = fileName;
    m_dnfList = jList;
    m_labels = labels;
    m_runMode = TxtDNF;
    m_canRunning = true;
}

void LogExportThread::exportToTxtPublic(const QString &fileName, const QList<LOG_MSG_DMESG> &jList, const QStringList &labels)
{
    m_fileName = fileName;
    m_dmesgList = jList;
    m_labels = labels;
    m_runMode = TxtDMESG;
    m_canRunning = true;
}

void LogExportThread::exportToTxtPublic(const QString &fileName, const QList<LOG_MSG_AUDIT> &jList, const QStringList &labels)
{
    m_fileName = fileName;
    m_alist = jList;
    m_labels = labels;
    m_runMode = TxtAUDIT;
    m_canRunning = true;
}

/**
 * @brief LogExportThread::exportToHtmlPublic 导出到日志html格式配置函数对QStandardItemModel数据类型的重载
 * @param fileName 导出文件路径全称
 * @param pModel 要导出的数据源，为QStandardItemModel
 * @param flag 导出的日志类型
 */
void LogExportThread::exportToHtmlPublic(const QString &fileName, QStandardItemModel *pModel, LOG_FLAG flag)
{
    m_fileName = fileName;
    m_pModel = pModel;
    m_flag = flag;
    m_runMode = HtmlModel;
    m_canRunning = true;

}

void LogExportThread::exportToHtmlPublic(const QString &fileName, const QList<QString> &jList, const QStringList &labels, LOG_FLAG flag)
{
    m_fileName = fileName;
    m_logDataList = jList;
    m_runMode = HtmlJOURNAL;
    m_labels = labels;
    m_flag = flag;
    m_canRunning = true;
}

/**
 * @brief LogExportThread::exportToHtmlPublic 导出到日志html格式配置函数，对LOG_MSG_JOURNAL数据类型的重载（指系统日志和内核日志）
 * @param fileName 导出文件路径全称
 * @param jList 要导出的数据源 数据类型为LOG_MSG_JOURNAL
 * @param labels 表头字符串
 * @param flag  导出的日志类型
 */
void LogExportThread::exportToHtmlPublic(const QString &fileName, const QList<LOG_MSG_JOURNAL> &jList, const QStringList &labels, LOG_FLAG flag)
{
    m_fileName = fileName;
    m_jList = jList;
    m_runMode = HtmlJOURNAL;
    m_labels = labels;
    m_flag = flag;
    m_canRunning = true;

}

/**
 * @brief LogExportThread::exportToHtmlPublic  导出到日志html格式配置函数，对LOG_MSG_APPLICATOIN数据类型的重载（指应用日志）
 * @param fileName 导出文件路径全称
 * @param jList 要导出的数据源 数据类型为LOG_MSG_APPLICATOIN
 * @param labels 表头字符串
 * @param iAppName 导出的应用日志的应用名称
 */
void LogExportThread::exportToHtmlPublic(const QString &fileName, const QList<LOG_MSG_APPLICATOIN> &jList, const QStringList &labels, const QString &iAppName)
{
    m_fileName = fileName;
    m_appList = jList;
    m_labels = labels;
    m_runMode = HtmlAPP;
    m_canRunning = true;
    m_appName = iAppName;
}

/**
 * @brief LogExportThread::exportToHtmlPublic 导出到日志html格式配置函数，对LOG_MSG_DPKG数据类型的重载（指dpkg日志）
 * @param fileName 导出文件路径全称
 * @param jList  要导出的数据源 LOG_MSG_DPKG
 * @param labels 表头字符串
 */
void LogExportThread::exportToHtmlPublic(const QString &fileName, const QList<LOG_MSG_DPKG> &jList, const QStringList &labels)
{
    m_fileName = fileName;
    m_dpkgList = jList;
    m_labels = labels;
    m_runMode = HtmlDPKG;
    m_canRunning = true;
}

/**
 * @brief LogExportThread::exportToHtmlPublic导出到日志html格式配置函数，对LOG_MSG_BOOT数据类型的重载（指启动日志）
 * @param fileName 导出文件路径全称
 * @param jList  要导出的数据源 LOG_MSG_BOOT
 * @param labels 表头字符串
 */
void LogExportThread::exportToHtmlPublic(const QString &fileName, const QList<LOG_MSG_BOOT> &jList, const QStringList &labels)
{
    m_fileName = fileName;
    m_bootList = jList;
    m_labels = labels;
    m_runMode = HtmlBOOT;
    m_canRunning = true;
}

/**
 * @brief LogExportThread::exportToHtmlPublic导出到日志html格式配置函数，对LOG_MSG_XORG数据类型的重载（指xorg日志）
 * @param fileName 导出文件路径全称
 * @param jList 要导出的数据源 LOG_MSG_XORG
 * @param labels 表头字符串
 */
void LogExportThread::exportToHtmlPublic(const QString &fileName, const QList<LOG_MSG_XORG> &jList, const QStringList &labels)
{
    m_fileName = fileName;
    m_xorgList = jList;
    m_labels = labels;
    m_runMode = HtmlXORG;
    m_canRunning = true;
}

/**
 * @brief LogExportThread::exportToHtmlPublic导出到日志html格式配置函数，对LOG_MSG_NORMAL数据类型的重载（指开关机日志）
 * @param fileName 导出文件路径全称
 * @param jList 要导出的数据源 LOG_MSG_NORMAL
 * @param labels 表头字符串
 */
void LogExportThread::exportToHtmlPublic(const QString &fileName, const QList<LOG_MSG_NORMAL> &jList, const QStringList &labels)
{
    m_fileName = fileName;
    m_normalList = jList;
    m_labels = labels;
    m_runMode = HtmlNORMAL;
    m_canRunning = true;
}

/**
 * @brief LogExportThread::exportToHtmlPublic导出到日志html格式配置函数，对LOG_MSG_KWIN数据类型的重载（指klu上的kwin日志）
 * @param fileName 导出文件路径全称
 * @param jList 要导出的数据源 LOG_MSG_KWIN
 * @param labels 表头字符串
 */
void LogExportThread::exportToHtmlPublic(const QString &fileName, const QList<LOG_MSG_KWIN> &jList, const QStringList &labels)
{
    m_fileName = fileName;
    m_kwinList = jList;
    m_labels = labels;
    m_runMode = HtmlKWIN;
    m_canRunning = true;
}
void LogExportThread::exportToHtmlPublic(const QString &fileName, const QList<LOG_MSG_DNF> &jList, const QStringList &labels)
{
    m_fileName = fileName;
    m_dnfList = jList;
    m_labels = labels;
    m_runMode = HtmlDNF;
    m_canRunning = true;
}

void LogExportThread::exportToHtmlPublic(const QString &fileName, const QList<LOG_MSG_DMESG> &jList, const QStringList &labels)
{
    m_fileName = fileName;
    m_dmesgList = jList;
    m_labels = labels;
    m_runMode = HtmlDMESG;
    m_canRunning = true;
}

void LogExportThread::exportToHtmlPublic(const QString &fileName, const QList<LOG_MSG_AUDIT> &jList, const QStringList &labels)
{
    m_fileName = fileName;
    m_alist = jList;
    m_labels = labels;
    m_runMode = HtmlAUDIT;
    m_canRunning = true;
}

/**
 * @brief LogExportThread::exportToDocPublic导出到日志doc格式配置函数对QStandardItemModel数据类型的重载
 * @param fileName 导出文件路径全称
 * @param pModel 要导出的数据源，为QStandardItemModel
 * @param flag 导出的日志类型
 */
void LogExportThread::exportToDocPublic(const QString &fileName, QStandardItemModel *pModel, LOG_FLAG flag)
{
    m_fileName = fileName;
    m_pModel = pModel;
    m_flag = flag;
    m_runMode = DocModel;
    m_canRunning = true;

}

void LogExportThread::exportToDocPublic(const QString &fileName, const QList<QString> &jList, const QStringList &labels, LOG_FLAG iFlag)
{
    m_fileName = fileName;
    m_logDataList = jList;
    m_flag = iFlag;
    m_labels = labels;
    m_runMode = DocJOURNAL;
    m_canRunning = true;
}

/**
 * @brief LogExportThread::exportToDocPublic导出到日志doc格式配置函数，对LOG_MSG_JOURNAL数据类型的重载（指系统日志和内核日志）
 * @param fileName 导出文件路径全称
 * @param jList 要导出的数据源 数据类型为LOG_MSG_JOURNAL
 * @param labels 表头字符串
 * @param flag  导出的日志类型
 */
void LogExportThread::exportToDocPublic(const QString &fileName, const QList<LOG_MSG_JOURNAL> &jList, const QStringList &labels, LOG_FLAG iFlag)
{
    m_fileName = fileName;
    m_jList = jList;
    m_flag = iFlag;
    m_labels = labels;
    m_runMode = DocJOURNAL;
    m_canRunning = true;

}

/**
 * @brief LogExportThread::exportToDocPublic导出到日志doc格式配置函数，对LOG_MSG_APPLICATOIN数据类型的重载（指应用日志）
 * @param fileName 导出文件路径全称
 * @param jList 要导出的数据源 数据类型为LOG_MSG_APPLICATOIN
 * @param labels 表头字符串
 * @param iAppName 导出的应用日志的应用名称
 */
void LogExportThread::exportToDocPublic(const QString &fileName, const QList<LOG_MSG_APPLICATOIN> &jList, const QStringList &labels, const QString &iAppName)
{
    m_fileName = fileName;
    m_appList = jList;
    m_labels = labels;
    m_runMode = DocAPP;
    m_canRunning = true;
    m_appName = iAppName;
}

/**
 * @brief LogExportThread::exportToDocPublic 导出到日志doc格式配置函数，对LOG_MSG_DPKG数据类型的重载（指dpkg日志）
 * @param fileName 导出文件路径全称
 * @param jList  要导出的数据源 LOG_MSG_DPKG
 * @param labels 表头字符串
 */
void LogExportThread::exportToDocPublic(const QString &fileName, const QList<LOG_MSG_DPKG> &jList, const QStringList &labels)
{
    m_fileName = fileName;
    m_dpkgList = jList;
    m_labels = labels;
    m_runMode = DocDPKG;
    m_canRunning = true;
}

/**
 * @brief LogExportThread::exportToDocPublic 导出到日志doc格式配置函数，对LOG_MSG_BOOT数据类型的重载（指启动日志）
 * @param fileName 导出文件路径全称
 * @param jList  要导出的数据源 LOG_MSG_BOOT
 * @param labels 表头字符串
 */
void LogExportThread::exportToDocPublic(const QString &fileName, const QList<LOG_MSG_BOOT> &jList, const QStringList &labels)
{
    m_fileName = fileName;
    m_bootList = jList;
    m_labels = labels;
    m_runMode = DocBOOT;
    m_canRunning = true;
}

/**
 * @brief LogExportThread::exportToDocPublic 导出到日志doc格式配置函数，对LOG_MSG_XORG数据类型的重载（指xorg日志）
 * @param fileName 导出文件路径全称
 * @param jList 要导出的数据源 LOG_MSG_XORG
 * @param labels 表头字符串
 */
void LogExportThread::exportToDocPublic(const QString &fileName, const QList<LOG_MSG_XORG> &jList, const QStringList &labels)
{
    m_fileName = fileName;
    m_xorgList = jList;
    m_labels = labels;
    m_runMode = DocXORG;
    m_canRunning = true;
}

/**
 * @brief LogExportThread::exportToDocPublic 导出到日志doc格式配置函数，对LOG_MSG_NORMAL数据类型的重载（指开关机日志）
 * @param fileName 导出文件路径全称
 * @param jList 要导出的数据源 LOG_MSG_NORMAL
 * @param labels 表头字符串
 */
void LogExportThread::exportToDocPublic(const QString &fileName, const QList<LOG_MSG_NORMAL> &jList, const QStringList &labels)
{
    m_fileName = fileName;
    m_normalList = jList;
    m_labels = labels;
    m_runMode = DocNORMAL;
    m_canRunning = true;
}

/**
 * @brief LogExportThread::exportToDocPublic导出到日志doc格式配置函数，对LOG_MSG_KWIN数据类型的重载（指klu上的kwin日志）
 * @param fileName 导出文件路径全称
 * @param jList 要导出的数据源 LOG_MSG_KWIN
 * @param labels 表头字符串
 */
void LogExportThread::exportToDocPublic(const QString &fileName, const QList<LOG_MSG_KWIN> &jList, const QStringList &labels)
{
    m_fileName = fileName;
    m_kwinList = jList;
    m_labels = labels;
    m_runMode = DocKWIN;
    m_canRunning = true;
}
void LogExportThread::exportToDocPublic(const QString &fileName, const QList<LOG_MSG_DNF> &jList, const QStringList &labels)
{
    m_fileName = fileName;
    m_dnfList = jList;
    m_labels = labels;
    m_runMode = DocDNF;
    m_canRunning = true;
}

void LogExportThread::exportToDocPublic(const QString &fileName, const QList<LOG_MSG_DMESG> &jList, const QStringList &labels)
{
    m_fileName = fileName;
    m_dmesgList = jList;
    m_labels = labels;
    m_runMode = DocDMESG;
    m_canRunning = true;
}

void LogExportThread::exportToDocPublic(const QString &fileName, const QList<LOG_MSG_AUDIT> &jList, const QStringList &labels)
{
    m_fileName = fileName;
    m_alist = jList;
    m_labels = labels;
    m_runMode = DocAUDIT;
    m_canRunning = true;
}
/**
 * @brief LogExportThread::exportToXlsPublic 导出到日志xlsx格式配置函数对QStandardItemModel数据类型的重载
 * @param fileName 导出文件路径全称
 * @param pModel 要导出的数据源，为QStandardItemModel
 * @param flag 导出的日志类型
 */
void LogExportThread::exportToXlsPublic(const QString &fileName, QStandardItemModel *pModel, LOG_FLAG flag)
{
    m_fileName = fileName;
    m_pModel = pModel;
    m_flag = flag;
    m_runMode = XlsModel;
    m_canRunning = true;
}

void LogExportThread::exportToXlsPublic(const QString &fileName, const QList<QString> &jList, const QStringList &labels, LOG_FLAG iFlag)
{
    m_fileName = fileName;
    m_logDataList = jList;
    m_flag = iFlag;
    m_labels = labels;
    m_runMode = XlsJOURNAL;
    m_canRunning = true;
}

/**
 * @brief LogExportThread::exportToXlsPublic 导出到日志xlsx格式配置函数，对LOG_MSG_JOURNAL数据类型的重载（指系统日志和内核日志）
 * @param fileName 导出文件路径全称
 * @param jList 要导出的数据源 数据类型为LOG_MSG_JOURNAL
 * @param labels 表头字符串
 * @param flag  导出的日志类型
 */
void LogExportThread::exportToXlsPublic(const QString &fileName, const QList<LOG_MSG_JOURNAL> &jList, const QStringList &labels, LOG_FLAG iFlag)
{
    m_fileName = fileName;
    m_jList = jList;
    m_flag = iFlag;
    m_labels = labels;
    m_runMode = XlsJOURNAL;
    m_canRunning = true;
}

/**
 * @brief LogExportThread::exportToXlsPublic 导出到日志xlsx格式配置函数，对LOG_MSG_APPLICATOIN数据类型的重载（指应用日志）
 * @param fileName 导出文件路径全称
 * @param jList 要导出的数据源 数据类型为LOG_MSG_APPLICATOIN
 * @param labels 表头字符串
 * @param iAppName 导出的应用日志的应用名称
 */
void LogExportThread::exportToXlsPublic(const QString &fileName, const QList<LOG_MSG_APPLICATOIN> &jList, const QStringList &labels, const QString &iAppName)
{
    m_fileName = fileName;
    m_appList = jList;
    m_labels = labels;
    m_runMode = XlsAPP;
    m_canRunning = true;
    m_appName = iAppName;
}

/**
 * @brief LogExportThread::exportToXlsPublic 导出到日志xlsx格式配置函数，对LOG_MSG_DPKG数据类型的重载（指dpkg日志）
 * @param fileName 导出文件路径全称
 * @param jList  要导出的数据源 LOG_MSG_DPKG
 * @param labels 表头字符串
 */
void LogExportThread::exportToXlsPublic(const QString &fileName, const QList<LOG_MSG_DPKG> &jList, const QStringList &labels)
{
    m_fileName = fileName;
    m_dpkgList = jList;
    m_labels = labels;
    m_runMode = XlsDPKG;
    m_canRunning = true;
}

/**
 * @brief LogExportThread::exportToXlsPublic导出到日志xlsx格式配置函数，对LOG_MSG_BOOT数据类型的重载（指启动日志）
 * @param fileName 导出文件路径全称
 * @param jList  要导出的数据源 LOG_MSG_DPKG
 * @param labels 表头字符串
 */
void LogExportThread::exportToXlsPublic(const QString &fileName, const QList<LOG_MSG_BOOT> &jList, const QStringList &labels)
{
    m_fileName = fileName;
    m_bootList = jList;
    m_labels = labels;
    m_runMode = XlsBOOT;
    m_canRunning = true;
}

/**
 * @brief LogExportThread::exportToXlsPublic导出到日志xlsx格式配置函数，对LOG_MSG_XORG数据类型的重载（指xorg日志）
 * @param fileName 导出文件路径全称
 * @param jList 要导出的数据源 LOG_MSG_XORG
 * @param labels 表头字符串
 */
void LogExportThread::exportToXlsPublic(const QString &fileName, const QList<LOG_MSG_XORG> &jList, const QStringList &labels)
{
    m_fileName = fileName;
    m_xorgList = jList;
    m_labels = labels;
    m_runMode = XlsXORG;
    m_canRunning = true;
}

/**
 * @brief LogExportThread::exportToXlsPublic 导出到日志xlsx格式配置函数，对LOG_MSG_NORMAL数据类型的重载（指开关机日志）
 * @param fileName 导出文件路径全称
 * @param jList 要导出的数据源 LOG_MSG_NORMAL
 * @param labels 表头字符串
 */
void LogExportThread::exportToXlsPublic(const QString &fileName, const QList<LOG_MSG_NORMAL> &jList, const QStringList &labels)
{
    m_fileName = fileName;
    m_normalList = jList;
    m_labels = labels;
    m_runMode = XlsNORMAL;
    m_canRunning = true;
}

/**
 * @brief LogExportThread::exportToXlsPublic 导出到日志xlsw格式配置函数，对LOG_MSG_KWIN数据类型的重载（指klu上的kwin日志）
 * @param fileName 导出文件路径全称
 * @param jList 要导出的数据源 LOG_MSG_KWIN
 * @param labels 表头字符串
 */
void LogExportThread::exportToXlsPublic(const QString &fileName, const QList<LOG_MSG_KWIN> &jList, const QStringList &labels)
{
    m_fileName = fileName;
    m_kwinList = jList;
    m_labels = labels;
    m_runMode = XlsKWIN;
    m_canRunning = true;
}
void LogExportThread::exportToXlsPublic(const QString &fileName, const QList<LOG_MSG_DNF> &jList, const QStringList &labels)
{
    m_fileName = fileName;
    m_dnfList = jList;
    m_labels = labels;
    m_runMode = XlsDNF;
    m_canRunning = true;
}

void LogExportThread::exportToXlsPublic(const QString &fileName, const QList<LOG_MSG_DMESG> &jList, const QStringList &labels)
{
    m_fileName = fileName;
    m_dmesgList = jList;
    m_labels = labels;
    m_runMode = XlsDMESG;
    m_canRunning = true;
}

void LogExportThread::exportToXlsPublic(const QString &fileName, const QList<LOG_MSG_AUDIT> &jList, const QStringList &labels)
{
    m_fileName = fileName;
    m_alist = jList;
    m_labels = labels;
    m_runMode = XlsAUDIT;
    m_canRunning = true;
}

void LogExportThread::exportToZipPublic(const QString &fileName, const QList<LOG_MSG_COREDUMP> &jList, const QStringList &labels)
{
    m_fileName = fileName;
    m_coredumplist = jList;
    m_labels = labels;
    m_runMode = ZipCoredump;
    m_canRunning = true;
}

/**
 * @brief LogExportThread::isProcessing 返回当前线程获取数据逻辑启动停止控制的变量
 * @return 当前线程获取数据逻辑启动停止控制的变量
 */
bool LogExportThread::isProcessing()
{
    return m_canRunning;
}

/**
 * @brief LogExportThread::stopImmediately 停止前线程获取数据逻辑
 */
void LogExportThread::stopImmediately()
{
    m_canRunning = false;
}

/**
 * @brief LogExportThread::exportToTxt 导出数据到txt格式函数对QStandardItemModel数据类型的重载
 * @param fileName 导出文件路径全称
 * @param pModel 要导出的数据源，为QStandardItemModel
 * @param flag 导出的日志类型
 * @return 是否导出成功
 */
bool LogExportThread::exportToTxt(const QString &fileName, QStandardItemModel *pModel, LOG_FLAG flag)
{
    QFile fi(fileName);
    //判断文件路径是否存在，不存在就返回错误
    if (!fi.open(QIODevice::WriteOnly)) {
        emit sigResult(false);
        emit sigError(openErroStr);
        return false;
    }
    try {
        //判读啊model指针是为空，为空则不导出
        if (!pModel) {
            throw  QString("model is null");
        }
        QTextStream out(&fi);
        //日志类型为应用日志时
        if (flag == APP) {
            for (int row = 0; row < pModel->rowCount(); ++row) {
                //导出逻辑启动停止控制，外部把m_canRunning置false时停止运行，抛出异常处理
                if (!m_canRunning) {
                    throw  QString(stopStr);
                }
                //获取表头项目和对应内容，拼成目标字符串写入文件
                out << pModel->horizontalHeaderItem(0)->text() << ": "
                    << pModel->item(row, 0)->data(Qt::UserRole + 6).toString() << " ";
                for (int col = 1; col < pModel->columnCount(); ++col) {
                    out << pModel->horizontalHeaderItem(col)->text() << ": "
                        << pModel->item(row, col)->text() << " ";
                }
                out << "\n";
                //导出进度信号
                sigProgress(row + 1, pModel->rowCount());
            }
        } else {
            //日志类型为其他所有日志时
            for (int row = 0; row < pModel->rowCount(); row++) {
                //导出逻辑启动停止控制，外部把m_canRunning置false时停止运行，抛出异常处理
                if (!m_canRunning) {
                    throw  QString(stopStr);
                }
                //获取表头项目和对应内容，拼成目标字符串写入文件
                for (int col = 0; col < pModel->columnCount(); col++) {
                    out << pModel->horizontalHeaderItem(col)->text() << ": "
                        << pModel->item(row, col)->text() << " ";
                }
                out << "\n";
                //导出进度信号
                sigProgress(row + 1, pModel->rowCount());
            }
        }
    } catch (const QString &ErrorStr) {
        //捕获到异常，导出失败，发出失败信号
        qCWarning(logExport) << "Export Stop" << ErrorStr;
        fi.close();
        emit sigResult(false);
        if (ErrorStr != stopStr) {
            emit sigError(QString("export error: %1").arg(ErrorStr));
        }
        return false;
    }
    fi.close();
    //导出成功，如果此时被停止，则发出导出失败信号
    emit sigResult(m_canRunning);
    return m_canRunning;
}

/**
 * @brief LogExportThread::exportToTxt 导出到日志txt格式配置函数，对Json数据类型的重载（内核日志、窗管日志等）
 * @param fileName 导出文件路径全称
 * @param jList 要导出的数据源 数据类型为QString
 * @param labels 表头字符串
 * @param flag  导出的日志类型
 * @return 是否导出成功
 */
bool LogExportThread::exportToTxt(const QString &fileName, const QList<QString> &jList,  const QStringList &labels, LOG_FLAG flag)
{
    //判断文件路径是否存在，不存在就返回错误
    QFile fi(fileName);
    if (!fi.open(m_appendExport ? (QIODevice::Append | QIODevice::WriteOnly) : QIODevice::WriteOnly)) {
        emit sigResult(false);
        emit sigError(openErroStr);
        return false;
    }
    try {

        QTextStream out(&fi);
        //导出日志为系统日志时
        if (flag == JOURNAL) {
            for (int i = 0; i < jList.count(); i++) {
                //导出逻辑启动停止控制，外部把m_canRunning置false时停止运行，抛出异常处理
                if (!m_canRunning) {
                    throw  QString(stopStr);
                }
                LOG_MSG_BASE jMsg;
                jMsg.fromJson(jList[i]);
                //导出各字段的描述和对应内容拼成目标字符串
                out << QString(DApplication::translate("Table", "Level:")) << jMsg.level << " ";
                out << QString(DApplication::translate("Table", "Process:")) << jMsg.daemonName << " ";
                out << QString(DApplication::translate("Table", "Date and Time:")) << jMsg.dateTime << " ";
                if (jMsg.msg.isEmpty()) {
                    out << QString(DApplication::translate("Table", "Info:"))
                        << QString(DApplication::translate("Table", "Null")) << " ";  // modify for bug
                } else {
                    out << QString(DApplication::translate("Table", "Info:")) << jMsg.msg << " ";
                }
                out << QString(DApplication::translate("Table", "User:")) << jMsg.hostName << " ";
                out << QString(DApplication::translate("Table", "PID:")) << jMsg.daemonId << " ";
                out << "\n";
                //导出进度信号
                sigProgress(i + 1, jList.count());
            }
        } else if (flag == KERN) {
            //导出日志为内核日志时
            for (int i = 0; i < jList.count(); i++) {
                //导出逻辑启动停止控制，外部把m_canRunning置false时停止运行，抛出异常处理
                if (!m_canRunning) {
                    throw  QString(stopStr);
                }
                int col = 0;
                LOG_MSG_BASE jMsg;
                jMsg.fromJson(jList.at(i));
                //导出各字段的描述和对应内容拼成目标字符串
                out << labels.value(col++, "") << ":" << jMsg.dateTime << " ";
                out << labels.value(col++, "") << ":" << jMsg.hostName << " ";
                out << labels.value(col++, "") << ":" << jMsg.daemonName << " ";
                out << labels.value(col++, "") << ":" << jMsg.msg << " ";
                out << "\n";
                //导出进度信号
                sigProgress(i + 1, jList.count());
            }
        } else if (flag == Kwin) {
            for (int i = 0; i < jList.count(); i++) {
                //导出逻辑启动停止控制，外部把m_canRunning置false时停止运行，抛出异常处理
                if (!m_canRunning) {
                    throw  QString(stopStr);
                }
                //导出各字段的描述和对应内容拼成目标字符串
                LOG_MSG_BASE jMsg;
                jMsg.fromJson(jList.at(i));
                int col = 0;
                out << labels.value(col++, "") << ":" << jMsg.msg << " ";
                out << "\n";
                //导出进度信号
                sigProgress(i + 1, jList.count());
            }
        }
        //设置文件编码为utf8
        out.setCodec(QTextCodec::codecForName("utf-8"));

    } catch (const QString &ErrorStr) {
        //捕获到异常，导出失败，发出失败信号
        qCWarning(logExport) << "Export Stop" << ErrorStr;
        fi.close();
        emit sigResult(false);
        if (ErrorStr != stopStr) {
            emit sigError(QString("export error: %1").arg(ErrorStr));
        }
        return false;
    }
    fi.close();
    //导出成功，如果此时被停止，则发出导出失败信号
    emit sigResult(m_canRunning);
    return m_canRunning;
}

/**
 * @brief LogExportThread::exportToTxt导出到日志txt格式函数，对LOG_MSG_JOURNAL数据类型的重载（指系统日志和内核日志）
 * @param fileName 导出文件路径全称
 * @param jList 要导出的数据源 数据类型为LOG_MSG_JOURNAL
 * @param labels 表头字符串
 * @param flag  导出的日志类型
 * @return 是否导出成功
 */
bool LogExportThread::exportToTxt(const QString &fileName, const QList<LOG_MSG_JOURNAL> &jList,  const QStringList &labels, LOG_FLAG flag)
{
    //判断文件路径是否存在，不存在就返回错误
    QFile fi(fileName);
    if (!fi.open(QIODevice::WriteOnly)) {
        emit sigResult(false);
        emit sigError(openErroStr);
        return false;
    }
    try {

        QTextStream out(&fi);
        //导出日志为系统日志时
        if (flag == JOURNAL) {
            for (int i = 0; i < jList.count(); i++) {
                //导出逻辑启动停止控制，外部把m_canRunning置false时停止运行，抛出异常处理
                if (!m_canRunning) {
                    throw  QString(stopStr);
                }
                LOG_MSG_JOURNAL jMsg = jList.at(i);
                //        out << QString(DApplication::translate("Table", "Date and Time:")) <<
                //        jMsg.dateTime << " "; out << QString(DApplication::translate("Table", "User:")) <<
                //        jMsg.hostName << " "; out << QString(DApplication::translate("Table", "Daemon:"))
                //        << jMsg.daemonName << " "; out << QString(DApplication::translate("Table",
                //        "PID:")) << jMsg.daemonId << " "; out << QString(DApplication::translate("Table",
                //        "Level:")) << jMsg.level << " "; out << QString(DApplication::translate("Table",
                //        "Info:")) << jMsg.msg << " "; //delete bug
                //导出各字段的描述和对应内容拼成目标字符串
                out << QString(DApplication::translate("Table", "Level:")) << jMsg.level << " ";
                out << QString(DApplication::translate("Table", "Process:")) << jMsg.daemonName << " ";
                out << QString(DApplication::translate("Table", "Date and Time:")) << jMsg.dateTime << " ";
                if (jMsg.msg.isEmpty()) {
                    out << QString(DApplication::translate("Table", "Info:"))
                        << QString(DApplication::translate("Table", "Null")) << " ";  // modify for bug
                } else {
                    out << QString(DApplication::translate("Table", "Info:")) << jMsg.msg << " ";
                }
                out << QString(DApplication::translate("Table", "User:")) << jMsg.hostName << " ";
                out << QString(DApplication::translate("Table", "PID:")) << jMsg.daemonId << " ";
                out << "\n";
                //导出进度信号
                sigProgress(i + 1, jList.count());
            }
        } else if (flag == KERN) {
            //导出日志为内核日志时
            for (int i = 0; i < jList.count(); i++) {
                //导出逻辑启动停止控制，外部把m_canRunning置false时停止运行，抛出异常处理
                if (!m_canRunning) {
                    throw  QString(stopStr);
                }
                int col = 0;
                LOG_MSG_JOURNAL jMsg = jList.at(i);
                //导出各字段的描述和对应内容拼成目标字符串
                out << labels.value(col++, "") << ":" << jMsg.dateTime << " ";
                out << labels.value(col++, "") << ":" << jMsg.hostName << " ";
                out << labels.value(col++, "") << ":" << jMsg.daemonName << " ";
                out << labels.value(col++, "") << ":" << jMsg.msg << " ";
                out << "\n";
                //导出进度信号
                sigProgress(i + 1, jList.count());
            }
        }
        //设置文件编码为utf8
        out.setCodec(QTextCodec::codecForName("utf-8"));

    } catch (const QString &ErrorStr) {
        //捕获到异常，导出失败，发出失败信号
        qCWarning(logExport) << "Export Stop" << ErrorStr;
        fi.close();
        emit sigResult(false);
        if (ErrorStr != stopStr) {
            emit sigError(QString("export error: %1").arg(ErrorStr));
        }
        return false;
    }
    fi.close();
    //导出成功，如果此时被停止，则发出导出失败信号
    emit sigResult(m_canRunning);
    return m_canRunning;
}

/**
 * @brief LogExportThread::exportToTxt导出到日志txt格式函数，对LOG_MSG_APPLICATOIN数据类型的重载（指应用日志）
 * @param fileName 导出文件路径全称
 * @param jList 要导出的数据源 数据类型为LOG_MSG_APPLICATOIN
 * @param labels 表头字符串
 * @param iAppName 导出的应用日志的应用名称
 * @return 是否导出成功
 */
bool LogExportThread::exportToTxt(const QString &fileName, const QList<LOG_MSG_APPLICATOIN> &jList, const QStringList &labels, const QString &iAppName)
{
    //判断文件路径是否存在，不存在就返回错误
    QFile fi(fileName);
    if (!fi.open(QIODevice::WriteOnly)) {
        emit sigResult(false);
        emit sigError(openErroStr);
        return false;
    }
    try {
        QTextStream out(&fi);
        out.setCodec(QTextCodec::codecForName("utf-8"));
        for (int i = 0; i < jList.count(); i++) {
            //导出逻辑启动停止控制，外部把m_canRunning置false时停止运行，抛出异常处理
            if (!m_canRunning) {
                throw  QString(stopStr);
            }
            int col = 0;
            //导出各字段的描述和对应内容拼成目标字符串
            LOG_MSG_APPLICATOIN jMsg = jList.at(i);
            out << labels.value(col++, "") << ":" << strTranslate(jMsg.level)  << " ";
            out << labels.value(col++, "") << ":" << jMsg.dateTime << " ";
            if (!jMsg.subModule.isEmpty())
                out << labels.value(col++, "") << ":" << iAppName + "_" + jMsg.subModule << " ";
            else
                out << labels.value(col++, "") << ":" << iAppName << " ";
            out << labels.value(col++, "") << ":" << jMsg.msg << " ";
            out << "\n";
            //导出进度信号
            sigProgress(i + 1, jList.count());
        }
    } catch (const QString &ErrorStr) {
        //捕获到异常，导出失败，发出失败信号
        qCWarning(logExport) << "Export Stop" << ErrorStr;
        fi.close();
        emit sigResult(false);
        if (ErrorStr != stopStr) {
            emit sigError(QString("export error: %1").arg(ErrorStr));
        }
        return false;
    }
    fi.close();
    //导出成功，如果此时被停止，则发出导出失败信号
    emit sigResult(m_canRunning);
    return m_canRunning;
}

/**
 * @brief LogExportThread::exportToTxt 导出到日志txt格式配置函数，对LOG_MSG_DPKG数据类型的重载（指dpkg日志）
 * @param fileName 导出文件路径全称
 * @param jList  要导出的数据源 LOG_MSG_DPKG
 * @param labels 表头字符串
 * @return 是否导出成功
 */
bool LogExportThread::exportToTxt(const QString &fileName, const QList<LOG_MSG_DPKG> &jList, const QStringList &labels)
{
    //判断文件路径是否存在，不存在就返回错误
    QFile fi(fileName);
    if (!fi.open(QIODevice::WriteOnly)) {
        emit sigResult(false);
        emit sigError(openErroStr);
        return false;
    }
    try {
        QTextStream out(&fi);
        out.setCodec(QTextCodec::codecForName("utf-8"));
        for (int i = 0; i < jList.count(); i++) {
            //导出逻辑启动停止控制，外部把m_canRunning置false时停止运行，抛出异常处理
            if (!m_canRunning) {
                throw  QString(stopStr);
            }
            int col = 0;
            //导出各字段的描述和对应内容拼成目标字符串
            LOG_MSG_DPKG jMsg = jList.at(i);
            out << labels.value(col++, "") << ":" << jMsg.dateTime << " ";
            out << labels.value(col++, "") << ":" << jMsg.msg << " ";
            out << labels.value(col++, "") << ":" << jMsg.action << " ";
            out << "\n";
            //导出进度信号
            sigProgress(i + 1, jList.count());
        }
    } catch (const QString &ErrorStr) {
        //捕获到异常，导出失败，发出失败信号
        qCWarning(logExport) << "Export Stop" << ErrorStr;
        fi.close();
        emit sigResult(false);
        if (ErrorStr != stopStr) {
            emit sigError(QString("export error: %1").arg(ErrorStr));
        }
        return false;
    }
    fi.close();
    //导出成功，如果此时被停止，则发出导出失败信号
    emit sigResult(m_canRunning);
    return m_canRunning;
}

/**
 * @brief LogExportThread::exportToTxt 导出到日志txt格式函数，对LOG_MSG_BOOT数据类型的重载（指启动日志）
 * @param fileName 导出文件路径全称
 * @param jList  要导出的数据源 LOG_MSG_BOOT
 * @param labels 表头字符串
 * @return 是否导出成功
 */
bool LogExportThread::exportToTxt(const QString &fileName, const QList<LOG_MSG_BOOT> &jList, const QStringList &labels)
{
    //判断文件路径是否存在，不存在就返回错误
    QFile fi(fileName);
    if (!fi.open(QIODevice::WriteOnly)) {
        emit sigResult(false);
        emit sigError(openErroStr);
        return false;
    }
    try {
        QTextStream out(&fi);
        out.setCodec(QTextCodec::codecForName("utf-8"));
        for (int i = 0; i < jList.count(); i++) {
            //导出逻辑启动停止控制，外部把m_canRunning置false时停止运行，抛出异常处理
            if (!m_canRunning) {
                throw  QString(stopStr);
            }
            //导出各字段的描述和对应内容拼成目标字符串
            LOG_MSG_BOOT jMsg = jList.at(i);
            int col = 0;
            out << labels.value(col++, "") << ":" << jMsg.status << " ";
            out << labels.value(col++, "") << ":" << jMsg.msg << " ";
            out << "\n";
            //导出进度信号
            sigProgress(i + 1, jList.count());
        }
    } catch (const QString &ErrorStr) {
        //捕获到异常，导出失败，发出失败信号
        qCWarning(logExport) << "Export Stop" << ErrorStr;
        fi.close();
        emit sigResult(false);
        if (ErrorStr != stopStr) {
            emit sigError(QString("export error: %1").arg(ErrorStr));
        }
        return false;
    }
    fi.close();
    //导出成功，如果此时被停止，则发出导出失败信号
    emit sigResult(m_canRunning);
    return m_canRunning;
}

/**
 * @brief LogExportThread::exportToTxt 导出到日志txt格式函数，对LOG_MSG_XORG数据类型的重载（指xorg日志）
 * @param fileName 导出文件路径全称
 * @param jList 要导出的数据源 LOG_MSG_XORG
 * @param labels 表头字符串
 * @return 是否导出成功
 */
bool LogExportThread::exportToTxt(const QString &fileName, const QList<LOG_MSG_XORG> &jList, const QStringList &labels)
{
    //判断文件路径是否存在，不存在就返回错误
    QFile fi(fileName);
    if (!fi.open(QIODevice::WriteOnly)) {
        emit sigResult(false);
        emit sigError(openErroStr);
        return false;
    }
    try {
        QTextStream out(&fi);
        out.setCodec(QTextCodec::codecForName("utf-8"));
        for (int i = 0; i < jList.count(); i++) {
            //导出逻辑启动停止控制，外部把m_canRunning置false时停止运行，抛出异常处理
            if (!m_canRunning) {
                throw  QString(stopStr);
            }
            //导出各字段的描述和对应内容拼成目标字符串
            LOG_MSG_XORG jMsg = jList.at(i);
            int col = 0;
            out << labels.value(col++, "") << ":" << jMsg.offset << " ";
            out << labels.value(col++, "") << ":" << jMsg.msg << " ";
            out << "\n";
            //导出进度信号
            sigProgress(i + 1, jList.count());
        }
    } catch (const QString &ErrorStr) {
        //捕获到异常，导出失败，发出失败信号
        qCWarning(logExport) << "Export Stop" << ErrorStr;
        fi.close();
        emit sigResult(false);
        if (ErrorStr != stopStr) {
            emit sigError(QString("export error: %1").arg(ErrorStr));
        }
        return false;
    }
    fi.close();
    //导出成功，如果此时被停止，则发出导出失败信号
    emit sigResult(m_canRunning);
    return m_canRunning;
}

/**
 * @brief LogExportThread::exportToTxt 导出到日志txt格式函数，对LOG_MSG_NORMAL数据类型的重载（指开关机日志）
 * @param fileName 导出文件路径全称
 * @param jList 要导出的数据源 LOG_MSG_NORMAL
 * @param labels 表头字符串
 * @return 是否导出成功
 */
bool LogExportThread::exportToTxt(const QString &fileName, const QList<LOG_MSG_NORMAL> &jList, const QStringList &labels)
{
    //判断文件路径是否存在，不存在就返回错误
    QFile fi(fileName);
    if (!fi.open(QIODevice::WriteOnly)) {
        emit sigResult(false);
        emit sigError(openErroStr);
        return false;
    }
    try {
        QTextStream out(&fi);
        out.setCodec(QTextCodec::codecForName("utf-8"));
        for (int i = 0; i < jList.count(); i++) {
            //导出逻辑启动停止控制，外部把m_canRunning置false时停止运行，抛出异常处理
            if (!m_canRunning) {
                throw  QString(stopStr);
            }
            //导出各字段的描述和对应内容拼成目标字符串
            LOG_MSG_NORMAL jMsg = jList.at(i);
            int col = 0;
            out << labels.value(col++, "") << ":" << jMsg.eventType << " ";
            out << labels.value(col++, "") << ":" << jMsg.userName << " ";
            out << labels.value(col++, "") << ":" << jMsg.dateTime << " ";
            out << labels.value(col++, "") << ":" << jMsg.msg << " ";
            out << "\n";
            //导出进度信号
            sigProgress(i + 1, jList.count());
        }
    } catch (const QString &ErrorStr) {
        //捕获到异常，导出失败，发出失败信号
        qCWarning(logExport) << "Export Stop" << ErrorStr;
        fi.close();
        emit sigResult(false);
        if (ErrorStr != stopStr) {
            emit sigError(QString("export error: %1").arg(ErrorStr));
        }
        return false;
    }
    fi.close();
    //导出成功，如果此时被停止，则发出导出失败信号
    emit sigResult(m_canRunning);
    return m_canRunning;
}
/**
 * @brief LogExportThread::exportToTxt导出到日志txt格式函数，对LOG_MSG_KWIN数据类型的重载（指klu上的kwin日志）
 * @param fileName 导出文件路径全称
 * @param jList 要导出的数据源 LOG_MSG_KWIN
 * @param labels 表头字符串
 * @return 是否导出成功
 */
bool LogExportThread::exportToTxt(const QString &fileName, const QList<LOG_MSG_KWIN> &jList, const QStringList &labels)
{
    //判断文件路径是否存在，不存在就返回错误
    QFile fi(fileName);
    if (!fi.open(QIODevice::WriteOnly)) {
        emit sigResult(false);
        emit sigError(openErroStr);
        return false;
    }
    try {
        QTextStream out(&fi);
        out.setCodec(QTextCodec::codecForName("utf-8"));
        for (int i = 0; i < jList.count(); i++) {
            //导出逻辑启动停止控制，外部把m_canRunning置false时停止运行，抛出异常处理
            if (!m_canRunning) {
                throw  QString(stopStr);
            }
            //导出各字段的描述和对应内容拼成目标字符串
            LOG_MSG_KWIN jMsg = jList.at(i);
            int col = 0;
            out << labels.value(col++, "") << ":" << jMsg.msg << " ";
            out << "\n";
            //导出进度信号
            sigProgress(i + 1, jList.count());
        }
    } catch (const QString &ErrorStr) {
        //捕获到异常，导出失败，发出失败信号
        qCWarning(logExport) << "Export Stop" << ErrorStr;
        fi.close();
        emit sigResult(false);
        if (ErrorStr != stopStr) {
            emit sigError(QString("export error: %1").arg(ErrorStr));
        }
        return false;
    }
    fi.close();
    //导出成功，如果此时被停止，则发出导出失败信号
    emit sigResult(m_canRunning);
    return m_canRunning;
}

bool LogExportThread::exportToTxt(const QString &fileName, const QList<LOG_MSG_DNF> &jList, const QStringList &labels)
{
    //判断文件路径是否存在，不存在就返回错误
    QFile fi(fileName);
    if (!fi.open(QIODevice::WriteOnly)) {
        emit sigResult(false);
        emit sigError(openErroStr);
        return false;
    }
    try {
        QTextStream out(&fi);
        out.setCodec(QTextCodec::codecForName("utf-8"));
        for (int i = 0; i < jList.count(); i++) {
            //导出逻辑启动停止控制，外部把m_canRunning置false时停止运行，抛出异常处理
            if (!m_canRunning) {
                throw QString(stopStr);
            }
            //导出各字段的描述和对应内容拼成目标字符串

            int col = 0;
            LOG_MSG_DNF jMsg = jList.at(i);
            out << labels.value(col++, "") << ":" << jMsg.level << " ";
            out << labels.value(col++, "") << ":" << jMsg.dateTime << " ";
            out << labels.value(col++, "") << ":" << jMsg.msg << " ";
            out << "\n";
            //导出进度信号
            sigProgress(i + 1, jList.count());
        }
    } catch (const QString &ErrorStr) {
        //捕获到异常，导出失败，发出失败信号
        qCWarning(logExport) << "Export Stop" << ErrorStr;
        fi.close();
        emit sigResult(false);
        if (ErrorStr != stopStr) {
            emit sigError(QString("export error: %1").arg(ErrorStr));
        }
        return false;
    }
    fi.close();
    //导出成功，如果此时被停止，则发出导出失败信号
    emit sigResult(m_canRunning);
    return m_canRunning;
}

bool LogExportThread::exportToTxt(const QString &fileName, const QList<LOG_MSG_DMESG> &jList, const QStringList &labels)
{
    //判断文件路径是否存在，不存在就返回错误
    QFile fi(fileName);
    if (!fi.open(QIODevice::WriteOnly)) {
        emit sigResult(false);
        emit sigError(openErroStr);
        return false;
    }
    try {
        QTextStream out(&fi);
        out.setCodec(QTextCodec::codecForName("utf-8"));
        for (int i = 0; i < jList.count(); i++) {
            //导出逻辑启动停止控制，外部把m_canRunning置false时停止运行，抛出异常处理
            if (!m_canRunning) {
                throw QString(stopStr);
            }
            //导出各字段的描述和对应内容拼成目标字符串

            int col = 0;
            LOG_MSG_DMESG jMsg = jList.at(i);
            out << labels.value(col++, "") << ":" << jMsg.level << " ";
            out << labels.value(col++, "") << ":" << jMsg.dateTime << " ";
            out << labels.value(col++, "") << ":" << jMsg.msg << " ";
            out << "\n";
            //导出进度信号
            sigProgress(i + 1, jList.count());
        }
    } catch (const QString &ErrorStr) {
        //捕获到异常，导出失败，发出失败信号
        qCWarning(logExport) << "Export Stop" << ErrorStr;
        fi.close();
        emit sigResult(false);
        if (ErrorStr != stopStr) {
            emit sigError(QString("export error: %1").arg(ErrorStr));
        }
        return false;
    }
    fi.close();
    //导出成功，如果此时被停止，则发出导出失败信号
    emit sigResult(m_canRunning);
    return m_canRunning;
}

bool LogExportThread::exportToTxt(const QString &fileName, const QList<LOG_MSG_AUDIT> &jList, const QStringList &labels)
{
    //判断文件路径是否存在，不存在就返回错误
    QFile fi(fileName);
    if (!fi.open(QIODevice::WriteOnly)) {
        emit sigResult(false);
        emit sigError(openErroStr);
        return false;
    }
    try {
        QTextStream out(&fi);
        out.setCodec(QTextCodec::codecForName("utf-8"));
        for (int i = 0; i < jList.count(); i++) {
            //导出逻辑启动停止控制，外部把m_canRunning置false时停止运行，抛出异常处理
            if (!m_canRunning) {
                throw  QString(stopStr);
            }
            //导出各字段的描述和对应内容拼成目标字符串
            LOG_MSG_AUDIT jMsg = jList.at(i);
            int col = 0;
            out << labels.value(col++, "") << ":" << jMsg.eventType << " ";
            out << labels.value(col++, "") << ":" << jMsg.dateTime << " ";
            out << labels.value(col++, "") << ":" << jMsg.processName << " ";
            out << labels.value(col++, "") << ":" << jMsg.status << " ";
            out << labels.value(col++, "") << ":" << jMsg.msg << " ";
            out << "\n";
            //导出进度信号
            sigProgress(i + 1, jList.count());
        }
    } catch (const QString &ErrorStr) {
        //捕获到异常，导出失败，发出失败信号
        qCWarning(logExport) << "Export Stop" << ErrorStr;
        fi.close();
        emit sigResult(false);
        if (ErrorStr != stopStr) {
            emit sigError(QString("export error: %1").arg(ErrorStr));
        }
        return false;
    }
    fi.close();
    //导出成功，如果此时被停止，则发出导出失败信号
    emit sigResult(m_canRunning);
    return m_canRunning;
}

bool LogExportThread::exportToDoc(const QString &fileName, const QList<QString> &jList, const QStringList &labels, LOG_FLAG iFlag)
{
    try {
        QString tempdir ;
        if (iFlag == JOURNAL) {
            tempdir = "/usr/share/deepin-log-viewer/DocxTemplate/6column.dfw";
        } else if (iFlag == KERN) {
            tempdir = "/usr/share/deepin-log-viewer/DocxTemplate/4column.dfw";
        } else if (iFlag == Kwin) {
            tempdir = "/usr/share/deepin-log-viewer/DocxTemplate/1column.dfw";
        } else {
            qCWarning(logExport) << "exportToDoc type is Wrong!";
            return false;
        }
        if (!QFile(tempdir).exists()) {
            qCWarning(logExport) << "export docx template is not exisits";
            return  false;
        }

        DocxFactory:: WordProcessingMerger &l_merger = DocxFactory:: WordProcessingMerger::getInstance();


        l_merger.load(tempdir.toStdString());
        //往表头中添加表头描述，表头为第一行，数据则在下面
        for (int col = 0; col < labels.count(); ++col) {
            l_merger.setClipboardValue("tableRow", QString("column%1").arg(col + 1).toStdString(), labels.at(col).toStdString());

        }
        l_merger.paste("tableRow");
        //计算导出进度条最后一段的长度，因为最后写入文件那一段没有进度，所以预先留出一段进度
        int end = static_cast<int>(jList.count() * 0.1 > 5 ? jList.count() * 0.1 : 5);
        for (int row = 0; row < jList.count(); ++row) {
            //导出逻辑启动停止控制，外部把m_canRunning置false时停止运行，抛出异常处理
            if (!m_canRunning) {
                throw  QString(stopStr);
            }
            LOG_MSG_BASE message;
            message.fromJson(jList.at(row));
            //把数据填入表格单元格中
            if (iFlag == JOURNAL) {
                l_merger.setClipboardValue("tableRow", QString("column1").toStdString(), message.level.toStdString());
                l_merger.setClipboardValue("tableRow", QString("column2").toStdString(), message.daemonName.toStdString());
                l_merger.setClipboardValue("tableRow", QString("column3").toStdString(), message.dateTime.toStdString());
                l_merger.setClipboardValue("tableRow", QString("column4").toStdString(), message.msg.toStdString());
                l_merger.setClipboardValue("tableRow", QString("column5").toStdString(), message.hostName.toStdString());
                l_merger.setClipboardValue("tableRow", QString("column6").toStdString(), message.daemonId.toStdString());
            } else if (iFlag == KERN) {
                l_merger.setClipboardValue("tableRow", QString("column1").toStdString(), message.dateTime.toStdString());
                l_merger.setClipboardValue("tableRow", QString("column2").toStdString(), message.hostName.toStdString());
                l_merger.setClipboardValue("tableRow", QString("column3").toStdString(), message.daemonName.toStdString());
                l_merger.setClipboardValue("tableRow", QString("column4").toStdString(), message.msg.toStdString());
            } else if (iFlag == Kwin) {
                l_merger.setClipboardValue("tableRow", QString("column1").toStdString(), message.msg.toStdString());
            }
            l_merger.paste("tableRow");
            //导出进度信号
            sigProgress(row + 1, jList.count() + end);
        }
        //保存，把拼好的xml写入文件中
        QString fileNamex = fileName + "x";

        QFile rsNameFile(fileName) ;
        if (rsNameFile.exists()) {
            rsNameFile.remove();
        }
        l_merger.save(fileNamex.toStdString());
        QFile(fileNamex).rename(fileName);
    } catch (const QString &ErrorStr) {
        //捕获到异常，导出失败，发出失败信号
        qCWarning(logExport) << "Export Stop" << ErrorStr;
        if (!m_canRunning) {
            Utils::checkAndDeleteDir(m_fileName);
        }

        emit sigResult(false);
        if (ErrorStr != stopStr) {
            emit sigError(QString("export error: %1").arg(ErrorStr));
        }
        return false;
    }
    //如果取消导出，则删除文件
    if (!m_canRunning) {
        Utils::checkAndDeleteDir(m_fileName);
    }
    //100%进度
    sigProgress(100, 100);
    //延时200ms再发送导出成功信号，关闭导出进度框，让100%的进度有时间显示
    Utils::sleep(200);
    //导出成功，如果此时被停止，则发出导出失败信号
    emit sigResult(m_canRunning);
    return m_canRunning;
}

/**
 * @brief LogExportThread::exportToDoc导出到日志doc格式函数，对LOG_MSG_JOURNAL数据类型的重载（指系统日志和内核日志）
 * @param fileName 导出文件路径全称
 * @param jList 要导出的数据源 数据类型为LOG_MSG_JOURNAL
 * @param labels 表头字符串
 * @param flag  导出的日志类型
 * @return 是否导出成功
 */
bool LogExportThread::exportToDoc(const QString &fileName, const QList<LOG_MSG_JOURNAL> &jList,
                                  const QStringList &labels, LOG_FLAG iFlag)
{
    try {
        QString tempdir ;
        if (iFlag == JOURNAL) {
            tempdir = "/usr/share/deepin-log-viewer/DocxTemplate/6column.dfw";
        } else if (iFlag == KERN) {
            tempdir = "/usr/share/deepin-log-viewer/DocxTemplate/4column.dfw";
        } else {
            qCWarning(logExport) << "exportToDoc type is Wrong!";
            return false;
        }
        if (!QFile(tempdir).exists()) {
            qCWarning(logExport) << "export docx template is not exisits";
            return  false;
        }

        DocxFactory:: WordProcessingMerger &l_merger = DocxFactory:: WordProcessingMerger::getInstance();


        l_merger.load(tempdir.toStdString());
        //往表头中添加表头描述，表头为第一行，数据则在下面
        for (int col = 0; col < labels.count(); ++col) {
            l_merger.setClipboardValue("tableRow", QString("column%1").arg(col + 1).toStdString(), labels.at(col).toStdString());

        }
        l_merger.paste("tableRow");
        //计算导出进度条最后一段的长度，因为最后写入文件那一段没有进度，所以预先留出一段进度
        int end = static_cast<int>(jList.count() * 0.1 > 5 ? jList.count() * 0.1 : 5);
        for (int row = 0; row < jList.count(); ++row) {
            //导出逻辑启动停止控制，外部把m_canRunning置false时停止运行，抛出异常处理
            if (!m_canRunning) {
                throw  QString(stopStr);
            }
            LOG_MSG_JOURNAL message = jList.at(row);
            //把数据填入表格单元格中
            if (iFlag == JOURNAL) {
                l_merger.setClipboardValue("tableRow", QString("column1").toStdString(), message.level.toStdString());
                l_merger.setClipboardValue("tableRow", QString("column2").toStdString(), message.daemonName.toStdString());
                l_merger.setClipboardValue("tableRow", QString("column3").toStdString(), message.dateTime.toStdString());
                l_merger.setClipboardValue("tableRow", QString("column4").toStdString(), message.msg.toStdString());
                l_merger.setClipboardValue("tableRow", QString("column5").toStdString(), message.hostName.toStdString());
                l_merger.setClipboardValue("tableRow", QString("column6").toStdString(), message.daemonId.toStdString());
            } else if (iFlag == KERN) {
                l_merger.setClipboardValue("tableRow", QString("column1").toStdString(), message.dateTime.toStdString());
                l_merger.setClipboardValue("tableRow", QString("column2").toStdString(), message.hostName.toStdString());
                l_merger.setClipboardValue("tableRow", QString("column3").toStdString(), message.daemonName.toStdString());
                l_merger.setClipboardValue("tableRow", QString("column4").toStdString(), message.msg.toStdString());
            }
            l_merger.paste("tableRow");
            //导出进度信号
            sigProgress(row + 1, jList.count() + end);
        }
        //保存，把拼好的xml写入文件中
        QString fileNamex = fileName + "x";

        QFile rsNameFile(fileName) ;
        if (rsNameFile.exists()) {
            rsNameFile.remove();
        }
        l_merger.save(fileNamex.toStdString());
        QFile(fileNamex).rename(fileName);
    } catch (const QString &ErrorStr) {
        //捕获到异常，导出失败，发出失败信号
        qCWarning(logExport) << "Export Stop" << ErrorStr;
        if (!m_canRunning) {
            Utils::checkAndDeleteDir(m_fileName);
        }

        emit sigResult(false);
        if (ErrorStr != stopStr) {
            emit sigError(QString("export error: %1").arg(ErrorStr));
        }
        return false;
    }
    //如果取消导出，则删除文件
    if (!m_canRunning) {
        Utils::checkAndDeleteDir(m_fileName);
    }
    //100%进度
    sigProgress(100, 100);
    //延时200ms再发送导出成功信号，关闭导出进度框，让100%的进度有时间显示
    Utils::sleep(200);
    //导出成功，如果此时被停止，则发出导出失败信号
    emit sigResult(m_canRunning);
    return m_canRunning;
}

/**
 * @brief LogExportThread::exportToDoc导出到日志doc格式配置函数，对LOG_MSG_APPLICATOIN数据类型的重载（指应用日志）
 * @param fileName 导出文件路径全称        if (!QFile(fileName).exists()) {
            qCWarning(logExport) << "export file dir is not exisits";
            return  false;
        }
 * @param jList 要导出的数据源 数据类型为LOG_MSG_APPLICATOIN
 * @param labels 表头字符串
 * @param iAppName 导出的应用日志的应用名称
 * @return 是否导出成功
 */
bool LogExportThread::exportToDoc(const QString &fileName, const QList<LOG_MSG_APPLICATOIN> &jList, const QStringList &labels, QString &iAppName)
{
    try {
        QString tempdir = "/usr/share/deepin-log-viewer/DocxTemplate/4column.dfw";
        if (!QFile(tempdir).exists()) {
            qCWarning(logExport) << "export docx template is not exisits";
            return  false;
        }

        DocxFactory:: WordProcessingMerger &l_merger = DocxFactory:: WordProcessingMerger::getInstance();
        l_merger.load(tempdir.toStdString());
        //往表头中添加表头描述，表头为第一行，数据则在下面
        for (int col = 0; col < labels.count(); ++col) {
            l_merger.setClipboardValue("tableRow", QString("column%1").arg(col + 1).toStdString(), labels.at(col).toStdString());

        }
        l_merger.paste("tableRow");
        //计算导出进度条最后一段的长度，因为最后写入文件那一段没有进度，所以预先留出一段进度
        int end = static_cast<int>(jList.count() * 0.1 > 5 ? jList.count() * 0.1 : 5);
        for (int row = 0; row < jList.count(); ++row) {
            //导出逻辑启动停止控制，外部把m_canRunning置false时停止运行，抛出异常处理
            if (!m_canRunning) {
                throw  QString(stopStr);
            }
            LOG_MSG_APPLICATOIN message = jList.at(row);
            l_merger.setClipboardValue("tableRow", QString("column1").toStdString(), strTranslate(message.level).toStdString());
            l_merger.setClipboardValue("tableRow", QString("column2").toStdString(), message.dateTime.toStdString());
            l_merger.setClipboardValue("tableRow", QString("column3").toStdString(), iAppName.toStdString());
            l_merger.setClipboardValue("tableRow", QString("column4").toStdString(), message.msg.toStdString());
            l_merger.paste("tableRow");
            //导出进度信号
            sigProgress(row + 1, jList.count() + end);
        }

        //保存，把拼好的xml写入文件中
        QString fileNamex = fileName + "x";

        QFile rsNameFile(fileName) ;
        if (rsNameFile.exists()) {
            rsNameFile.remove();
        }
        l_merger.save(fileNamex.toStdString());
        QFile(fileNamex).rename(fileName);

    } catch (const QString &ErrorStr) {
        //捕获到异常，导出失败，发出失败信号
        qCWarning(logExport) << "Export Stop" << ErrorStr;
        if (!m_canRunning) {
            Utils::checkAndDeleteDir(m_fileName);
        }

        emit sigResult(false);
        if (ErrorStr != stopStr) {
            emit sigError(QString("export error: %1").arg(ErrorStr));
        }
        return false;
    }
    //如果取消导出，则删除文件
    if (!m_canRunning) {
        Utils::checkAndDeleteDir(m_fileName);
    }
    //100%进度
    sigProgress(100, 100);
    //延时200ms再发送导出成功信号，关闭导出进度框，让100%的进度有时间显示
    Utils::sleep(200);
    //导出成功，如果此时被停止，则发出导出失败信号
    emit sigResult(m_canRunning);

    return m_canRunning;
}

/**
 * @brief LogExportThread::exportToDoc导出到日志doc格式函数，对LOG_MSG_DPKG数据类型的重载（指dpkg日志）
 * @param fileName 导出文件路径全称
 * @param jList  要导出的数据源 LOG_MSG_DPKG
 * @param labels 表头字符串
 * @return 是否导出成功
 */
bool LogExportThread::exportToDoc(const QString &fileName, const QList<LOG_MSG_DPKG> &jList, const QStringList &labels)
{
    try {
        QString tempdir = "/usr/share/deepin-log-viewer/DocxTemplate/3column.dfw";
        if (!QFile(tempdir).exists()) {
            qCWarning(logExport) << "export docx template is not exisits";
            return  false;
        }

        DocxFactory:: WordProcessingMerger &l_merger = DocxFactory:: WordProcessingMerger::getInstance();
        l_merger.load(tempdir.toStdString());
        //往表头中添加表头描述，表头为第一行，数据则在下面
        for (int col = 0; col < labels.count(); ++col) {
            l_merger.setClipboardValue("tableRow", QString("column%1").arg(col + 1).toStdString(), labels.at(col).toStdString());

        }
        l_merger.paste("tableRow");
        //计算导出进度条最后一段的长度，因为最后写入文件那一段没有进度，所以预先留出一段进度
        int end = static_cast<int>(jList.count() * 0.1 > 5 ? jList.count() * 0.1 : 5);
        for (int row = 0; row < jList.count(); ++row) {
            //导出逻辑启动停止控制，外部把m_canRunning置false时停止运行，抛出异常处理
            if (!m_canRunning) {
                throw  QString(stopStr);
            }
            LOG_MSG_DPKG message = jList.at(row);
            l_merger.setClipboardValue("tableRow", QString("column1").toStdString(), message.dateTime.toStdString());
            l_merger.setClipboardValue("tableRow", QString("column2").toStdString(), message.msg.toStdString());
            l_merger.setClipboardValue("tableRow", QString("column3").toStdString(), message.action.toStdString());
            l_merger.paste("tableRow");
            //导出进度信号
            sigProgress(row + 1, jList.count() + end);
        }

        //保存，把拼好的xml写入文件中
        QString fileNamex = fileName + "x";

        QFile rsNameFile(fileName) ;
        if (rsNameFile.exists()) {
            rsNameFile.remove();
        }
        l_merger.save(fileNamex.toStdString());
        QFile(fileNamex).rename(fileName);

    } catch (const QString &ErrorStr) {
        //捕获到异常，导出失败，发出失败信号
        qCWarning(logExport) << "Export Stop" << ErrorStr;
        if (!m_canRunning) {
            Utils::checkAndDeleteDir(m_fileName);
        }

        emit sigResult(false);
        if (ErrorStr != stopStr) {
            emit sigError(QString("export error: %1").arg(ErrorStr));
        }
        return false;
    }
    //如果取消导出，则删除文件
    if (!m_canRunning) {
        Utils::checkAndDeleteDir(m_fileName);
    }
    //100%进度
    sigProgress(100, 100);
    //延时200ms再发送导出成功信号，关闭导出进度框，让100%的进度有时间显示
    Utils::sleep(200);
    //导出成功，如果此时被停止，则发出导出失败信号
    emit sigResult(m_canRunning);

    return m_canRunning;
}

/**
 * @brief LogExportThread::exportToDoc导出到日志doc格式配置函数，对LOG_MSG_BOOT数据类型的重载（指启动日志）
 * @param fileName 导出文件路径全称
 * @param jList  要导出的数据源 LOG_MSG_BOOT
 * @param labels 表头字符串
 * @return 是否导出成功
 */
bool LogExportThread::exportToDoc(const QString &fileName, const QList<LOG_MSG_BOOT> &jList, const QStringList &labels)
{
    try {
        QString tempdir = "/usr/share/deepin-log-viewer/DocxTemplate/2column.dfw";
        if (!QFile(tempdir).exists()) {
            qCWarning(logExport) << "export docx template is not exisits";
            return  false;
        }

        DocxFactory:: WordProcessingMerger &l_merger = DocxFactory:: WordProcessingMerger::getInstance();
        l_merger.load(tempdir.toStdString());
        //往表头中添加表头描述，表头为第一行，数据则在下面
        for (int col = 0; col < labels.count(); ++col) {
            l_merger.setClipboardValue("tableRow", QString("column%1").arg(col + 1).toStdString(), labels.at(col).toStdString());

        }
        l_merger.paste("tableRow");
        //计算导出进度条最后一段的长度，因为最后写入文件那一段没有进度，所以预先留出一段进度
        int end = static_cast<int>(jList.count() * 0.1 > 5 ? jList.count() * 0.1 : 5);
        for (int row = 0; row < jList.count(); ++row) {
            //导出逻辑启动停止控制，外部把m_canRunning置false时停止运行，抛出异常处理
            if (!m_canRunning) {
                throw  QString(stopStr);
            }
            LOG_MSG_BOOT message = jList.at(row);
            l_merger.setClipboardValue("tableRow", QString("column1").toStdString(), message.status.toStdString());
            l_merger.setClipboardValue("tableRow", QString("column2").toStdString(), message.msg.toStdString());
            l_merger.paste("tableRow");
            //导出进度信号
            sigProgress(row + 1, jList.count() + end);
        }
        //保存，把拼好的xml写入文件中
        QString fileNamex = fileName + "x";

        QFile rsNameFile(fileName) ;
        if (rsNameFile.exists()) {
            rsNameFile.remove();
        }
        l_merger.save(fileNamex.toStdString());
        QFile(fileNamex).rename(fileName);
    } catch (const QString &ErrorStr) {
        //捕获到异常，导出失败，发出失败信号
        qCWarning(logExport) << "Export Stop" << ErrorStr;
        if (!m_canRunning) {
            Utils::checkAndDeleteDir(m_fileName);
        }

        emit sigResult(false);
        if (ErrorStr != stopStr) {
            emit sigError(QString("export error: %1").arg(ErrorStr));
        }
        return false;
    }
    //如果取消导出，则删除文件
    if (!m_canRunning) {
        Utils::checkAndDeleteDir(m_fileName);
    }
    //100%进度
    sigProgress(100, 100);
    //延时200ms再发送导出成功信号，关闭导出进度框，让100%的进度有时间显示
    Utils::sleep(200);
    //导出成功，如果此时被停止，则发出导出失败信号
    emit sigResult(m_canRunning);
    return m_canRunning;
}

/**
 * @brief LogExportThread::exportToDoc导出到日志doc格式函数，对LOG_MSG_XORG数据类型的重载（指xorg日志）
 * @param fileName 导出文件路径全称
 * @param jList 要导出的数据源 LOG_MSG_XORG
 * @param labels 表头字符串
 * @return 是否导出成功
 */
bool LogExportThread::exportToDoc(const QString &fileName, const QList<LOG_MSG_XORG> &jList, const QStringList &labels)
{
    try {
        QString tempdir = "/usr/share/deepin-log-viewer/DocxTemplate/2column.dfw";
        if (!QFile(tempdir).exists()) {
            qCWarning(logExport) << "export docx template is not exisits";
            return  false;
        }

        DocxFactory:: WordProcessingMerger &l_merger = DocxFactory:: WordProcessingMerger::getInstance();
        l_merger.load(tempdir.toStdString());
        //往表头中添加表头描述，表头为第一行，数据则在下面
        for (int col = 0; col < labels.count(); ++col) {
            l_merger.setClipboardValue("tableRow", QString("column%1").arg(col + 1).toStdString(), labels.at(col).toStdString());

        }
        l_merger.paste("tableRow");
        //计算导出进度条最后一段的长度，因为最后写入文件那一段没有进度，所以预先留出一段进度
        int end = static_cast<int>(jList.count() * 0.1 > 5 ? jList.count() * 0.1 : 5);
        for (int row = 0; row < jList.count(); ++row) {
            //导出逻辑启动停止控制，外部把m_canRunning置false时停止运行，抛出异常处理
            if (!m_canRunning) {
                throw  QString(stopStr);
            }
            LOG_MSG_XORG message = jList.at(row);
            l_merger.setClipboardValue("tableRow", QString("column1").toStdString(), message.offset.toStdString());
            l_merger.setClipboardValue("tableRow", QString("column2").toStdString(), message.msg.toStdString());
            l_merger.paste("tableRow");
            //导出进度信号
            sigProgress(row + 1, jList.count() + end);
        }
        //保存，把拼好的xml写入文件中
        QString fileNamex = fileName + "x";

        QFile rsNameFile(fileName) ;
        if (rsNameFile.exists()) {
            rsNameFile.remove();
        }
        l_merger.save(fileNamex.toStdString());
        QFile(fileNamex).rename(fileName);

    } catch (const QString &ErrorStr) {
        //捕获到异常，导出失败，发出失败信号
        qCWarning(logExport) << "Export Stop" << ErrorStr;
        if (!m_canRunning) {
            Utils::checkAndDeleteDir(m_fileName);
        }

        emit sigResult(false);
        if (ErrorStr != stopStr) {
            emit sigError(QString("export error: %1").arg(ErrorStr));
        }
        return false;
    }
    //如果取消导出，则删除文件
    if (!m_canRunning) {
        Utils::checkAndDeleteDir(m_fileName);
    }
    //100%进度
    sigProgress(100, 100);
    //延时200ms再发送导出成功信号，关闭导出进度框，让100%的进度有时间显示
    Utils::sleep(200);
    //导出成功，如果此时被停止，则发出导出失败信号
    emit sigResult(m_canRunning);

    return m_canRunning;
}

/**
 * @brief LogExportThread::exportToDoc 导出到日志doc格式函数，对LOG_MSG_NORMAL数据类型的重载（指开关机日志）
 * @param fileName 导出文件路径全称
 * @param jList 要导出的数据源 LOG_MSG_NORMAL
 * @param labels 表头字符串
 * @return 是否导出成功
 */
bool LogExportThread::exportToDoc(const QString &fileName, const QList<LOG_MSG_NORMAL> &jList, const QStringList &labels)
{

    try {
        QString tempdir = "/usr/share/deepin-log-viewer/DocxTemplate/4column.dfw";
        if (!QFile(tempdir).exists()) {
            qCWarning(logExport) << "export docx template is not exisits";
            return  false;
        }

        DocxFactory:: WordProcessingMerger &l_merger = DocxFactory:: WordProcessingMerger::getInstance();
        l_merger.load(tempdir.toStdString());
        //往表头中添加表头描述，表头为第一行，数据则在下面
        for (int col = 0; col < labels.count(); ++col) {
            l_merger.setClipboardValue("tableRow", QString("column%1").arg(col + 1).toStdString(), labels.at(col).toStdString());

        }
        l_merger.paste("tableRow");
        //计算导出进度条最后一段的长度，因为最后写入文件那一段没有进度，所以预先留出一段进度
        int end = static_cast<int>(jList.count() * 0.1 > 5 ? jList.count() * 0.1 : 5);
        for (int row = 0; row < jList.count(); ++row) {
            //导出逻辑启动停止控制，外部把m_canRunning置false时停止运行，抛出异常处理
            if (!m_canRunning) {
                throw  QString(stopStr);
            }
            LOG_MSG_NORMAL message = jList.at(row);
            l_merger.setClipboardValue("tableRow", QString("column1").toStdString(), message.eventType.toStdString());
            l_merger.setClipboardValue("tableRow", QString("column2").toStdString(), message.userName.toStdString());
            l_merger.setClipboardValue("tableRow", QString("column3").toStdString(), message.dateTime.toStdString());
            l_merger.setClipboardValue("tableRow", QString("column4").toStdString(), message.msg.toStdString());
            l_merger.paste("tableRow");
            //导出进度信号
            sigProgress(row + 1, jList.count() + end);
        }
        //保存，把拼好的xml写入文件中
        QString fileNamex = fileName + "x";

        QFile rsNameFile(fileName) ;
        if (rsNameFile.exists()) {
            rsNameFile.remove();
        }
        l_merger.save(fileNamex.toStdString());
        QFile(fileNamex).rename(fileName);

    } catch (const QString &ErrorStr) {
        //捕获到异常，导出失败，发出失败信号
        qCWarning(logExport) << "Export Stop" << ErrorStr;
        if (!m_canRunning) {
            Utils::checkAndDeleteDir(m_fileName);
        }

        emit sigResult(false);
        if (ErrorStr != stopStr) {
            emit sigError(QString("export error: %1").arg(ErrorStr));
        }
        return false;
    }
    //如果取消导出，则删除文件
    if (!m_canRunning) {
        Utils::checkAndDeleteDir(m_fileName);
    }
    //100%进度
    sigProgress(100, 100);
    //延时200ms再发送导出成功信号，关闭导出进度框，让100%的进度有时间显示
    Utils::sleep(200);
    //导出成功，如果此时被停止，则发出导出失败信号
    emit sigResult(m_canRunning);

    return m_canRunning;
}

/**
 * @brief LogExportThread::exportToDoc导出到日志doc格式函数，对LOG_MSG_KWIN数据类型的重载（指klu上的kwin日志）
 * @param fileName 导出文件路径全称
 * @param jList 要导出的数据源 LOG_MSG_KWIN
 * @param labels 表头字符串
 * @return 是否导出成功
 */
bool LogExportThread::exportToDoc(const QString &fileName, const QList<LOG_MSG_KWIN> &jList, const QStringList &labels)
{

    try {
        QString tempdir = "/usr/share/deepin-log-viewer/DocxTemplate/1column.dfw";
        if (!QFile(tempdir).exists()) {
            qCWarning(logExport) << "export docx template is not exisits";
            return  false;
        }

        DocxFactory:: WordProcessingMerger &l_merger = DocxFactory:: WordProcessingMerger::getInstance();
        l_merger.load(tempdir.toStdString());
        //往表头中添加表头描述，表头为第一行，数据则在下面
        for (int col = 0; col < labels.count(); ++col) {
            l_merger.setClipboardValue("tableRow", QString("column%1").arg(col + 1).toStdString(), labels.at(col).toStdString());

        }
        l_merger.paste("tableRow");
        //计算导出进度条最后一段的长度，因为最后写入文件那一段没有进度，所以预先留出一段进度
        int end = static_cast<int>(jList.count() * 0.1 > 5 ? jList.count() * 0.1 : 5);
        for (int row = 0; row < jList.count(); ++row) {
            //导出逻辑启动停止控制，外部把m_canRunning置false时停止运行，抛出异常处理
            if (!m_canRunning) {
                throw  QString(stopStr);
            }
            LOG_MSG_KWIN message = jList.at(row);
            l_merger.setClipboardValue("tableRow", QString("column1").toStdString(), message.msg.toStdString());
            l_merger.paste("tableRow");
            //导出进度信号
            sigProgress(row + 1, jList.count() + end);
        }
        //保存，把拼好的xml写入文件中
        QString fileNamex = fileName + "x";

        QFile rsNameFile(fileName) ;
        if (rsNameFile.exists()) {
            rsNameFile.remove();
        }
        l_merger.save(fileNamex.toStdString());
        QFile(fileNamex).rename(fileName);

    } catch (const QString &ErrorStr) {
        //捕获到异常，导出失败，发出失败信号
        qCWarning(logExport) << "Export Stop" << ErrorStr;
        if (!m_canRunning) {
            Utils::checkAndDeleteDir(m_fileName);
        }

        emit sigResult(false);
        if (ErrorStr != stopStr) {
            emit sigError(QString("export error: %1").arg(ErrorStr));
        }
        return false;
    }
    //如果取消导出，则删除文件
    if (!m_canRunning) {
        Utils::checkAndDeleteDir(m_fileName);
    }
    //100%进度
    sigProgress(100, 100);
    //延时200ms再发送导出成功信号，关闭导出进度框，让100%的进度有时间显示
    Utils::sleep(200);
    //导出成功，如果此时被停止，则发出导出失败信号
    emit sigResult(m_canRunning);

    return m_canRunning;
}

bool LogExportThread::exportToDoc(const QString &fileName, const QList<LOG_MSG_DNF> &jList, const QStringList &labels)
{
    try {
        QString tempdir = "/usr/share/deepin-log-viewer/DocxTemplate/1column.dfw";
        if (!QFile(tempdir).exists()) {
            qCWarning(logExport) << "export docx template is not exisits";
            return false;
        }

        DocxFactory::WordProcessingMerger &l_merger = DocxFactory::WordProcessingMerger::getInstance();
        l_merger.load(tempdir.toStdString());
        //往表头中添加表头描述，表头为第一行，数据则在下面
        for (int col = 0; col < labels.count(); ++col) {
            l_merger.setClipboardValue("tableRow", QString("column%1").arg(col + 1).toStdString(), labels.at(col).toStdString());
        }
        l_merger.paste("tableRow");
        //计算导出进度条最后一段的长度，因为最后写入文件那一段没有进度，所以预先留出一段进度
        int end = static_cast<int>(jList.count() * 0.1 > 5 ? jList.count() * 0.1 : 5);
        for (int row = 0; row < jList.count(); ++row) {
            //导出逻辑启动停止控制，外部把m_canRunning置false时停止运行，抛出异常处理
            if (!m_canRunning) {
                throw QString(stopStr);
            }
            LOG_MSG_DNF message = jList.at(row);
            l_merger.setClipboardValue("tableRow", QString("column1").toStdString(), message.msg.toStdString());
            l_merger.paste("tableRow");
            //导出进度信号
            sigProgress(row + 1, jList.count() + end);
        }
        //保存，把拼好的xml写入文件中
        QString fileNamex = fileName + "x";

        QFile rsNameFile(fileName);
        if (rsNameFile.exists()) {
            rsNameFile.remove();
        }
        l_merger.save(fileNamex.toStdString());
        QFile(fileNamex).rename(fileName);

    } catch (const QString &ErrorStr) {
        //捕获到异常，导出失败，发出失败信号
        qCWarning(logExport) << "Export Stop" << ErrorStr;
        if (!m_canRunning) {
            Utils::checkAndDeleteDir(m_fileName);
        }

        emit sigResult(false);
        if (ErrorStr != stopStr) {
            emit sigError(QString("export error: %1").arg(ErrorStr));
        }
        return false;
    }
    //如果取消导出，则删除文件
    if (!m_canRunning) {
        Utils::checkAndDeleteDir(m_fileName);
    }
    //100%进度
    sigProgress(100, 100);
    //延时200ms再发送导出成功信号，关闭导出进度框，让100%的进度有时间显示
    Utils::sleep(200);
    //导出成功，如果此时被停止，则发出导出失败信号
    emit sigResult(m_canRunning);

    return m_canRunning;
}

bool LogExportThread::exportToDoc(const QString &fileName, const QList<LOG_MSG_DMESG> &jList, const QStringList &labels)
{
    try {
        QString tempdir = "/usr/share/deepin-log-viewer/DocxTemplate/1column.dfw";
        if (!QFile(tempdir).exists()) {
            qCWarning(logExport) << "export docx template is not exisits";
            return false;
        }

        DocxFactory::WordProcessingMerger &l_merger = DocxFactory::WordProcessingMerger::getInstance();
        l_merger.load(tempdir.toStdString());
        //往表头中添加表头描述，表头为第一行，数据则在下面
        for (int col = 0; col < labels.count(); ++col) {
            l_merger.setClipboardValue("tableRow", QString("column%1").arg(col + 1).toStdString(), labels.at(col).toStdString());
        }
        l_merger.paste("tableRow");
        //计算导出进度条最后一段的长度，因为最后写入文件那一段没有进度，所以预先留出一段进度
        int end = static_cast<int>(jList.count() * 0.1 > 5 ? jList.count() * 0.1 : 5);
        for (int row = 0; row < jList.count(); ++row) {
            //导出逻辑启动停止控制，外部把m_canRunning置false时停止运行，抛出异常处理
            if (!m_canRunning) {
                throw QString(stopStr);
            }
            LOG_MSG_DMESG message = jList.at(row);
            l_merger.setClipboardValue("tableRow", QString("column1").toStdString(), message.msg.toStdString());
            l_merger.paste("tableRow");
            //导出进度信号
            sigProgress(row + 1, jList.count() + end);
        }
        //保存，把拼好的xml写入文件中
        QString fileNamex = fileName + "x";

        QFile rsNameFile(fileName);
        if (rsNameFile.exists()) {
            rsNameFile.remove();
        }
        l_merger.save(fileNamex.toStdString());
        QFile(fileNamex).rename(fileName);

    } catch (const QString &ErrorStr) {
        //捕获到异常，导出失败，发出失败信号
        qCWarning(logExport) << "Export Stop" << ErrorStr;
        if (!m_canRunning) {
            Utils::checkAndDeleteDir(m_fileName);
        }

        emit sigResult(false);
        if (ErrorStr != stopStr) {
            emit sigError(QString("export error: %1").arg(ErrorStr));
        }
        return false;
    }
    //如果取消导出，则删除文件
    if (!m_canRunning) {
        Utils::checkAndDeleteDir(m_fileName);
    }
    //100%进度
    sigProgress(100, 100);
    //延时200ms再发送导出成功信号，关闭导出进度框，让100%的进度有时间显示
    Utils::sleep(200);
    //导出成功，如果此时被停止，则发出导出失败信号
    emit sigResult(m_canRunning);

    return m_canRunning;
}

bool LogExportThread::exportToDoc(const QString &fileName, const QList<LOG_MSG_AUDIT> &jList, const QStringList &labels)
{
    try {
        QString tempdir = "/usr/share/deepin-log-viewer/DocxTemplate/5column.dfw";
        if (!QFile(tempdir).exists()) {
            qCWarning(logExport) << "export docx template is not exisits";
            return  false;
        }

        DocxFactory:: WordProcessingMerger &l_merger = DocxFactory:: WordProcessingMerger::getInstance();
        l_merger.load(tempdir.toStdString());
        //往表头中添加表头描述，表头为第一行，数据则在下面
        for (int col = 0; col < labels.count(); ++col) {
            l_merger.setClipboardValue("tableRow", QString("column%1").arg(col + 1).toStdString(), labels.at(col).toStdString());

        }
        l_merger.paste("tableRow");
        //计算导出进度条最后一段的长度，因为最后写入文件那一段没有进度，所以预先留出一段进度
        int end = static_cast<int>(jList.count() * 0.1 > 5 ? jList.count() * 0.1 : 5);
        for (int row = 0; row < jList.count(); ++row) {
            //导出逻辑启动停止控制，外部把m_canRunning置false时停止运行，抛出异常处理
            if (!m_canRunning) {
                throw  QString(stopStr);
            }
            LOG_MSG_AUDIT message = jList.at(row);
            l_merger.setClipboardValue("tableRow", QString("column1").toStdString(), message.eventType.toStdString());
            l_merger.setClipboardValue("tableRow", QString("column2").toStdString(), message.dateTime.toStdString());
            l_merger.setClipboardValue("tableRow", QString("column3").toStdString(), message.processName.toStdString());
            l_merger.setClipboardValue("tableRow", QString("column4").toStdString(), message.status.toStdString());
            l_merger.setClipboardValue("tableRow", QString("column5").toStdString(), message.msg.toStdString());
            l_merger.paste("tableRow");
            //导出进度信号
            sigProgress(row + 1, jList.count() + end);
        }
        //保存，把拼好的xml写入文件中
        QString fileNamex = fileName + "x";

        QFile rsNameFile(fileName) ;
        if (rsNameFile.exists()) {
            rsNameFile.remove();
        }
        l_merger.save(fileNamex.toStdString());
        QFile(fileNamex).rename(fileName);

    } catch (const QString &ErrorStr) {
        //捕获到异常，导出失败，发出失败信号
        qCWarning(logExport) << "Export Stop" << ErrorStr;
        if (!m_canRunning) {
            Utils::checkAndDeleteDir(m_fileName);
        }

        emit sigResult(false);
        if (ErrorStr != stopStr) {
            emit sigError(QString("export error: %1").arg(ErrorStr));
        }
        return false;
    }
    //如果取消导出，则删除文件
    if (!m_canRunning) {
        Utils::checkAndDeleteDir(m_fileName);
    }
    //100%进度
    sigProgress(100, 100);
    //延时200ms再发送导出成功信号，关闭导出进度框，让100%的进度有时间显示
    Utils::sleep(200);
    //导出成功，如果此时被停止，则发出导出失败信号
    emit sigResult(m_canRunning);

    return m_canRunning;
}

/**
 * @brief LogExportThread::exportToHtml导出到日志html格式函数对QStandardItemModel数据类型的重载
 * @param fileName 导出文件路径全称
 * @param pModel 要导出的数据源，为QStandardItemModel
 * @param flag 导出的日志类型
 * @return 是否导出成功
 */
bool LogExportThread::exportToHtml(const QString &fileName, QStandardItemModel *pModel, LOG_FLAG flag)
{
    QFile html(fileName);
    //判断文件路径是否存在，不存在就返回错误
    if (!html.open(QIODevice::WriteOnly)) {
        emit sigResult(false);
        emit sigError(openErroStr);
        return false;
    }
    try {
        //判读啊model指针是为空，为空则不导出
        if (!pModel) {
            throw  QString("model is null");
        }
        //写网页头
        html.write("<!DOCTYPE html>\n");
        html.write("<html>\n");
        html.write("<body>\n");
        //写入表格标签
        html.write("<table border=\"1\">\n");
        // 写入表头
        html.write("<tr>");
        for (int i = 0; i < pModel->columnCount(); ++i) {
            QString labelInfo = QString("<td>%1</td>").arg(pModel->horizontalHeaderItem(i)->text());
            html.write(labelInfo.toUtf8().data());
        }
        html.write("</tr>");
        // 写入内容
        //日志类型为应用日志时
        if (flag == APP) {
            for (int row = 0; row < pModel->rowCount(); ++row) {
                //导出逻辑启动停止控制，外部把m_canRunning置false时停止运行，抛出异常处理
                if (!m_canRunning) {
                    throw  QString(stopStr);
                }
                //根据字段拼出每行的网页内容
                html.write("<tr>");

                QString info =
                    QString("<td>%1</td>").arg(pModel->item(row, 0)->data(Qt::UserRole + 6).toString());
                html.write(info.toUtf8().data());

                for (int col = 1; col < pModel->columnCount(); ++col) {
                    QString m_info = QString("<td>%1</td>").arg(pModel->item(row, col)->text());
                    htmlEscapeCovert(m_info);
                    html.write(m_info.toUtf8().data());
                }
                html.write("</tr>");
                //导出进度信号
                sigProgress(row + 1, pModel->rowCount());
            }
        } else {
            //日志类型为其他所有日志时
            for (int row = 0; row < pModel->rowCount(); ++row) {
                //导出逻辑启动停止控制，外部把m_canRunning置false时停止运行，抛出异常处理
                if (!m_canRunning) {
                    throw  QString(stopStr);
                }
                //根据字段拼出每行的网页内容
                html.write("<tr>");
                for (int col = 0; col < pModel->columnCount(); ++col) {
                    QString info = QString("<td>%1</td>").arg(pModel->item(row, col)->text());
                    htmlEscapeCovert(info);
                    html.write(info.toUtf8().data());
                }
                html.write("</tr>");
                //导出进度信号
                sigProgress(row + 1, pModel->rowCount());
            }
        }
        html.write("</table>\n");
        html.write("</body>\n");
        html.write("</html>\n");
    } catch (const QString &ErrorStr) {
        //捕获到异常，导出失败，发出失败信号
        qCWarning(logExport) << "Export Stop" << ErrorStr;
        html.close();
        emit sigResult(false);
        if (ErrorStr != stopStr) {
            emit sigError(QString("export error: %1").arg(ErrorStr));
        }
        return false;
    }
    html.close();
    //导出成功，如果此时被停止，则发出导出失败信号
    emit sigResult(m_canRunning);
    return m_canRunning;
}

bool LogExportThread::exportToHtml(const QString &fileName, const QList<QString> &jList, const QStringList &labels, LOG_FLAG flag)
{
    QFile html(fileName);
    //判断文件路径是否存在，不存在就返回错误
    if (!html.open(m_appendExport ? (QIODevice::Append | QIODevice::WriteOnly) : QIODevice::WriteOnly)) {
        emit sigResult(false);
        emit sigError(openErroStr);
        return false;
    }
    try {
        //写网页头
        html.write("<!DOCTYPE html>\n");
        html.write("<html>\n");
        html.write("<body>\n");
        //写入表格标签
        html.write("<table border=\"1\">\n");
        // 写入内容
        //日志类型为系统日志时
        if (flag == JOURNAL) {
            // 写入表头
            QString title = QString("<tr><td>") + QString(DApplication::translate("Table", "Level")) +
                            QString("</td><td>") + QString(DApplication::translate("Table", "Process")) +
                            QString("</td><td>") +
                            QString(DApplication::translate("Table", "Date and Time")) +
                            QString("</td><td>") + QString(DApplication::translate("Table", "Info")) +
                            QString("</td><td>") + QString(DApplication::translate("Table", "User")) +
                            QString("</td><td>") + QString(DApplication::translate("Table", "PID")) +
                            QString("</td></tr>");
            html.write(title.toUtf8().data());
            // 写入内容
            for (int i = 0; i < jList.count(); i++) {
                //导出逻辑启动停止控制，外部把m_canRunning置false时停止运行，抛出异常处理
                if (!m_canRunning) {
                    throw  QString(stopStr);
                }
                LOG_MSG_BASE jMsg;
                jMsg.fromJson(jList.at(i));
                htmlEscapeCovert(jMsg.msg);
                //根据字段拼出每行的网页内容
                QString info =
                    QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td><td>%6</td></tr>")
                    .arg(jMsg.level)
                    .arg(jMsg.daemonName)
                    .arg(jMsg.dateTime)
                    .arg(jMsg.msg)
                    .arg(jMsg.hostName)
                    .arg(jMsg.daemonId);
                html.write(info.toUtf8().data());
                //导出进度信号
                sigProgress(i + 1, jList.count());

            }
        } else if (flag == KERN) {
            // 写入表头
            html.write("<tr>");
            for (int i = 0; i < labels.count(); ++i) {
                QString labelInfo = QString("<td>%1</td>").arg(labels.value(i));
                html.write(labelInfo.toUtf8().data());
            }
            //根据字段拼出每行的网页内容
            html.write("</tr>");
            for (int row = 0; row < jList.count(); ++row) {
                if (!m_canRunning) {
                    throw  QString(stopStr);
                }
                LOG_MSG_BASE jMsg;
                jMsg.fromJson(jList.at(row));
                html.write("<tr>");
                QString info = QString("<td>%1</td>").arg(jMsg.dateTime);
                html.write(info.toUtf8().data());
                info = QString("<td>%1</td>").arg(jMsg.hostName);
                html.write(info.toUtf8().data());
                info = QString("<td>%1</td>").arg(jMsg.daemonName);
                html.write(info.toUtf8().data());
                info = QString("<td>%1</td>").arg(jMsg.msg);
                html.write(info.toUtf8().data());
                html.write("</tr>");
                sigProgress(row + 1, jList.count());
            }

        } else if (flag == Kwin) {
            // 写入表头
            html.write("<tr>");
            for (int i = 0; i < labels.count(); ++i) {
                QString labelInfo = QString("<td>%1</td>").arg(labels.value(i));
                html.write(labelInfo.toUtf8().data());
            }
            html.write("</tr>");
            // 写入内容
            for (int row = 0; row < jList.count(); ++row) {
                //导出逻辑启动停止控制，外部把m_canRunning置false时停止运行，抛出异常处理
                if (!m_canRunning) {
                    throw  QString(stopStr);
                }
                //根据字段拼出每行的网页内容
                LOG_MSG_BASE jMsg;
                jMsg.fromJson(jList.at(row));
                htmlEscapeCovert(jMsg.msg);
                html.write("<tr>");
                QString info = QString("<td>%1</td>").arg(jMsg.msg);
                html.write(info.toUtf8().data());
                html.write("</tr>");
                //导出进度信号
                sigProgress(row + 1, jList.count());
            }
        }

        html.write("</table>\n");
        html.write("</body>\n");
        html.write("</html>\n");
    } catch (const QString &ErrorStr) {
        //捕获到异常，导出失败，发出失败信号
        qCWarning(logExport) << "Export Stop" << ErrorStr;
        html.close();
        emit sigResult(false);
        if (ErrorStr != stopStr) {
            emit sigError(QString("export error: %1").arg(ErrorStr));
        }
        return false;
    }
    html.close();
    //导出成功，如果此时被停止，则发出导出失败信号
    emit sigResult(m_canRunning);
    return m_canRunning;
}

/**
 * @brief LogExportThread::exportToHtml 导出到日志html格式函数，对LOG_MSG_JOURNAL数据类型的重载（指系统日志和内核日志）
 * @param fileName 导出文件路径全称
 * @param jList 要导出的数据源 数据类型为LOG_MSG_JOURNAL
 * @param labels 表头字符串
 * @param flag  导出的日志类型
 * @return 是否导出成功
 */
bool LogExportThread::exportToHtml(const QString &fileName, const QList<LOG_MSG_JOURNAL> &jList,  const QStringList &labels, LOG_FLAG flag)
{
    QFile html(fileName);
    //判断文件路径是否存在，不存在就返回错误
    if (!html.open(QIODevice::WriteOnly)) {
        emit sigResult(false);
        emit sigError(openErroStr);
        return false;
    }
    try {
        //写网页头
        html.write("<!DOCTYPE html>\n");
        html.write("<html>\n");
        html.write("<body>\n");
        //写入表格标签
        html.write("<table border=\"1\">\n");
        // 写入内容
        //日志类型为系统日志时
        if (flag == JOURNAL) {
            // 写入表头
            QString title = QString("<tr><td>") + QString(DApplication::translate("Table", "Level")) +
                            QString("</td><td>") + QString(DApplication::translate("Table", "Process")) +
                            QString("</td><td>") +
                            QString(DApplication::translate("Table", "Date and Time")) +
                            QString("</td><td>") + QString(DApplication::translate("Table", "Info")) +
                            QString("</td><td>") + QString(DApplication::translate("Table", "User")) +
                            QString("</td><td>") + QString(DApplication::translate("Table", "PID")) +
                            QString("</td></tr>");
            html.write(title.toUtf8().data());
            // 写入内容
            for (int i = 0; i < jList.count(); i++) {
                //导出逻辑启动停止控制，外部把m_canRunning置false时停止运行，抛出异常处理
                if (!m_canRunning) {
                    throw  QString(stopStr);
                }
                LOG_MSG_JOURNAL jMsg = jList.at(i);
                htmlEscapeCovert(jMsg.msg);
                //根据字段拼出每行的网页内容
                QString info =
                    QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td><td>%6</td></tr>")
                    .arg(jMsg.level)
                    .arg(jMsg.daemonName)
                    .arg(jMsg.dateTime)
                    .arg(jMsg.msg)
                    .arg(jMsg.hostName)
                    .arg(jMsg.daemonId);
                html.write(info.toUtf8().data());
                //导出进度信号
                sigProgress(i + 1, jList.count());

            }
        } else if (flag == KERN) {
            //日志类型为内核日志时
            // 写入表头
            html.write("<tr>");
            for (int i = 0; i < labels.count(); ++i) {
                QString labelInfo = QString("<td>%1</td>").arg(labels.value(i));
                html.write(labelInfo.toUtf8().data());
            }
            //根据字段拼出每行的网页内容
            html.write("</tr>");
            for (int row = 0; row < jList.count(); ++row) {
                if (!m_canRunning) {
                    throw  QString(stopStr);
                }
                LOG_MSG_JOURNAL jMsg = jList.at(row);
                html.write("<tr>");
                QString info = QString("<td>%1</td>").arg(jMsg.dateTime);
                html.write(info.toUtf8().data());
                info = QString("<td>%1</td>").arg(jMsg.hostName);
                html.write(info.toUtf8().data());
                info = QString("<td>%1</td>").arg(jMsg.daemonName);
                html.write(info.toUtf8().data());
                info = QString("<td>%1</td>").arg(jMsg.msg);
                html.write(info.toUtf8().data());
                html.write("</tr>");
                sigProgress(row + 1, jList.count());
            }

        }

        html.write("</table>\n");
        html.write("</body>\n");
        html.write("</html>\n");
    } catch (const QString &ErrorStr) {
        //捕获到异常，导出失败，发出失败信号
        qCWarning(logExport) << "Export Stop" << ErrorStr;
        html.close();
        emit sigResult(false);
        if (ErrorStr != stopStr) {
            emit sigError(QString("export error: %1").arg(ErrorStr));
        }
        return false;
    }
    html.close();
    //导出成功，如果此时被停止，则发出导出失败信号
    emit sigResult(m_canRunning);
    return m_canRunning;
}

/**
 * @brief LogExportThread::exportToHtml导出到日志html格式函数，对LOG_MSG_APPLICATOIN数据类型的重载（指应用日志）
 * @param fileName 导出文件路径全称
 * @param jList 要导出的数据源 数据类型为LOG_MSG_APPLICATOIN
 * @param labels 表头字符串
 * @param iAppName 导出的应用日志的应用名称
 * @return 是否导出成功
 */
bool LogExportThread::exportToHtml(const QString &fileName, const QList<LOG_MSG_APPLICATOIN> &jList, const QStringList &labels, QString &iAppName)
{
    //判断文件路径是否存在，不存在就返回错误
    QFile html(fileName);
    if (!html.open(QIODevice::WriteOnly)) {
        emit sigResult(false);
        emit sigError(openErroStr);
        return false;
    }
    try {
        //写网页头
        html.write("<!DOCTYPE html>\n");
        html.write("<html>\n");
        html.write("<body>\n");
        //写入表格标签
        html.write("<table border=\"1\">\n");
        // 写入表头
        html.write("<tr>");
        for (int i = 0; i < labels.count(); ++i) {
            QString labelInfo = QString("<td>%1</td>").arg(labels.value(i));
            html.write(labelInfo.toUtf8().data());
        }
        html.write("</tr>");
        // 写入内容
        for (int row = 0; row < jList.count(); ++row) {
            //导出逻辑启动停止控制，外部把m_canRunning置false时停止运行，抛出异常处理
            if (!m_canRunning) {
                throw  QString(stopStr);
            }
            //根据字段拼出每行的网页内容
            LOG_MSG_APPLICATOIN jMsg = jList.at(row);
            htmlEscapeCovert(jMsg.msg);
            html.write("<tr>");
            QString info = QString("<td>%1</td>").arg(strTranslate(jMsg.level));
            html.write(info.toUtf8().data());
            info = QString("<td>%1</td>").arg(jMsg.dateTime);
            html.write(info.toUtf8().data());
            info = QString("<td>%1</td>").arg(iAppName);
            html.write(info.toUtf8().data());
            info = QString("<td>%1</td>").arg(jMsg.msg);
            html.write(info.toUtf8().data());
            html.write("</tr>");
            //导出进度信号
            sigProgress(row + 1, jList.count());
        }

        html.write("</table>\n");
        html.write("</body>\n");
        html.write("</html>\n");
    } catch (const QString &ErrorStr) {
        //捕获到异常，导出失败，发出失败信号
        qCWarning(logExport) << "Export Stop" << ErrorStr;
        html.close();
        emit sigResult(false);
        if (ErrorStr != stopStr) {
            emit sigError(QString("export error: %1").arg(ErrorStr));
        }
        return false;
    }
    html.close();
    //导出成功，如果此时被停止，则发出导出失败信号
    emit sigResult(m_canRunning);
    return m_canRunning;
}

/**
 * @brief LogExportThread::exportToHtml 导出到日志html格式配置函数，对LOG_MSG_DPKG数据类型的重载（指dpkg日志）
 * @param fileName 导出文件路径全称
 * @param jList  要导出的数据源 LOG_MSG_DPKG
 * @param labels 表头字符串
 * @return 是否导出成功
 */
bool LogExportThread::exportToHtml(const QString &fileName, const QList<LOG_MSG_DPKG> &jList, const QStringList &labels)
{
    //判断文件路径是否存在，不存在就返回错误
    QFile html(fileName);
    if (!html.open(QIODevice::WriteOnly)) {
        emit sigResult(false);
        emit sigError(openErroStr);
        return false;
    }
    try {
        //写网页头
        html.write("<!DOCTYPE html>\n");
        html.write("<html>\n");
        html.write("<body>\n");
        //写入表格标签
        html.write("<table border=\"1\">\n");
        // 写入表头
        html.write("<tr>");
        for (int i = 0; i < labels.count(); ++i) {
            QString labelInfo = QString("<td>%1</td>").arg(labels.value(i));
            html.write(labelInfo.toUtf8().data());
        }
        html.write("</tr>");
        // 写入内容
        for (int row = 0; row < jList.count(); ++row) {
            //导出逻辑启动停止控制，外部把m_canRunning置false时停止运行，抛出异常处理
            if (!m_canRunning) {
                throw  QString(stopStr);
            }
            //根据字段拼出每行的网页内容
            LOG_MSG_DPKG jMsg = jList.at(row);
            htmlEscapeCovert(jMsg.msg);
            html.write("<tr>");
            QString info = QString("<td>%1</td>").arg(jMsg.dateTime);
            html.write(info.toUtf8().data());
            info = QString("<td>%1</td>").arg(jMsg.msg);
            html.write(info.toUtf8().data());
            info = QString("<td>%1</td>").arg(jMsg.action);
            html.write(info.toUtf8().data());
            html.write("</tr>");
            //导出进度信号
            sigProgress(row + 1, jList.count());
        }

        html.write("</table>\n");
        html.write("</body>\n");
        html.write("</html>\n");
    } catch (const QString &ErrorStr) {
        //捕获到异常，导出失败，发出失败信号
        qCWarning(logExport) << "Export Stop" << ErrorStr;
        html.close();
        emit sigResult(false);
        if (ErrorStr != stopStr) {
            emit sigError(QString("export error: %1").arg(ErrorStr));
        }
        return false;
    }
    html.close();
    //导出成功，如果此时被停止，则发出导出失败信号
    emit sigResult(m_canRunning);
    return m_canRunning;
}

/**
 * @brief LogExportThread::exportToHtml导出到日志html格式函数，对LOG_MSG_BOOT数据类型的重载（指启动日志）
 * @param fileName 导出文件路径全称
 * @param jList  要导出的数据源 LOG_MSG_BOOT
 * @param labels 表头字符串
 * @return 是否导出成功
 */
bool LogExportThread::exportToHtml(const QString &fileName, const QList<LOG_MSG_BOOT> &jList, const QStringList &labels)
{
    //判断文件路径是否存在，不存在就返回错误
    QFile html(fileName);
    if (!html.open(QIODevice::WriteOnly)) {
        emit sigResult(false);
        emit sigError(openErroStr);
        return false;
    }
    try {
        //写网页头
        html.write("<!DOCTYPE html>\n");
        html.write("<html>\n");
        html.write("<body>\n");
        //写入表格标签
        html.write("<table border=\"1\">\n");
        // 写入表头
        html.write("<tr>");
        for (int i = 0; i < labels.count(); ++i) {
            QString labelInfo = QString("<td>%1</td>").arg(labels.value(i));
            html.write(labelInfo.toUtf8().data());
        }
        html.write("</tr>");
        // 写入内容
        for (int row = 0; row < jList.count(); ++row) {
            //导出逻辑启动停止控制，外部把m_canRunning置false时停止运行，抛出异常处理
            if (!m_canRunning) {
                throw  QString(stopStr);
            }
            //根据字段拼出每行的网页内容
            LOG_MSG_BOOT jMsg = jList.at(row);
            htmlEscapeCovert(jMsg.msg);
            html.write("<tr>");
            QString info = QString("<td>%1</td>").arg(jMsg.status);
            html.write(info.toUtf8().data());
            info = QString("<td>%1</td>").arg(jMsg.msg);
            html.write(info.toUtf8().data());
            html.write("</tr>");
            //导出进度信号
            sigProgress(row + 1, jList.count());
        }

        html.write("</table>\n");
        html.write("</body>\n");
        html.write("</html>\n");
    } catch (const QString &ErrorStr) {
        //捕获到异常，导出失败，发出失败信号
        qCWarning(logExport) << "Export Stop" << ErrorStr;
        html.close();
        emit sigResult(false);
        if (ErrorStr != stopStr) {
            emit sigError(QString("export error: %1").arg(ErrorStr));
        }
        return false;
    }
    html.close();
    //导出成功，如果此时被停止，则发出导出失败信号
    emit sigResult(m_canRunning);
    return m_canRunning;
}

/**
 * @brief LogExportThread::exportToHtml 导出到日志html格式函数，对LOG_MSG_XORG数据类型的重载（指xorg日志）
 * @param fileName 导出文件路径全称
 * @param jList 要导出的数据源 LOG_MSG_XORG
 * @param labels 表头字符串
 * @return 是否导出成功
 */
bool LogExportThread::exportToHtml(const QString &fileName, const QList<LOG_MSG_XORG> &jList, const QStringList &labels)
{
    //判断文件路径是否存在，不存在就返回错误
    QFile html(fileName);
    if (!html.open(QIODevice::WriteOnly)) {
        emit sigResult(false);
        emit sigError(openErroStr);
        return false;
    }
    try {
        //写网页头
        html.write("<!DOCTYPE html>\n");
        html.write("<html>\n");
        html.write("<body>\n");
        //写入表格标签
        html.write("<table border=\"1\">\n");
        // 写入表头
        html.write("<tr>");
        for (int i = 0; i < labels.count(); ++i) {
            QString labelInfo = QString("<td>%1</td>").arg(labels.value(i));
            html.write(labelInfo.toUtf8().data());
        }
        html.write("</tr>");
        // 写入内容
        for (int row = 0; row < jList.count(); ++row) {
            //导出逻辑启动停止控制，外部把m_canRunning置false时停止运行，抛出异常处理
            if (!m_canRunning) {
                throw  QString(stopStr);
            }
            //根据字段拼出每行的网页内容
            LOG_MSG_XORG jMsg = jList.at(row);
            htmlEscapeCovert(jMsg.msg);
            html.write("<tr>");
            QString info = QString("<td>%1</td>").arg(jMsg.offset);
            html.write(info.toUtf8().data());
            info = QString("<td>%1</td>").arg(jMsg.msg);
            html.write(info.toUtf8().data());
            html.write("</tr>");
            //导出进度信号
            sigProgress(row + 1, jList.count());
        }

        html.write("</table>\n");
        html.write("</body>\n");
        html.write("</html>\n");
    } catch (const QString &ErrorStr) {
        //捕获到异常，导出失败，发出失败信号
        qCWarning(logExport) << "Export Stop" << ErrorStr;
        html.close();
        emit sigResult(false);
        if (ErrorStr != stopStr) {
            emit sigError(QString("export error: %1").arg(ErrorStr));
        }
        return false;
    }
    html.close();
    //导出成功，如果此时被停止，则发出导出失败信号
    emit sigResult(m_canRunning);
    return m_canRunning;
}

/**
 * @brief LogExportThread::exportToHtml 导出到日志html格式配置函数，对LOG_MSG_NORMAL数据类型的重载（指开关机日志）
 * @param fileName 导出文件路径全称
 * @param jList 要导出的数据源 LOG_MSG_NORMAL
 * @param labels 表头字符串
 * @return 是否导出成功
 */
bool LogExportThread::exportToHtml(const QString &fileName, const QList<LOG_MSG_NORMAL> &jList, const QStringList &labels)
{
    //判断文件路径是否存在，不存在就返回错误
    QFile html(fileName);
    if (!html.open(QIODevice::WriteOnly)) {
        emit sigResult(false);
        emit sigError(openErroStr);
        return false;
    }
    try {
        //写网页头
        html.write("<!DOCTYPE html>\n");
        html.write("<html>\n");
        html.write("<body>\n");
        //写入表格标签
        html.write("<table border=\"1\">\n");
        // 写入表头
        html.write("<tr>");
        for (int i = 0; i < labels.count(); ++i) {
            QString labelInfo = QString("<td>%1</td>").arg(labels.value(i));
            html.write(labelInfo.toUtf8().data());
        }
        html.write("</tr>");
        // 写入内容
        for (int row = 0; row < jList.count(); ++row) {
            //导出逻辑启动停止控制，外部把m_canRunning置false时停止运行，抛出异常处理
            if (!m_canRunning) {
                throw  QString(stopStr);
            }
            //根据字段拼出每行的网页内容
            LOG_MSG_NORMAL jMsg = jList.at(row);
            htmlEscapeCovert(jMsg.msg);
            html.write("<tr>");
            QString info = QString("<td>%1</td>").arg(jMsg.eventType);
            html.write(info.toUtf8().data());
            info = QString("<td>%1</td>").arg(jMsg.userName);
            html.write(info.toUtf8().data());
            info = QString("<td>%1</td>").arg(jMsg.dateTime);
            html.write(info.toUtf8().data());
            info = QString("<td>%1</td>").arg(jMsg.msg);
            html.write(info.toUtf8().data());
            html.write("</tr>");
            //导出进度信号
            sigProgress(row + 1, jList.count());
        }

        html.write("</table>\n");
        html.write("</body>\n");
        html.write("</html>\n");
    } catch (const QString &ErrorStr) {
        //捕获到异常，导出失败，发出失败信号
        qCWarning(logExport) << "Export Stop" << ErrorStr;
        html.close();
        emit sigResult(false);
        if (ErrorStr != stopStr) {
            emit sigError(QString("export error: %1").arg(ErrorStr));
        }
        return false;
    }
    html.close();
    //导出成功，如果此时被停止，则发出导出失败信号
    emit sigResult(m_canRunning);
    return m_canRunning;
}

/**
 * @brief LogExportThread::exportToHtml导出到日志html格式函数，对LOG_MSG_KWIN数据类型的重载（指klu上的kwin日志）
 * @param fileName 导出文件路径全称
 * @param jList 要导出的数据源 LOG_MSG_KWIN
 * @param labels 表头字符串
 * @return 是否导出成功
 */
bool LogExportThread::exportToHtml(const QString &fileName, const QList<LOG_MSG_KWIN> &jList, const QStringList &labels)
{
    //判断文件路径是否存在，不存在就返回错误
    QFile html(fileName);
    if (!html.open(QIODevice::WriteOnly)) {
        emit sigResult(false);
        emit sigError(openErroStr);
        return false;
    }
    try {
        //写网页头
        html.write("<!DOCTYPE html>\n");
        html.write("<html>\n");
        html.write("<body>\n");
        //写入表格标签
        html.write("<table border=\"1\">\n");
        // 写入表头
        html.write("<tr>");
        for (int i = 0; i < labels.count(); ++i) {
            QString labelInfo = QString("<td>%1</td>").arg(labels.value(i));
            html.write(labelInfo.toUtf8().data());
        }
        html.write("</tr>");
        // 写入内容
        for (int row = 0; row < jList.count(); ++row) {
            //导出逻辑启动停止控制，外部把m_canRunning置false时停止运行，抛出异常处理
            if (!m_canRunning) {
                throw  QString(stopStr);
            }
            //根据字段拼出每行的网页内容
            LOG_MSG_KWIN jMsg = jList.at(row);
            htmlEscapeCovert(jMsg.msg);
            html.write("<tr>");
            QString info = QString("<td>%1</td>").arg(jMsg.msg);
            html.write(info.toUtf8().data());
            html.write("</tr>");
            //导出进度信号
            sigProgress(row + 1, jList.count());
        }

        html.write("</table>\n");
        html.write("</body>\n");
        html.write("</html>\n");
    } catch (const QString &ErrorStr) {
        //捕获到异常，导出失败，发出失败信号
        qCWarning(logExport) << "Export Stop" << ErrorStr;
        html.close();
        emit sigResult(false);
        if (ErrorStr != stopStr) {
            emit sigError(QString("export error: %1").arg(ErrorStr));
        }
        return false;
    }
    html.close();
    //导出成功，如果此时被停止，则发出导出失败信号
    emit sigResult(m_canRunning);
    return m_canRunning;
}

bool LogExportThread::exportToHtml(const QString &fileName, const QList<LOG_MSG_DNF> &jList, const QStringList &labels)
{
    //判断文件路径是否存在，不存在就返回错误
    QFile html(fileName);
    if (!html.open(QIODevice::WriteOnly)) {
        emit sigResult(false);
        emit sigError(openErroStr);
        return false;
    }
    try {
        //写网页头
        html.write("<!DOCTYPE html>\n");
        html.write("<html>\n");
        html.write("<body>\n");
        //写入表格标签
        html.write("<table border=\"1\">\n");
        // 写入表头
        html.write("<tr>");
        for (int i = 0; i < labels.count(); ++i) {
            QString labelInfo = QString("<td>%1</td>").arg(labels.value(i));
            html.write(labelInfo.toUtf8().data());
        }
        html.write("</tr>");
        // 写入内容
        for (int row = 0; row < jList.count(); ++row) {
            //导出逻辑启动停止控制，外部把m_canRunning置false时停止运行，抛出异常处理
            if (!m_canRunning) {
                throw QString(stopStr);
            }
            //根据字段拼出每行的网页内容
            LOG_MSG_DNF jMsg = jList.at(row);
            htmlEscapeCovert(jMsg.msg);
            html.write("<tr>");
            QString info = QString("<td>%1</td>").arg(jMsg.level);
            html.write(info.toUtf8().data());
            info = QString("<td>%1</td>").arg(jMsg.dateTime);
            html.write(info.toUtf8().data());
            //此style为使元素内\n换行符起效
            info = QString("<td style='white-space: pre-line;'>%1</td>").arg(jMsg.msg);
            html.write(info.toUtf8().data());
            html.write("</tr>");
            //导出进度信号
            sigProgress(row + 1, jList.count());
        }

        html.write("</table>\n");
        html.write("</body>\n");
        html.write("</html>\n");
    } catch (const QString &ErrorStr) {
        //捕获到异常，导出失败，发出失败信号
        qCWarning(logExport) << "Export Stop" << ErrorStr;
        html.close();
        emit sigResult(false);
        if (ErrorStr != stopStr) {
            emit sigError(QString("export error: %1").arg(ErrorStr));
        }
        return false;
    }
    html.close();
    //导出成功，如果此时被停止，则发出导出失败信号
    emit sigResult(m_canRunning);
    return m_canRunning;
}

bool LogExportThread::exportToHtml(const QString &fileName, const QList<LOG_MSG_DMESG> &jList, const QStringList &labels)
{
    //判断文件路径是否存在，不存在就返回错误
    QFile html(fileName);
    if (!html.open(QIODevice::WriteOnly)) {
        emit sigResult(false);
        emit sigError(openErroStr);
        return false;
    }
    try {
        //写网页头
        html.write("<!DOCTYPE html>\n");
        html.write("<html>\n");
        html.write("<body>\n");
        //写入表格标签
        html.write("<table border=\"1\">\n");
        // 写入表头
        html.write("<tr>");
        for (int i = 0; i < labels.count(); ++i) {
            QString labelInfo = QString("<td>%1</td>").arg(labels.value(i));
            html.write(labelInfo.toUtf8().data());
        }
        html.write("</tr>");
        // 写入内容
        for (int row = 0; row < jList.count(); ++row) {
            //导出逻辑启动停止控制，外部把m_canRunning置false时停止运行，抛出异常处理
            if (!m_canRunning) {
                throw QString(stopStr);
            }
            //根据字段拼出每行的网页内容
            LOG_MSG_DMESG jMsg = jList.at(row);
            htmlEscapeCovert(jMsg.msg);
            html.write("<tr>");
            QString info = QString("<td>%1</td>").arg(jMsg.level);
            html.write(info.toUtf8().data());
            info = QString("<td>%1</td>").arg(jMsg.dateTime);
            html.write(info.toUtf8().data());
            //此style为使元素内\n换行符起效
            info = QString("<td style='white-space: pre-line;'>%1</td>").arg(jMsg.msg);
            html.write(info.toUtf8().data());
            html.write("</tr>");
            //导出进度信号
            sigProgress(row + 1, jList.count());
        }

        html.write("</table>\n");
        html.write("</body>\n");
        html.write("</html>\n");
    } catch (const QString &ErrorStr) {
        //捕获到异常，导出失败，发出失败信号
        qCWarning(logExport) << "Export Stop" << ErrorStr;
        html.close();
        emit sigResult(false);
        if (ErrorStr != stopStr) {
            emit sigError(QString("export error: %1").arg(ErrorStr));
        }
        return false;
    }
    html.close();
    //导出成功，如果此时被停止，则发出导出失败信号
    emit sigResult(m_canRunning);
    return m_canRunning;
}

bool LogExportThread::exportToHtml(const QString &fileName, const QList<LOG_MSG_AUDIT> &jList, const QStringList &labels)
{
    //判断文件路径是否存在，不存在就返回错误
    QFile html(fileName);
    if (!html.open(QIODevice::WriteOnly)) {
        emit sigResult(false);
        emit sigError(openErroStr);
        return false;
    }
    try {
        //写网页头
        html.write("<!DOCTYPE html>\n");
        html.write("<html>\n");
        html.write("<body>\n");
        //写入表格标签
        html.write("<table border=\"1\">\n");
        // 写入表头
        html.write("<tr>");
        for (int i = 0; i < labels.count(); ++i) {
            QString labelInfo = QString("<td>%1</td>").arg(labels.value(i));
            html.write(labelInfo.toUtf8().data());
        }
        html.write("</tr>");
        // 写入内容
        for (int row = 0; row < jList.count(); ++row) {
            //导出逻辑启动停止控制，外部把m_canRunning置false时停止运行，抛出异常处理
            if (!m_canRunning) {
                throw  QString(stopStr);
            }
            //根据字段拼出每行的网页内容
            LOG_MSG_AUDIT jMsg = jList.at(row);
            htmlEscapeCovert(jMsg.msg);
            html.write("<tr>");
            QString info = QString("<td>%1</td>").arg(jMsg.eventType);
            html.write(info.toUtf8().data());
            info = QString("<td>%1</td>").arg(jMsg.dateTime);
            html.write(info.toUtf8().data());
            info = QString("<td>%1</td>").arg(jMsg.processName);
            html.write(info.toUtf8().data());
            info = QString("<td>%1</td>").arg(jMsg.status);
            html.write(info.toUtf8().data());
            info = QString("<td>%1</td>").arg(jMsg.msg);
            html.write(info.toUtf8().data());
            html.write("</tr>");
            //导出进度信号
            sigProgress(row + 1, jList.count());
        }

        html.write("</table>\n");
        html.write("</body>\n");
        html.write("</html>\n");
    } catch (const QString &ErrorStr) {
        //捕获到异常，导出失败，发出失败信号
        qCWarning(logExport) << "Export Stop" << ErrorStr;
        html.close();
        emit sigResult(false);
        if (ErrorStr != stopStr) {
            emit sigError(QString("export error: %1").arg(ErrorStr));
        }
        return false;
    }
    html.close();
    //导出成功，如果此时被停止，则发出导出失败信号
    emit sigResult(m_canRunning);
    return m_canRunning;
}

bool LogExportThread::exportToXls(const QString &fileName, const QList<QString> &jList, const QStringList &labels, LOG_FLAG iFlag)
{
    try {
        auto currentXlsRow = 0;
        lxw_workbook  *workbook  = workbook_new(fileName.toStdString().c_str());
        lxw_worksheet *worksheet = workbook_add_worksheet(workbook, nullptr);
        lxw_format *format = workbook_add_format(workbook);
        format_set_bold(format);
        for (int col = 0; col < labels.count(); ++col) {
            worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col), labels.at(col).toStdString().c_str(), format);
        }
        ++currentXlsRow;
        int end = static_cast<int>(jList.count() * 0.1 > 5 ? jList.count() * 0.1 : 5);

        for (int row = 0; row < jList.count() ; ++row) {
            if (!m_canRunning) {
                throw  QString(stopStr);
            }
            LOG_MSG_BASE message;
            message.fromJson(jList.at(row));
            int col = 0;

            if (iFlag == JOURNAL) {
                worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col++), message.level.toStdString().c_str(), nullptr);
                worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col++), message.daemonName.toStdString().c_str(), nullptr);
                worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col++), message.dateTime.toStdString().c_str(), nullptr);
                worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col++), message.msg.toStdString().c_str(), nullptr);
                worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col++), message.hostName.toStdString().c_str(), nullptr);
                worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col++), message.daemonId.toStdString().c_str(), nullptr);
            } else if (iFlag == KERN) {
                worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col++), message.dateTime.toStdString().c_str(), nullptr);
                worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col++), message.hostName.toStdString().c_str(), nullptr);
                worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col++), message.daemonName.toStdString().c_str(), nullptr);
                worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col++), message.msg.toStdString().c_str(), nullptr);
            } else if (iFlag == Kwin) {
                worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col++), message.msg.toStdString().c_str(), nullptr);
            }

            ++currentXlsRow;
            sigProgress(row + 1, jList.count() + end);
        }


        workbook_close(workbook);
        malloc_trim(0);
        sigProgress(100, 100);
    } catch (const QString &ErrorStr) {
        qCWarning(logExport) << "Export Stop" << ErrorStr;
        emit sigResult(false);
        if (ErrorStr != stopStr) {
            emit sigError(QString("export error: %1").arg(ErrorStr));
        }
        return false;
    }
    emit sigResult(m_canRunning);
    return m_canRunning;

}

/**
 * @brief LogExportThread::exportToXls导出到日志xlsx格式函数，对LOG_MSG_JOURNAL数据类型的重载（指系统日志和内核日志）
 * @param fileName 导出文件路径全称
 * @param jList 要导出的数据源 数据类型为LOG_MSG_JOURNAL
 * @param labels 表头字符串
 * @param flag  导出的日志类型
 * @return 是否导出成功
 */
bool LogExportThread::exportToXls(const QString &fileName, const QList<LOG_MSG_JOURNAL> &jList,
                                  const QStringList &labels, LOG_FLAG iFlag)
{
    try {
        auto currentXlsRow = 0;
        lxw_workbook  *workbook  = workbook_new(fileName.toStdString().c_str());
        lxw_worksheet *worksheet = workbook_add_worksheet(workbook, nullptr);
        lxw_format *format = workbook_add_format(workbook);
        format_set_bold(format);
        for (int col = 0; col < labels.count(); ++col) {
            worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col), labels.at(col).toStdString().c_str(), format);
        }
        ++currentXlsRow;
        int end = static_cast<int>(jList.count() * 0.1 > 5 ? jList.count() * 0.1 : 5);

        for (int row = 0; row < jList.count() ; ++row) {
            if (!m_canRunning) {
                throw  QString(stopStr);
            }
            LOG_MSG_JOURNAL message = jList.at(row);
            int col = 0;

            if (iFlag == JOURNAL) {
                worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col++), message.level.toStdString().c_str(), nullptr);
                worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col++), message.daemonName.toStdString().c_str(), nullptr);
                worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col++), message.dateTime.toStdString().c_str(), nullptr);
                worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col++), message.msg.toStdString().c_str(), nullptr);
                worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col++), message.hostName.toStdString().c_str(), nullptr);
                worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col++), message.daemonId.toStdString().c_str(), nullptr);
            } else if (iFlag == KERN) {
                worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col++), message.dateTime.toStdString().c_str(), nullptr);
                worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col++), message.hostName.toStdString().c_str(), nullptr);
                worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col++), message.daemonName.toStdString().c_str(), nullptr);
                worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col++), message.msg.toStdString().c_str(), nullptr);
            }

            ++currentXlsRow;
            sigProgress(row + 1, jList.count() + end);
        }


        workbook_close(workbook);
        malloc_trim(0);
        sigProgress(100, 100);
    } catch (const QString &ErrorStr) {
        qCWarning(logExport) << "Export Stop" << ErrorStr;
        emit sigResult(false);
        if (ErrorStr != stopStr) {
            emit sigError(QString("export error: %1").arg(ErrorStr));
        }
        return false;
    }
    emit sigResult(m_canRunning);
    return m_canRunning;

}

/**
 * @brief LogExportThread::exportToXls导出到日志xlsx格式函数，对LOG_MSG_APPLICATOIN数据类型的重载（指应用日志）
 * @param fileName 导出文件路径全称
 * @param jList 要导出的数据源 数据类型为LOG_MSG_APPLICATOIN
 * @param labels 表头字符串
 * @param iAppName 导出的应用日志的应用名称
 * @return 是否导出成功
 */
bool LogExportThread::exportToXls(const QString &fileName, const QList<LOG_MSG_APPLICATOIN> &jList, const QStringList &labels, QString &iAppName)
{
    try {
        auto currentXlsRow = 0;
        lxw_workbook  *workbook  = workbook_new(fileName.toStdString().c_str());
        lxw_worksheet *worksheet = workbook_add_worksheet(workbook, nullptr);
        lxw_format *format = workbook_add_format(workbook);
        format_set_bold(format);
        for (int col = 0; col < labels.count(); ++col) {
            worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col), labels.at(col).toStdString().c_str(), format);
        }
        ++currentXlsRow;
        int end = static_cast<int>(jList.count() * 0.1 > 5 ? jList.count() * 0.1 : 5);

        for (int row = 0; row < jList.count() ; ++row) {
            if (!m_canRunning) {
                throw  QString(stopStr);
            }
            LOG_MSG_APPLICATOIN message = jList.at(row);
            int col = 0;
            worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col++), strTranslate(message.level).toStdString().c_str(), nullptr);
            worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col++), message.dateTime.toStdString().c_str(), nullptr);
            worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col++), iAppName.toStdString().c_str(), nullptr);
            worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col++), message.msg.toStdString().c_str(), nullptr);
            ++currentXlsRow;
            sigProgress(row + 1, jList.count() + end);
        }


        workbook_close(workbook);
        malloc_trim(0);
        sigProgress(100, 100);
    } catch (const QString &ErrorStr) {
        qCWarning(logExport) << "Export Stop" << ErrorStr;
        emit sigResult(false);
        if (ErrorStr != stopStr) {
            emit sigError(QString("export error: %1").arg(ErrorStr));
        }
        return false;
    }
    emit sigResult(m_canRunning);
    return m_canRunning;

}

/**
 * @brief LogExportThread::exportToXls 导出到日志xlsx格式函数，对LOG_MSG_DPKG数据类型的重载（指dpkg日志）
 * @param fileName 导出文件路径全称
 * @param jList  要导出的数据源 LOG_MSG_DPKG
 * @param labels 表头字符串
 * @return 是否导出成功
 */
bool LogExportThread::exportToXls(const QString &fileName, const QList<LOG_MSG_DPKG> &jList, const QStringList &labels)
{
    try {
        auto currentXlsRow = 0;
        lxw_workbook  *workbook  = workbook_new(fileName.toStdString().c_str());
        lxw_worksheet *worksheet = workbook_add_worksheet(workbook, nullptr);
        lxw_format *format = workbook_add_format(workbook);
        format_set_bold(format);
        for (int col = 0; col < labels.count(); ++col) {
            worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col), labels.at(col).toStdString().c_str(), format);
        }
        ++currentXlsRow;
        int end = static_cast<int>(jList.count() * 0.1 > 5 ? jList.count() * 0.1 : 5);

        for (int row = 0; row < jList.count() ; ++row) {
            if (!m_canRunning) {
                throw  QString(stopStr);
            }
            LOG_MSG_DPKG message = jList.at(row);
            int col = 0;
            worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col++), message.dateTime.toStdString().c_str(), nullptr);
            worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col++), message.msg.toStdString().c_str(), nullptr);
            worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col++), message.action.toStdString().c_str(), nullptr);
            ++currentXlsRow;
            sigProgress(row + 1, jList.count() + end);
        }


        workbook_close(workbook);
        malloc_trim(0);
        sigProgress(100, 100);
    } catch (const QString &ErrorStr) {
        qCWarning(logExport) << "Export Stop" << ErrorStr;
        emit sigResult(false);
        if (ErrorStr != stopStr) {
            emit sigError(QString("export error: %1").arg(ErrorStr));
        }
        return false;
    }
    emit sigResult(m_canRunning);
    return m_canRunning;
}

/**
 * @brief LogExportThread::exportToXls导出到日志xlsx格式函数，对LOG_MSG_BOOT数据类型的重载（指启动日志）
 * @param fileName 导出文件路径全称
 * @param jList  要导出的数据源 LOG_MSG_DPKG
 * @param labels 表头字符串
 * @return 是否导出成功
 */
bool LogExportThread::exportToXls(const QString &fileName, const QList<LOG_MSG_BOOT> &jList, const QStringList &labels)
{
    try {
        auto currentXlsRow = 0;
        lxw_workbook  *workbook  = workbook_new(fileName.toStdString().c_str());
        lxw_worksheet *worksheet = workbook_add_worksheet(workbook, nullptr);
        lxw_format *format = workbook_add_format(workbook);
        format_set_bold(format);
        for (int col = 0; col < labels.count(); ++col) {
            worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col), labels.at(col).toStdString().c_str(), format);
        }
        ++currentXlsRow;
        int end = static_cast<int>(jList.count() * 0.1 > 5 ? jList.count() * 0.1 : 5);

        for (int row = 0; row < jList.count() ; ++row) {
            if (!m_canRunning) {
                throw  QString(stopStr);
            }
            LOG_MSG_BOOT message = jList.at(row);
            int col = 0;
            worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col++), message.status.toStdString().c_str(), nullptr);
            worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col++), message.msg.toStdString().c_str(), nullptr);
            ++currentXlsRow;
            sigProgress(row + 1, jList.count() + end);
        }


        workbook_close(workbook);
        malloc_trim(0);
        sigProgress(100, 100);
    } catch (const QString &ErrorStr) {
        qCWarning(logExport) << "Export Stop" << ErrorStr;
        emit sigResult(false);
        if (ErrorStr != stopStr) {
            emit sigError(QString("export error: %1").arg(ErrorStr));
        }
        return false;
    }
    emit sigResult(m_canRunning);
    return m_canRunning;
}

/**
 * @brief LogExportThread::exportToXls导出到日志xlsx格式函数，对LOG_MSG_XORG数据类型的重载（指xorg日志）
 * @param fileName 导出文件路径全称
 * @param jList 要导出的数据源 LOG_MSG_XORG
 * @param labels 表头字符串
 * @return 是否导出成功
 */
bool LogExportThread::exportToXls(const QString &fileName, const QList<LOG_MSG_XORG> &jList, const QStringList &labels)
{
    try {
        auto currentXlsRow = 0;
        lxw_workbook  *workbook  = workbook_new(fileName.toStdString().c_str());
        lxw_worksheet *worksheet = workbook_add_worksheet(workbook, nullptr);
        lxw_format *format = workbook_add_format(workbook);
        format_set_bold(format);
        for (int col = 0; col < labels.count(); ++col) {
            worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col), labels.at(col).toStdString().c_str(), format);
        }
        ++currentXlsRow;
        int end = static_cast<int>(jList.count() * 0.1 > 5 ? jList.count() * 0.1 : 5);

        for (int row = 0; row < jList.count() ; ++row) {
            if (!m_canRunning) {
                throw  QString(stopStr);
            }
            LOG_MSG_XORG message = jList.at(row);
            int col = 0;
            worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col++), message.offset.toStdString().c_str(), nullptr);
            worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col++), message.msg.toStdString().c_str(), nullptr);
            ++currentXlsRow;
            sigProgress(row + 1, jList.count() + end);
        }


        workbook_close(workbook);
        malloc_trim(0);
        sigProgress(100, 100);
    } catch (const QString &ErrorStr) {
        qCWarning(logExport) << "Export Stop" << ErrorStr;
        emit sigResult(false);
        if (ErrorStr != stopStr) {
            emit sigError(QString("export error: %1").arg(ErrorStr));
        }
        return false;
    }
    emit sigResult(m_canRunning);
    return m_canRunning;
}

/**
 * @brief LogExportThread::exportToXls导出到日志xlsx格式函数，对LOG_MSG_NORMAL数据类型的重载（指开关机日志）
 * @param fileName 导出文件路径全称
 * @param jList 要导出的数据源 LOG_MSG_NORMAL
 * @param labels 表头字符串
 * @return 是否导出成功
 */
bool LogExportThread::exportToXls(const QString &fileName, const QList<LOG_MSG_NORMAL> &jList, const QStringList &labels)
{
    try {
        auto currentXlsRow = 0;
        lxw_workbook  *workbook  = workbook_new(fileName.toStdString().c_str());
        lxw_worksheet *worksheet = workbook_add_worksheet(workbook, nullptr);
        lxw_format *format = workbook_add_format(workbook);
        format_set_bold(format);
        for (int col = 0; col < labels.count(); ++col) {
            worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col), labels.at(col).toStdString().c_str(), format);
        }
        ++currentXlsRow;
        int end = static_cast<int>(jList.count() * 0.1 > 5 ? jList.count() * 0.1 : 5);

        for (int row = 0; row < jList.count() ; ++row) {
            if (!m_canRunning) {
                throw  QString(stopStr);
            }
            LOG_MSG_NORMAL message = jList.at(row);
            int col = 0;
            worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col++), message.eventType.toStdString().c_str(), nullptr);
            worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col++), message.userName.toStdString().c_str(), nullptr);
            worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col++), message.dateTime.toStdString().c_str(), nullptr);
            worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col++), message.msg.toStdString().c_str(), nullptr);
            ++currentXlsRow;
            sigProgress(row + 1, jList.count() + end);
        }


        workbook_close(workbook);
        malloc_trim(0);
        sigProgress(100, 100);
    } catch (const QString &ErrorStr) {
        qCWarning(logExport) << "Export Stop" << ErrorStr;
        emit sigResult(false);
        if (ErrorStr != stopStr) {
            emit sigError(QString("export error: %1").arg(ErrorStr));
        }
        return false;
    }
    emit sigResult(m_canRunning);
    return m_canRunning;
}

/**
 * @brief LogExportThread::exportToXls导出到日志xlsw格式函数，对LOG_MSG_KWIN数据类型的重载（指klu上的kwin日志）
 * @param fileName 导出文件路径全称
 * @param jList 要导出的数据源 LOG_MSG_KWIN
 * @param labels 表头字符串
 * @return 是否导出成功
 */
bool LogExportThread::exportToXls(const QString &fileName, const QList<LOG_MSG_KWIN> &jList, const QStringList &labels)
{
    try {
        auto currentXlsRow = 0;
        lxw_workbook  *workbook  = workbook_new(fileName.toStdString().c_str());
        lxw_worksheet *worksheet = workbook_add_worksheet(workbook, nullptr);
        lxw_format *format = workbook_add_format(workbook);
        format_set_bold(format);
        for (int col = 0; col < labels.count(); ++col) {
            worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col), labels.at(col).toStdString().c_str(), format);
        }
        ++currentXlsRow;
        int end = static_cast<int>(jList.count() * 0.1 > 5 ? jList.count() * 0.1 : 5);

        for (int row = 0; row < jList.count() ; ++row) {
            if (!m_canRunning) {
                throw  QString(stopStr);
            }
            LOG_MSG_KWIN message = jList.at(row);
            int col = 0;
            worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col++), message.msg.toStdString().c_str(), nullptr);
            ++currentXlsRow;
            sigProgress(row + 1, jList.count() + end);
        }


        workbook_close(workbook);
        malloc_trim(0);
        sigProgress(100, 100);
    } catch (const QString &ErrorStr) {
        qCWarning(logExport) << "Export Stop" << ErrorStr;
        emit sigResult(false);
        if (ErrorStr != stopStr) {
            emit sigError(QString("export error: %1").arg(ErrorStr));
        }
        return false;
    }
    emit sigResult(m_canRunning);
    return m_canRunning;
}

bool LogExportThread::exportToXls(const QString &fileName, const QList<LOG_MSG_DNF> &jList, const QStringList &labels)
{
    try {
        auto currentXlsRow = 0;
        lxw_workbook *workbook = workbook_new(fileName.toStdString().c_str());
        lxw_worksheet *worksheet = workbook_add_worksheet(workbook, nullptr);
        lxw_format *format = workbook_add_format(workbook);
        format_set_bold(format);
        for (int col = 0; col < labels.count(); ++col) {
            worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col), labels.at(col).toStdString().c_str(), format);
        }
        ++currentXlsRow;
        int end = static_cast<int>(jList.count() * 0.1 > 5 ? jList.count() * 0.1 : 5);

        for (int row = 0; row < jList.count(); ++row) {
            if (!m_canRunning) {
                throw QString(stopStr);
            }
            LOG_MSG_DNF message = jList.at(row);
            int col = 0;
            worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col++), message.dateTime.toStdString().c_str(), nullptr);
            worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col++), message.level.toStdString().c_str(), nullptr);
            worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col++), message.msg.toStdString().c_str(), nullptr);
            ++currentXlsRow;
            sigProgress(row + 1, jList.count() + end);
        }

        workbook_close(workbook);
        malloc_trim(0);
        sigProgress(100, 100);
    } catch (const QString &ErrorStr) {
        qCWarning(logExport) << "Export Stop" << ErrorStr;
        emit sigResult(false);
        if (ErrorStr != stopStr) {
            emit sigError(QString("export error: %1").arg(ErrorStr));
        }
        return false;
    }
    emit sigResult(m_canRunning);
    return m_canRunning;
}

bool LogExportThread::exportToXls(const QString &fileName, const QList<LOG_MSG_DMESG> &jList, const QStringList &labels)
{
    try {
        auto currentXlsRow = 0;
        lxw_workbook *workbook = workbook_new(fileName.toStdString().c_str());
        lxw_worksheet *worksheet = workbook_add_worksheet(workbook, nullptr);
        lxw_format *format = workbook_add_format(workbook);
        format_set_bold(format);
        for (int col = 0; col < labels.count(); ++col) {
            worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col), labels.at(col).toStdString().c_str(), format);
        }
        ++currentXlsRow;
        int end = static_cast<int>(jList.count() * 0.1 > 5 ? jList.count() * 0.1 : 5);

        for (int row = 0; row < jList.count(); ++row) {
            if (!m_canRunning) {
                throw QString(stopStr);
            }
            LOG_MSG_DMESG message = jList.at(row);
            int col = 0;
            worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col++), message.dateTime.toStdString().c_str(), nullptr);
            worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col++), message.level.toStdString().c_str(), nullptr);
            worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col++), message.msg.toStdString().c_str(), nullptr);
            ++currentXlsRow;
            sigProgress(row + 1, jList.count() + end);
        }

        workbook_close(workbook);
        malloc_trim(0);
        sigProgress(100, 100);
    } catch (const QString &ErrorStr) {
        qCWarning(logExport) << "Export Stop" << ErrorStr;
        emit sigResult(false);
        if (ErrorStr != stopStr) {
            emit sigError(QString("export error: %1").arg(ErrorStr));
        }
        return false;
    }
    emit sigResult(m_canRunning);
    return m_canRunning;
}

bool LogExportThread::exportToXls(const QString &fileName, const QList<LOG_MSG_AUDIT> &jList, const QStringList &labels)
{
    try {
        auto currentXlsRow = 0;
        lxw_workbook  *workbook  = workbook_new(fileName.toStdString().c_str());
        lxw_worksheet *worksheet = workbook_add_worksheet(workbook, nullptr);
        lxw_format *format = workbook_add_format(workbook);
        format_set_bold(format);
        for (int col = 0; col < labels.count(); ++col) {
            worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col), labels.at(col).toStdString().c_str(), format);
        }
        ++currentXlsRow;
        int end = static_cast<int>(jList.count() * 0.1 > 5 ? jList.count() * 0.1 : 5);

        for (int row = 0; row < jList.count() ; ++row) {
            if (!m_canRunning) {
                throw  QString(stopStr);
            }
            LOG_MSG_AUDIT message = jList.at(row);
            int col = 0;
            worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col++), message.eventType.toStdString().c_str(), nullptr);
            worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col++), message.dateTime.toStdString().c_str(), nullptr);
            worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col++), message.processName.toStdString().c_str(), nullptr);
            worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col++), message.status.toStdString().c_str(), nullptr);
            worksheet_write_string(worksheet, static_cast<lxw_row_t>(currentXlsRow), static_cast<lxw_col_t>(col++), message.msg.toStdString().c_str(), nullptr);
            ++currentXlsRow;
            sigProgress(row + 1, jList.count() + end);
        }

        workbook_close(workbook);
        malloc_trim(0);
        sigProgress(100, 100);
    } catch (const QString &ErrorStr) {
        qCWarning(logExport) << "Export Stop" << ErrorStr;
        emit sigResult(false);
        if (ErrorStr != stopStr) {
            emit sigError(QString("export error: %1").arg(ErrorStr));
        }
        return false;
    }
    emit sigResult(m_canRunning);
    return m_canRunning;
}

bool LogExportThread::exportToZip(const QString &fileName, const QList<LOG_MSG_COREDUMP> &jList)
{
    QString tmpPath = Utils::getAppDataPath() + "/tmp/";
    QDir dir(tmpPath);
    //删除临时目录
    dir.removeRecursively();
    //创建临时目录
    Utils::mkMutiDir(tmpPath);

    //复制文件
    int nCoreDumpCount = 0;
    for (auto &it : jList) {
        DLDBusHandler::instance(this)->exportLog(tmpPath, it.storagePath, true);
        if (it.coreFile == "present")
            nCoreDumpCount++;
        if (!m_canRunning) {
            break;
        }
    }

    if (!m_canRunning) {
        return false;
    }
    //打包日志文件
    QProcess procss;
    procss.setWorkingDirectory(tmpPath);
    QStringList arg = {"-c"};
    // 使用7z进行压缩，方便获取进度
    arg.append(QString("7z a -l -bsp1 tmp.zip ./*;mv tmp.zip '%1'").arg(fileName));

    // refresh progress
    bool ret = false;
    // 有coredump文件，才读取压缩进度
    if (nCoreDumpCount > 0) {
        connect(&procss, &QProcess::readyReadStandardOutput, this, [&](){
            if (!m_canRunning) {
                procss.kill();
                ret = false;
                return;
            }

            QByteArray dd = procss.readAllStandardOutput();
            QList<QString> lines = QString(dd).split('\n', QString::SkipEmptyParts);
            for (const QString &line : qAsConst(lines)) {
                int pos = line.indexOf(QLatin1Char('%'));
                if (pos > 1) {
                    int percentage = line.midRef(pos - 3, 3).toInt();
                    sigProgress(percentage, 100);
                }
            }
            ret = true;
        });
    } else {
        ret = true;
    }

    procss.start("/bin/bash", arg);
    procss.waitForFinished(-1);

    emit sigResult(ret);

    dir.removeRecursively();
    return m_canRunning;
}

/**
 * @brief LogExportThread::initMap 初始化等级和对应显示字符的map
 */
void LogExportThread::initMap()
{
    m_levelStrMap.clear();
    m_levelStrMap.insert("Emergency", DApplication::translate("Level", "Emergency"));
    m_levelStrMap.insert("Alert", DApplication::translate("Level", "Alert"));
    m_levelStrMap.insert("Critical", DApplication::translate("Level", "Critical"));
    m_levelStrMap.insert("Error", DApplication::translate("Level", "Error"));
    m_levelStrMap.insert("Warning", DApplication::translate("Level", "Warning"));
    m_levelStrMap.insert("Notice", DApplication::translate("Level", "Notice"));
    m_levelStrMap.insert("Info", DApplication::translate("Level", "Info"));
    m_levelStrMap.insert("Debug", DApplication::translate("Level", "Debug"));
}

/**
 * @brief LogExportThread::strTranslate 通过等级字符获取显示字符
 * @param iLevelStr 等级字符
 * @return 显示字符
 */
QString LogExportThread::strTranslate(const QString &iLevelStr)
{
    return m_levelStrMap.value(iLevelStr, iLevelStr);
}

void LogExportThread::htmlEscapeCovert(QString &htmlMsg)
{
    //无法对所有转义字符进行转换，对常用转义字符转换
    htmlMsg.replace("<", "&lt", Qt::CaseInsensitive);
    htmlMsg.replace(">", "&gt", Qt::CaseInsensitive);
    htmlMsg.replace("?", "&iexcl", Qt::CaseInsensitive);
    htmlMsg.replace("￥", "&yen", Qt::CaseInsensitive);
    htmlMsg.replace("|", "&brvbar", Qt::CaseInsensitive);
}

/**
 * @brief LogExportThread::run 线程的run函数，通过配置类型执行相应的导出逻辑
 */
void LogExportThread::run()
{
    qCDebug(logExport) << "threadrun";
    sigProgress(0, 100);
    switch (m_runMode) {
    case TxtModel: {
        exportToTxt(m_fileName, m_pModel, m_flag);
        break;
    }
    case TxtJOURNAL: {
        if (m_flag == JOURNAL)
            exportToTxt(m_fileName, m_jList, m_labels, m_flag);
        else if (m_flag == KERN || m_flag == Kwin)
            exportToTxt(m_fileName, m_logDataList, m_labels, m_flag);
        break;
    }
    case TxtAPP: {
        exportToTxt(m_fileName, m_appList, m_labels, m_appName);
        break;
    }
    case TxtDPKG: {
        exportToTxt(m_fileName, m_dpkgList, m_labels);
        break;
    }
    case TxtBOOT: {
        exportToTxt(m_fileName, m_bootList, m_labels);
        break;
    }
    case TxtXORG: {
        exportToTxt(m_fileName, m_xorgList, m_labels);
        break;
    }
    case TxtNORMAL: {
        exportToTxt(m_fileName, m_normalList, m_labels);
        break;
    }
    case TxtDNF: {
        exportToTxt(m_fileName, m_dnfList, m_labels);
        break;
    }
    case TxtDMESG: {
        exportToTxt(m_fileName, m_dmesgList, m_labels);
        break;
    }
    case TxtKWIN: {
        exportToTxt(m_fileName, m_kwinList, m_labels);
        break;
    }
    case TxtAUDIT: {
        exportToTxt(m_fileName, m_alist, m_labels);
    }
        break;
    case HtmlModel: {
        exportToHtml(m_fileName, m_pModel, m_flag);
        break;
    }
    case HtmlJOURNAL: {
        if (m_flag == JOURNAL)
            exportToHtml(m_fileName, m_jList, m_labels, m_flag);
        else if (m_flag == KERN || m_flag == Kwin)
            exportToHtml(m_fileName, m_logDataList, m_labels, m_flag);
        break;
    }
    case HtmlAPP: {
        exportToHtml(m_fileName, m_appList, m_labels, m_appName);
        break;
    }
    case HtmlDPKG: {
        exportToHtml(m_fileName, m_dpkgList, m_labels);
        break;
    }
    case HtmlBOOT: {
        exportToHtml(m_fileName, m_bootList, m_labels);
        break;
    }
    case HtmlXORG: {
        exportToHtml(m_fileName, m_xorgList, m_labels);
        break;
    }
    case HtmlNORMAL: {
        exportToHtml(m_fileName, m_normalList, m_labels);
        break;
    }
    case HtmlKWIN: {
        exportToHtml(m_fileName, m_kwinList, m_labels);
        break;
    }
    case HtmlDNF: {
        exportToHtml(m_fileName, m_dnfList, m_labels);
        break;
    }
    case HtmlDMESG: {
        exportToHtml(m_fileName, m_dmesgList, m_labels);
        break;
    }
    case HtmlAUDIT: {
        exportToHtml(m_fileName, m_alist, m_labels);
    }
        break;
    case DocJOURNAL: {
        if (m_flag == JOURNAL)
            exportToDoc(m_fileName, m_jList, m_labels, m_flag);
        else if (m_flag == KERN || m_flag == Kwin)
            exportToDoc(m_fileName, m_logDataList, m_labels, m_flag);
        break;
    }
    case DocAPP: {
        exportToDoc(m_fileName, m_appList, m_labels, m_appName);
        break;
    }
    case DocDPKG: {
        exportToDoc(m_fileName, m_dpkgList, m_labels);
        break;
    }
    case DocBOOT: {
        exportToDoc(m_fileName, m_bootList, m_labels);
        break;
    }
    case DocXORG: {
        exportToDoc(m_fileName, m_xorgList, m_labels);
        break;
    }
    case DocNORMAL: {
        exportToDoc(m_fileName, m_normalList, m_labels);
        break;
    }
    case DocKWIN: {
        exportToDoc(m_fileName, m_kwinList, m_labels);
        break;
    }
    case DocDNF: {
        exportToDoc(m_fileName, m_dnfList, m_labels);
        break;
    }
    case DocDMESG: {
        exportToDoc(m_fileName, m_dmesgList, m_labels);
        break;
    }
    case DocAUDIT: {
        exportToDoc(m_fileName, m_alist, m_labels);
    }
        break;
    case XlsJOURNAL: {
        if (m_flag == JOURNAL)
            exportToXls(m_fileName, m_jList, m_labels, m_flag);
        else if (m_flag == KERN || m_flag == Kwin)
            exportToXls(m_fileName, m_logDataList, m_labels, m_flag);
        break;
    }
    case XlsAPP: {
        exportToXls(m_fileName, m_appList, m_labels, m_appName);
        break;
    }
    case XlsDPKG: {
        exportToXls(m_fileName, m_dpkgList, m_labels);
        break;
    }
    case XlsBOOT: {
        exportToXls(m_fileName, m_bootList, m_labels);
        break;
    }
    case XlsXORG: {
        exportToXls(m_fileName, m_xorgList, m_labels);
        break;
    }
    case XlsNORMAL: {
        exportToXls(m_fileName, m_normalList, m_labels);
        break;
    }
    case XlsKWIN: {
        exportToXls(m_fileName, m_kwinList, m_labels);
        break;
    }
    case XlsDNF: {
        exportToXls(m_fileName, m_dnfList, m_labels);
        break;
    }
    case XlsDMESG: {
        exportToXls(m_fileName, m_dmesgList, m_labels);
        break;
    }
    case XlsAUDIT: {
        exportToXls(m_fileName, m_alist, m_labels);
        break;
    }
    case ZipCoredump: {
        exportToZip(m_fileName, m_coredumplist);
        break;
    }
    default:
        break;
    }
    if (!m_canRunning) {
        Utils::checkAndDeleteDir(m_fileName);
    }

    m_canRunning = false;
}
