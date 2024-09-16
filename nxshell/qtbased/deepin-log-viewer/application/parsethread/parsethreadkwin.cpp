// SPDX-FileCopyrightText: 2019 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "parsethreadkwin.h"

#include <QDebug>
#include <QDateTime>
#include <time.h>
using namespace std;

#ifdef QT_DEBUG
Q_LOGGING_CATEGORY(logParseWorkKwin, "org.deepin.log.viewer.parse.work.kwin")
#else
Q_LOGGING_CATEGORY(logParseWorkKwin, "org.deepin.log.viewer.parse.work.kwin", QtInfoMsg)
#endif

/**
 * @brief ParseThreadKwin::ParseThreadKwin 构造函数
 * @param parent 父对象
 */
ParseThreadKwin::ParseThreadKwin(QObject *parent)
    : ParseThreadBase(parent)
{
    
}

/**
 * @brief ParseThreadKwin::~ParseThreadKwin 析构函数，停止并销毁process指针
 */
ParseThreadKwin::~ParseThreadKwin()
{

}

/**
 * @brief ParseThreadKwin::run 线程执行虚函数
 */
void ParseThreadKwin::run()
{
    //此线程刚开始把可以继续变量置true，不然下面没法跑
    m_canRun = true;

    handleKwin();
}

/**
 * @brief ParseThreadKwin::handleKern 处理获取内核日志
 */
void ParseThreadKwin::handleKwin()
{
    QFile file(KWIN_TREE_DATA);
    if (!m_canRun) {
        return;
    }
    QList<QString> dataList;
    if (!file.exists()) {
        emit parseFinished(m_threadCount, m_type);
        return;
    }
    if (!m_canRun) {
        return;
    }

    qint64 gStartLine = m_filter.segementIndex * SEGEMENT_SIZE;
    qint64 lineCount = DLDBusHandler::instance(this)->getLineCount(KWIN_TREE_DATA);

    // 获取全局起始行在当前文件的相对起始行位置
    if (gStartLine >= lineCount) {
        return;
    }

    qint64 startLine = gStartLine;
    QStringList strList = DLDBusHandler::instance(this)->readLogLinesInRange(KWIN_TREE_DATA, startLine, SEGEMENT_SIZE);
    if (!m_canRun) {
        return;
    }

    for (int i = strList.size() - 1; i >= 0 ; --i)  {
        QString str = strList.at(i);
        if (!m_canRun) {
            return;
        }
        if (str.trimmed().isEmpty()) {
            continue;
        }
        LOG_MSG_BASE msg;
        msg.msg = str;
        dataList.append(QJsonDocument(msg.toJson()).toJson(QJsonDocument::Compact));
        //每获得500个数据就发出信号给控件加载
        if (dataList.count() % SINGLE_READ_CNT == 0) {
            emit logData(m_threadCount, dataList, m_type);
            dataList.clear();
        }
    }

    if (!m_canRun) {
        return;
    }
    //最后可能有余下不足500的数据
    if (dataList.count() >= 0) {
        emit logData(m_threadCount, dataList, m_type);
    }
    emit parseFinished(m_threadCount, m_type);
}
