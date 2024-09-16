// SPDX-FileCopyrightText: 2019 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logsegementexportthread.h"
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
Q_LOGGING_CATEGORY(logSegementExport, "org.deepin.log.viewer.segement.export.work")
#else
Q_LOGGING_CATEGORY(logSegementExport, "org.deepin.log.viewer.segement.export.work", QtInfoMsg)
#endif

/**
 * @brief LogSegementExportThread::LogSegementExportThread 导出日志线程类构造函数
 * @param parent 父对象
 */
LogSegementExportThread::LogSegementExportThread(QObject *parent)
    :  QObject(parent),
       QRunnable()
{
    setAutoDelete(true);
}
/**
 * @brief LogSegementExportThread::~LogSegementExportThread 析构函数
 */
LogSegementExportThread::~LogSegementExportThread()
{
    qCDebug(logSegementExport) << "LogSegementExportThread destoryed.";
    //释放空闲内存
    malloc_trim(0);
}

void LogSegementExportThread::setParameter(const QString &fileName, const QList<QString> &jList, const QStringList &lables, LOG_FLAG flag)
{
    QMutexLocker locker(&mutex);
    m_fileName = fileName;
    m_logDataList = jList;
    m_flag = flag;
    m_labels = lables;
    m_bForceStop = false;
    if (m_fileName.endsWith(".txt")) {
        m_runMode = Txt;
    } else if (fileName.endsWith(".html")) {
        m_runMode = Html;
    } else if (fileName.endsWith(".doc")) {
        m_runMode = Doc;
        if (!m_pDocMerger)
            initDoc();
    } else if (fileName.endsWith(".xls")) {
        m_runMode = Xls;
        if (!m_pWorkbook)
            initXls();
    }
    condition.wakeOne();
}

void LogSegementExportThread::initDoc()
{
    QString tempdir = getDocTemplatePath();
    if (tempdir.isEmpty())
        return;

    m_pDocMerger = &DocxFactory:: WordProcessingMerger::getInstance();
    m_pDocMerger->load(tempdir.toStdString());
    //往表头中添加表头描述，表头为第一行，数据则在下面
    for (int col = 0; col < m_labels.count(); ++col) {
        m_pDocMerger->setClipboardValue("tableRow", QString("column%1").arg(col + 1).toStdString(), m_labels.at(col).toStdString());
    }
    m_pDocMerger->paste("tableRow");
}

void LogSegementExportThread::initXls()
{
    m_currentXlsRow = 0;
    m_pWorkbook = workbook_new(m_fileName.toStdString().c_str());
    m_pWorksheet = workbook_add_worksheet(m_pWorkbook, nullptr);
    lxw_format *format = workbook_add_format(m_pWorkbook);
    format_set_bold(format);
    for (int col = 0; col < m_labels.count(); ++col) {
        worksheet_write_string(m_pWorksheet, static_cast<lxw_row_t>(m_currentXlsRow), static_cast<lxw_col_t>(col), m_labels.at(col).toStdString().c_str(), format);
    }
    ++m_currentXlsRow;
}

QString LogSegementExportThread::getDocTemplatePath()
{
    QString tempdir("");
    if (m_flag == KERN) {
        tempdir = "/usr/share/deepin-log-viewer/DocxTemplate/4column.dfw";
    } else if (m_flag == Kwin) {
        tempdir = "/usr/share/deepin-log-viewer/DocxTemplate/1column.dfw";
    } else {
        qCWarning(logSegementExport) << "exportToDoc type is Wrong!";
        return "";
    }
    if (!QFile(tempdir).exists()) {
        qCWarning(logSegementExport) << "export docx template is not exisits";
        return "";
    }

    return tempdir;
}

/**
 * @brief LogSegementExportThread::isProcessing 返回当前线程获取数据逻辑启动停止控制的变量
 * @return 当前线程获取数据逻辑启动停止控制的变量
 */
bool LogSegementExportThread::isProcessing()
{
    return !m_bForceStop;
}

/**
 * @brief LogSegementExportThread::stopImmediately 强制停止前线程获取数据逻辑
 */
void LogSegementExportThread::stopImmediately()
{
    QMutexLocker locker(&mutex);
    m_bForceStop = true;
    condition.wakeOne();
}

/**
 * @brief LogSegementExportThread::stop 停止前线程获取数据逻辑
 */
void LogSegementExportThread::stop()
{
    QMutexLocker locker(&mutex);
    m_bStop = true;
    condition.wakeOne();
}

/**
 * @brief LogSegementExportThread::run 线程的run函数，通过配置类型执行相应的导出逻辑
 */
void LogSegementExportThread::run()
{
    qCDebug(logSegementExport) << "threadrun";

    QMutexLocker locker(&mutex);
    while(!m_bForceStop && !m_bStop) {
        if (m_logDataList.isEmpty()) {
            condition.wait(&mutex);
        } else {
            try {
                switch (m_runMode) {
                case Txt: {
                    exportTxt();
                    break;
                }
                case Html:{
                    exportHtml();
                    break;
                }
                case Doc: {
                    exportToDoc();
                    break;
                }
                case Xls: {
                    exportToXls();
                    break;
                }
                default:
                    break;
                }
                emit sigProgress(++m_nCurProcess, m_nTotalProcess);
                m_logDataList.clear();
            } catch (const QString &ErrorStr) {
                // 捕获到异常，导出失败，发出失败信号
                qCWarning(logSegementExport) << "Export Stop" << ErrorStr;
                emit sigResult(false);
                if (ErrorStr != m_forceStopStr) {
                    emit sigError(QString("export error: %1").arg(ErrorStr));
                }
            }
        }
    }

    if (!m_bForceStop) {
        // 保存数据
        switch (m_runMode) {
        case Doc: 
            saveDoc();
            break;
        case Xls:
            closeXls();
            break;
        default:
            break;
        }

        // 进度100%
        emit sigProgress(m_nTotalProcess, m_nTotalProcess);
        // 延时200ms再发送导出成功信号，关闭导出进度框，让100%的进度有时间显示
        Utils::sleep(200);
    }

    emit sigResult(!m_bForceStop);

    if (m_bForceStop) {
        Utils::checkAndDeleteDir(m_fileName);
    }

    m_bForceStop = false;
}

bool LogSegementExportThread::exportTxt()
{
    //判断文件路径是否存在，不存在就返回错误
    QFile fi(m_fileName);
    if (!fi.open(m_bAppendWrite ? (QIODevice::Append | QIODevice::WriteOnly) : QIODevice::WriteOnly)) {
        emit sigError(m_openErroStr);
        return false;
    }

    QTextStream out(&fi);

    for (int i = 0; i < m_logDataList.count(); i++) {
        //导出逻辑启动停止控制，外部把m_forceStopStr置true时停止运行，抛出异常处理
        if (m_bForceStop) {
            fi.close();
            throw  QString(m_forceStopStr);
        }

        int col = 0;
        LOG_MSG_BASE jMsg;
        jMsg.fromJson(m_logDataList[i]);
        if (m_flag == KERN) {
            out << m_labels.value(col++, "") << ":" << jMsg.dateTime << " ";
            out << m_labels.value(col++, "") << ":" << jMsg.hostName << " ";
            out << m_labels.value(col++, "") << ":" << jMsg.daemonName << " ";
            out << m_labels.value(col++, "") << ":" << jMsg.msg << " ";
            out << "\n";
        } else if (m_flag == Kwin) {
            out << m_labels.value(col++, "") << ":" << jMsg.msg << " ";
            out << "\n";
        }
    }

    //设置文件编码为utf8
    out.setCodec(QTextCodec::codecForName("utf-8"));
    fi.close();

    return true;
}

bool LogSegementExportThread::exportHtml()
{
    QFile html(m_fileName);
    //判断文件路径是否存在，不存在就返回错误
    if (!html.open(m_bAppendWrite ? (QIODevice::Append | QIODevice::WriteOnly) : QIODevice::WriteOnly)) {
        emit sigError(m_openErroStr);
        return false;
    }

    //写网页头
    html.write("<!DOCTYPE html>\n");
    html.write("<html>\n");
    html.write("<body>\n");
    //写入表格标签
    html.write("<table border=\"1\">\n");
    //写入表头
    html.write("<tr>");
    for (int i = 0; i < m_labels.count(); ++i) {
        QString labelInfo = QString("<td>%1</td>").arg(m_labels.value(i));
        html.write(labelInfo.toUtf8().data());
    }
    //写入内容
    //根据字段拼出每行的网页内容
    html.write("</tr>");

    for (int row = 0; row < m_logDataList.count(); ++row) {
        if (m_bForceStop) {
            html.close();
            throw  QString(m_forceStopStr);
        }
        LOG_MSG_BASE jMsg;
        jMsg.fromJson(m_logDataList.at(row));
        htmlEscapeCovert(jMsg.msg);
        html.write("<tr>");
        if (m_flag == KERN) {
            QString info = QString("<td>%1</td>").arg(jMsg.dateTime);
            html.write(info.toUtf8().data());
            info = QString("<td>%1</td>").arg(jMsg.hostName);
            html.write(info.toUtf8().data());
            info = QString("<td>%1</td>").arg(jMsg.daemonName);
            html.write(info.toUtf8().data());
            info = QString("<td>%1</td>").arg(jMsg.msg);
            html.write(info.toUtf8().data());
        } else if (m_flag == Kwin) {
            QString info = QString("<td>%1</td>").arg(jMsg.msg);
            html.write(info.toUtf8().data());
        }
        html.write("</tr>");
    }

    //写入结尾
    html.write("</table>\n");
    html.write("</body>\n");
    html.write("</html>\n");

    html.close();

    return true;
}

bool LogSegementExportThread::exportToDoc()
{
    if (!m_pDocMerger)
        return false;

    for (int row = 0; row < m_logDataList.count(); ++row) {
        //导出逻辑启动停止控制，外部把m_forceStopStr置true时停止运行，抛出异常处理
        if (m_bForceStop) {
            throw  QString(m_forceStopStr);
        }
        LOG_MSG_BASE message;
        message.fromJson(m_logDataList.at(row));
        //把数据填入表格单元格中
        if (m_flag == KERN) {
            m_pDocMerger->setClipboardValue("tableRow", QString("column1").toStdString(), message.dateTime.toStdString());
            m_pDocMerger->setClipboardValue("tableRow", QString("column2").toStdString(), message.hostName.toStdString());
            m_pDocMerger->setClipboardValue("tableRow", QString("column3").toStdString(), message.daemonName.toStdString());
            m_pDocMerger->setClipboardValue("tableRow", QString("column4").toStdString(), message.msg.toStdString());
        } else if (m_flag == Kwin) {
            m_pDocMerger->setClipboardValue("tableRow", QString("column1").toStdString(), message.msg.toStdString());
        }
        m_pDocMerger->paste("tableRow");
    }

    return true;
}

bool LogSegementExportThread::exportToXls()
{
    if (!m_pWorksheet)
        return false;

    for (int row = 0; row < m_logDataList.count() ; ++row) {
        if (m_bForceStop) {
            throw  QString(m_forceStopStr);
        }
        LOG_MSG_BASE message;
        message.fromJson(m_logDataList.at(row));
        int col = 0;

        if (m_flag == KERN) {
            worksheet_write_string(m_pWorksheet, static_cast<lxw_row_t>(m_currentXlsRow), static_cast<lxw_col_t>(col++), message.dateTime.toStdString().c_str(), nullptr);
            worksheet_write_string(m_pWorksheet, static_cast<lxw_row_t>(m_currentXlsRow), static_cast<lxw_col_t>(col++), message.hostName.toStdString().c_str(), nullptr);
            worksheet_write_string(m_pWorksheet, static_cast<lxw_row_t>(m_currentXlsRow), static_cast<lxw_col_t>(col++), message.daemonName.toStdString().c_str(), nullptr);
            worksheet_write_string(m_pWorksheet, static_cast<lxw_row_t>(m_currentXlsRow), static_cast<lxw_col_t>(col++), message.msg.toStdString().c_str(), nullptr);
        } else if (m_flag == Kwin) {
            worksheet_write_string(m_pWorksheet, static_cast<lxw_row_t>(m_currentXlsRow), static_cast<lxw_col_t>(col++), message.msg.toStdString().c_str(), nullptr);
        }

        ++m_currentXlsRow;
    }

    return true;
}

void LogSegementExportThread::htmlEscapeCovert(QString &htmlMsg)
{
    //无法对所有转义字符进行转换，对常用转义字符转换
    htmlMsg.replace("<", "&lt", Qt::CaseInsensitive);
    htmlMsg.replace(">", "&gt", Qt::CaseInsensitive);
    htmlMsg.replace("?", "&iexcl", Qt::CaseInsensitive);
    htmlMsg.replace("￥", "&yen", Qt::CaseInsensitive);
    htmlMsg.replace("|", "&brvbar", Qt::CaseInsensitive);
}

void LogSegementExportThread::saveDoc()
{
    //保存，把拼好的xml写入文件中
    QString fileNamex = m_fileName + "x";

    QFile rsNameFile(m_fileName) ;
    if (rsNameFile.exists()) {
        rsNameFile.remove();
    }
    m_pDocMerger->save(fileNamex.toStdString());
    QFile(fileNamex).rename(m_fileName);
}

void LogSegementExportThread::closeXls()
{
    workbook_close(m_pWorkbook);
    malloc_trim(0);
}
