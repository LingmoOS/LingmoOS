// SPDX-FileCopyrightText: 2019 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "parsethreadkern.h"

#include <QDebug>
#include <QDateTime>
#include <time.h>
using namespace std;

#ifdef QT_DEBUG
Q_LOGGING_CATEGORY(logParseWorkKern, "org.deepin.log.viewer.parse.work.kern")
#else
Q_LOGGING_CATEGORY(logParseWorkKern, "org.deepin.log.viewer.parse.work.kern", QtInfoMsg)
#endif

/**
 * @brief ParseThreadKern::ParseThreadKern 构造函数
 * @param parent 父对象
 */
ParseThreadKern::ParseThreadKern(QObject *parent)
    : ParseThreadBase(parent)
{
    
}

/**
 * @brief ParseThreadKern::~ParseThreadKern 析构函数，停止并销毁process指针
 */
ParseThreadKern::~ParseThreadKern()
{

}

/**
 * @brief ParseThreadKern::run 线程执行虚函数
 */
void ParseThreadKern::run()
{
    //此线程刚开始把可以继续变量置true，不然下面没法跑
    m_canRun = true;

    handleKern();
}

/**
 * @brief ParseThreadKern::handleKern 处理获取内核日志
 */
void ParseThreadKern::handleKern()
{
    QList<QString> dataList;
    qint64 gStartLine = m_filter.segementIndex * SEGEMENT_SIZE;
    m_FilePath = DLDBusHandler::instance(this)->getFileInfo(m_filter.filePath, false);
    for (int i = 0; i < m_FilePath.count(); i++) {
        if (!m_FilePath.at(i).contains("txt")) {
            QFile file(m_FilePath.at(i)); // add by Airy
            if (!file.exists()) {
                emit parseFinished(m_threadCount, m_type);
                return;
            }
        }
        if (!m_canRun) {
            return;
        }

        if (!Utils::runInCmd) {
            initProccess();
            if (!m_canRun) {
                return;
            }
            m_process->setProcessChannelMode(QProcess::MergedChannels);
            if (!m_canRun) {
                return;
            }
            //共享内存对应变量置true，允许进程内部逻辑运行
            ShareMemoryInfo shareInfo;
            shareInfo.isStart = true;
            SharedMemoryManager::instance()->setRunnableTag(shareInfo);
            //启动日志需要提权获取，运行的时候把对应共享内存的名称传进去，方便获取进程拿标记量判断是否继续运行
            m_process->start("pkexec", QStringList() << "logViewerAuth"
                             << m_FilePath.at(i) << SharedMemoryManager::instance()->getRunnableKey());
            m_process->waitForFinished(-1);
            //有错则传出空数据
            if (m_process->exitCode() != 0) {
                emit parseFinished(m_threadCount, m_type, CancelAuth);
                return;
            }
        }

        if (!m_canRun) {
            return;
        }

        //如果是压缩文件，对其解压缩
        QString filePath = m_FilePath.at(i);
        if(QString::compare(QFileInfo(filePath).suffix(), "gz", Qt::CaseInsensitive) == 0){
            QStringList filePathList = DLDBusHandler::instance(this)->getFileInfo(filePath);
            if(filePathList.size()){
                filePath = filePathList.at(0);
            }else {
                filePath = "";
            }
        }

        qint64 lineCount = DLDBusHandler::instance(this)->getLineCount(filePath);

        // 获取全局起始行在当前文件的相对起始行位置
        if (gStartLine >= lineCount) {
            gStartLine -= lineCount;
            continue;
        }

        qint64 startLine = gStartLine;

        QStringList strList = DLDBusHandler::instance(this)->readLogLinesInRange(filePath, startLine, SEGEMENT_SIZE);
        for (int j = strList.size() - 1; j >= 0; --j) {
            if (!m_canRun) {
                return;
            }
            QString str = strList.at(j);
            LOG_MSG_BASE msg;
            //删除颜色格式字符
            str.replace(QRegExp("\\#033\\[\\d+(;\\d+){0,2}m"), "");
            QStringList list = str.split(" ", QString::SkipEmptyParts);
            if (list.size() < 5)
                continue;
            //获取内核年份接口已添加，等待系统接口添加年份改变相关日志
            QStringList timeList;
            if (list[0].contains("-")) {
                timeList.append(list[0]);
                timeList.append(list[1]);
                iTime = formatDateTime(list[0], list[1]);
            } else {
                timeList.append(list[0]);
                timeList.append(list[1]);
                timeList.append(list[2]);
                iTime = formatDateTime(list[0], list[1], list[2]);
            }

            //对时间筛选
            if (m_filter.timeFilterBegin > 0 && m_filter.timeFilterEnd > 0) {
                if (iTime < m_filter.timeFilterBegin || iTime > m_filter.timeFilterEnd)
                    continue;
            }

            msg.dateTime = timeList.join(" ");
            QStringList tmpList;
            if (list[0].contains("-")) {
                msg.hostName = list[2];
                tmpList = list[3].split("[");
            } else {
                msg.hostName = list[3];
                tmpList = list[4].split("[");
            }

            int m = 0;
            //内核日志存在年份，解析用户名和进程id
            if (list[0].contains("-")) {
                if (tmpList.size() != 2) {
                    msg.daemonName = list[3].split(":")[0];
                } else {
                    msg.daemonName = list[3].split("[")[0];
                    QString id = list[3].split("[")[1];
                    id.chop(2);
                    msg.daemonId = id;
                }
                m = 4;
            } else {//内核日志不存在年份,解析用户名和进程id
                if (tmpList.size() != 2) {
                    msg.daemonName = list[4].split(":")[0];
                } else {
                    msg.daemonName = list[4].split("[")[0];
                    QString id = list[4].split("[")[1];
                    id.chop(2);
                    msg.daemonId = id;
                }
                m = 5;
            }

            QString msgInfo;
            for (int k = m; k < list.size(); k++) {
                msgInfo.append(list[k] + " ");
            }
            msg.msg = msgInfo;

            dataList.append(QJsonDocument(msg.toJson()).toJson(QJsonDocument::Compact));
            if (!m_canRun) {
                return;
            }
            //每获得500个数据就发出信号给控件加载
            if (dataList.count() % SINGLE_READ_CNT == 0) {
                emit logData(m_threadCount, dataList, m_type);
                dataList.clear();
            }
            if (!m_canRun) {
                return;
            }
        }
    }
    //最后可能有余下不足500的数据
    if (dataList.count() >= 0) {
        emit logData(m_threadCount, dataList, m_type);
    }
    emit parseFinished(m_threadCount, m_type);
}

/**
 * @brief ParseThreadKern::formatDateTime 内核日志没有年份 格式为Sep 29 15:53:34 所以需要特殊转换
 * @param m 月份字符串
 * @param d 日期字符串
 * @param t 时间字符串
 * @return 时间毫秒数
 */
qint64 ParseThreadKern::formatDateTime(QString m, QString d, QString t)
{
    QLocale local(QLocale::English, QLocale::UnitedStates);

    QDate curdt = QDate::currentDate();

    QString tStr = QString("%1 %2 %3 %4").arg(m).arg(d).arg(curdt.year()).arg(t);
    QDateTime dt = local.toDateTime(tStr, "MMM d yyyy hh:mm:ss");
    return dt.toMSecsSinceEpoch();
}

/**
 * @brief ParseThreadKern::formatDateTime 内核日志有年份 格式为2020-01-05 所以需要特殊转换
 * @param y 年月日
 * @param t 时间字符串
 * @return 时间毫秒数
 */
qint64 ParseThreadKern::formatDateTime(QString y, QString t)
{
    //when /var/kern.log have the year
    QLocale local(QLocale::English, QLocale::UnitedStates);
    QString tStr = QString("%1 %2").arg(y).arg(t);
    QDateTime dt = local.toDateTime(tStr, "yyyy-MM-dd hh:mm:ss");
    return dt.toMSecsSinceEpoch();
}
