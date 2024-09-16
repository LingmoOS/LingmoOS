// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logapplicationparsethread.h"
#include "utils.h"
#include "dbusproxy/dldbushandler.h"

#include <DMessageBox>
#include <DApplication>

#include <systemd/sd-journal.h>

#include <QDateTime>
#include <QDebug>
#include <QProcess>

#include <QLoggingCategory>

#ifdef QT_DEBUG
Q_LOGGING_CATEGORY(logApp, "org.deepin.log.viewer.parse.app.work")
#else
Q_LOGGING_CATEGORY(logApp, "org.deepin.log.viewer.parse.app.work", QtInfoMsg)
#endif

DWIDGET_USE_NAMESPACE

//std::atomic<LogApplicationParseThread *> LogApplicationParseThread::m_instance;
//std::mutex LogApplicationParseThread::m_mutex;
int LogApplicationParseThread::thread_count = 0;
/**
 * @brief LogApplicationParseThread::LogApplicationParseThread 构造函数
 * @param parent 父对象
 */
LogApplicationParseThread::LogApplicationParseThread(QObject *parent)
    : QThread(parent)
{
    qRegisterMetaType<QList<LOG_MSG_APPLICATOIN> >("QList<LOG_MSG_APPLICATOIN>");

    initMap();
    //初始化等级数字对应显示文本的map
    initJournalMap();
    //静态计数变量加一并赋值给本对象的成员变量，以供外部判断是否为最新线程发出的数据信号
    thread_count++;
    m_threadCount = thread_count;
}

/**
 * @brief LogApplicationParseThread::~LogApplicationParseThread 析构函数，停止并销毁process指针
 */
LogApplicationParseThread::~LogApplicationParseThread()
{
    m_appList.clear();
    m_levelDict.clear();
    m_journalMap.clear();

    if (m_process) {
        m_process->kill();
        m_process->close();
        delete  m_process;
        m_process = nullptr;
    }
}

void LogApplicationParseThread::setFilters(const APP_FILTERSList &iFilters)
{
    m_AppFilers = iFilters;
}

/**
 * @brief LogApplicationParseThread::stopProccess 停止qprocess获取进程
 */
void LogApplicationParseThread::stopProccess()
{
    qCDebug(logApp) << "stopWork";
    m_canRun = false;
    if (m_process && m_process->isOpen()) {
        m_process->kill();
    }
}

int LogApplicationParseThread::getIndex()
{
    return m_threadCount;
}

/**
 * @brief LogApplicationParseThread::doWork 获取数据线程逻辑
 */
void LogApplicationParseThread::doWork()
{
    //此线程刚开始把可以继续变量置true，不然下面没法跑
    m_canRun = true;
    mutex.lock();
    m_appList.clear();
    mutex.unlock();

    // 遍历每个子模块对应的日志过滤配置项
    for (auto appFilter : m_AppFilers) {
        if (appFilter.logType == "file") {
            if (!parseByFile(appFilter))
                return;
        } else if (appFilter.logType == "journal") {
            if (!parseByJournal(appFilter))
                return;
        }
    }

    //最后可能有余下不足500的数据
    if (m_appList.count() >= 0) {
        emit appData(m_threadCount, m_appList);
    }

    emit appFinished(m_threadCount);
}

bool LogApplicationParseThread::parseByFile(const APP_FILTERS &app_filter)
{
    m_AppFiler = app_filter;

    initProccess();
    //connect(m_process, SIGNAL(finished(int)), m_process, SLOT(deleteLater()));
    //因为筛选信息中含有日志文件路径，所以不能为空，否则无法获取
    if (m_AppFiler.path.isEmpty()) {  //modified by Airy for bug 20457::if path is empty,item is not empty
        emit appFinished(m_threadCount);
    } else {
        QStringList filePath = DLDBusHandler::instance(this)->getFileInfo(m_AppFiler.path);
        for (int i = 0; i < filePath.count(); i++) {
            if (!m_canRun) {
                mutex.unlock();
                return false;
            }
            //按行解析
            QByteArray outByte = DLDBusHandler::instance(this)->readLog(filePath[i]).toUtf8();
            QString output = Utils::replaceEmptyByteArray(outByte);
            QStringList strList = QString(output ).split('\n', QString::SkipEmptyParts);
            //开启贪婪匹配
            QRegularExpression re("^(\\d{4}-[0-2]\\d-[0-3]\\d)\\D*([0-2]\\d:[0-5]\\d:[0-5]\\d.\\d*)[^A-Za-z]*([A-Za-z]*)[^\\[]*[^\\]]*\\]*\\s*(.*)$");

            for (int j = strList.size() - 1; j >= 0; --j) {
                if (!m_canRun) {
                    mutex.unlock();
                    return false;
                }
                LOG_MSG_APPLICATOIN msg;
                QString str = strList[j];

                QRegularExpressionMatch match = re.match(str);
                bool matchRes = match.hasMatch();
                if(!matchRes){
                    continue;
                }

                QString dateTime = match.captured(1)+" "+match.captured(2);
                qint64 dt = QDateTime::fromString(dateTime, "yyyy-MM-dd hh:mm:ss.zzz").toMSecsSinceEpoch();
                //按筛选条件筛选时间段
                if (m_AppFiler.timeFilterBegin > 0 && m_AppFiler.timeFilterEnd > 0) {
                    if (dt < m_AppFiler.timeFilterBegin || dt > m_AppFiler.timeFilterEnd)
                        continue;
                }

                msg.subModule = m_AppFiler.submodule;
                msg.dateTime = dateTime;
                msg.level = match.captured(3);
                //筛选日志等级
                if (m_AppFiler.lvlFilter != LVALL) {
                    if (m_levelDict.value(msg.level) != m_AppFiler.lvlFilter)
                        continue;
                }
                //获取信息
                msg.msg=match.captured(4);
                msg.detailInfo=match.captured(4);

                //如果日志太长就显示一部分
                if (msg.detailInfo.size() > 500) {
                    msg.msg = msg.detailInfo.mid(0, 500);
                }
                mutex.lock();
                m_appList.append(msg);
                mutex.unlock();
                //每获得500个数据就发出信号给控件加载
                if (m_appList.count() % SINGLE_READ_CNT == 0) {
                    mutex.lock();
                    emit appData(m_threadCount, m_appList);
                    m_appList.clear();
                    mutex.unlock();
                }
            }
            if (!m_canRun) {
                mutex.unlock();
                return false;
            }
        }
    }

    return true;
}

bool LogApplicationParseThread::parseByJournal(const APP_FILTERS &app_filter)
{
    m_AppFiler = app_filter;

    if ((!m_canRun)) {
        mutex.unlock();
        return false;
    }

    int r;

    sd_journal *j ;
    if ((!m_canRun)) {
        mutex.unlock();
        return false;
    }
    //打开日志文件
    r = sd_journal_open(&j, SD_JOURNAL_LOCAL_ONLY);
    if ((!m_canRun)) {
        mutex.unlock();
        sd_journal_close(j);
        return false;
    }
    //r为系统借口返回值，小于0则表示失败，直接返回
    if (r < 0) {
        fprintf(stderr, "Failed to open journal: %s\n", strerror(-r));
        return false;
    }
    //从尾部开始读，这样出来数据是倒叙，符合需求
    sd_journal_seek_tail(j);
    if ((!m_canRun)) {
        mutex.unlock();
        sd_journal_close(j);
        return false;
    }

    // 增加日志等级筛选
    if (m_AppFiler.lvlFilter != LVALL) {
        QString prio = QString("PRIORITY=%1").arg(m_AppFiler.lvlFilter);
        sd_journal_add_match(j, prio.toStdString().c_str(), 0);
    }

    // 查看是否开启通配符匹配
    bool bWildcardMatch = m_AppFiler.filter.endsWith("*");
    QString wildcard_CodeCategory = "";
    if (bWildcardMatch)
        wildcard_CodeCategory = m_AppFiler.filter.split("*").first();

    // 通配符匹配规则为空，不开启通配符匹配
    if (wildcard_CodeCategory.isEmpty())
        bWildcardMatch = false;

    bool bCanMatch = false;
    // 增加日志exec筛选
    if (!m_AppFiler.execPath.isEmpty()) {
        sd_journal_add_match(j, QString("_EXE=%1").arg(m_AppFiler.execPath).toStdString().c_str(), 0);
        bCanMatch = true;
    }

    // 增加code_category筛选，filter字段内容必须为完整的code_category种类名称
    if (!m_AppFiler.filter.isEmpty() && m_AppFiler.filter != "*" && !bWildcardMatch) {
        sd_journal_add_match(j, QString("CODE_CATEGORY=%1").arg(m_AppFiler.filter).toStdString().c_str(), 0);
        bCanMatch = true;
    }

    // exec和filter都未配置，只能按进程名称进行筛选
    if (!bCanMatch && !m_AppFiler.app.isEmpty()) {
        sd_journal_add_match(j, QString("SYSLOG_IDENTIFIER=%1").arg(m_AppFiler.app).toStdString().c_str(), 0);
        bCanMatch = true;
    }

    if ((!m_canRun)) {
        mutex.unlock();
        sd_journal_close(j);
        return false;
    }

    // journal不具备匹配条件，放弃journal应用日志解析
    if (!bCanMatch)
        return true;

    uint64_t beginTime = 0;
    uint64_t endTime = 0;
    if (m_AppFiler.timeFilterBegin != -1) {
        beginTime = static_cast<uint64_t>(m_AppFiler.timeFilterBegin * 1000);
        endTime = static_cast<uint64_t>(m_AppFiler.timeFilterEnd * 1000);
    }

    int cnt = 0;
    //调用宏开始迭代
    SD_JOURNAL_FOREACH_BACKWARDS(j) {
        if ((!m_canRun)) {
            mutex.unlock();
            sd_journal_close(j);
            return false;
        }
        const char *d;
        size_t l;

        LOG_MSG_APPLICATOIN logMsg;
        //获取时间
        r = sd_journal_get_data(j, "_SOURCE_REALTIME_TIMESTAMP", reinterpret_cast<const void **>(&d), &l);
        if (r < 0) {
            r = sd_journal_get_data(j, "__REALTIME_TIMESTAMP", reinterpret_cast<const void **>(&d), &l);
            if (r < 0) {
                continue;
            }
        }

        logMsg.subModule = m_AppFiler.submodule;

        uint64_t t;
        sd_journal_get_realtime_usec(j, &t);
        //解锁返回字符串长度上限，默认是64k，写0为无限
        // sd_journal_set_data_threshold(j, 0);
        QString dt = Utils::getReplaceColorStr(d).split("=").value(1);
        if (m_AppFiler.timeFilterBegin != -1) {
            if (t < beginTime || t > endTime)
                continue;
        }
        logMsg.dateTime = getDateTimeFromStamp(dt);

        // 根据filter进行通配符匹配查找
        if (bWildcardMatch) {
            r = sd_journal_get_data(j, "CODE_CATEGORY", reinterpret_cast<const void **>(&d), &l);
            QString code_category;
            if (r >= 0) {
                QStringList strList = Utils::getReplaceColorStr(d).split("=");
                strList.removeFirst();
                code_category = strList.join("=");
                if (!code_category.startsWith(wildcard_CodeCategory))
                    continue;
            }
        }

        //获取信息体
        r = sd_journal_get_data(j, "MESSAGE", reinterpret_cast<const void **>(&d), &l);
        if (r < 0) {
            logMsg.msg = "";
        } else {
            //出来的数据格式为 字段名= 信息体，但是因为信息体中也可能有=号，所以要把第一个去掉，后面的用=号拼起来
            QStringList strList = Utils::getReplaceColorStr(d).split("=");
            strList.removeFirst();
            logMsg.msg = strList.join("=");
        }
        logMsg.detailInfo = logMsg.msg;

        //如果日志太长就显示一部分
        if (logMsg.detailInfo.size() > 500) {
            logMsg.msg = logMsg.detailInfo.mid(0, 500);
        }

        //获取等级
        r = sd_journal_get_data(j, "PRIORITY", reinterpret_cast<const void **>(&d), &l);
        if (r < 0) {
            //有些时候的确会产生没有等级的日志，按照需求此时一律按调试处理，和journalctl 的筛选行为一致
            logMsg.level = i2str(7);
        } else {
            //获取等级为字段名= 数字 ，数字为0-7 ，对应紧急到调试，需要转换
            logMsg.level = i2str(Utils::getReplaceColorStr(d).split("=").value(1).toInt());
        }
        cnt++;
        mutex.lock();
        m_appList.append(logMsg);
        mutex.unlock();

        //每获得500个数据就发出信号给控件加载
        if (cnt % SINGLE_READ_CNT == 0) {
            mutex.lock();
            emit appData(m_threadCount, m_appList);
            m_appList.clear();
            //sleep(100);
            mutex.unlock();
        }
    }

    //第一次加载时这个之后的代码都不执行?故放到最后
    sd_journal_close(j);

    return true;
}

void LogApplicationParseThread::onProcFinished(int ret)
{
    Q_UNUSED(ret)
}

QString LogApplicationParseThread::getDateTimeFromStamp(const QString &str)
{
    QString ret = "";
    QString dtstr = str.left(str.length() - 3);
    QDateTime dt = QDateTime::fromMSecsSinceEpoch(dtstr.toLongLong());
    ret = dt.toString("yyyy-MM-dd hh:mm:ss.zzz");  // + QString(".%1").arg(ums);
    return ret;
}

/**
 * @brief LogApplicationParseThread::initMap 初始化数据结构
 */
void LogApplicationParseThread::initMap()
{
    m_levelDict.insert("Warning", WARN);
    m_levelDict.insert("Debug", DEB);
    m_levelDict.insert("Info", INF);
    m_levelDict.insert("Error", ERR);
}

/**
 * @brief LogApplicationParseThread::initMap 初始化等级数字和等级显示文本的map
 */
void LogApplicationParseThread::initJournalMap()
{
    m_journalMap.clear();
    m_journalMap.insert(0, DApplication::translate("Level", "Emergency"));
    m_journalMap.insert(1, DApplication::translate("Level", "Alert"));
    m_journalMap.insert(2, DApplication::translate("Level", "Critical"));
    m_journalMap.insert(3, DApplication::translate("Level", "Error"));
    m_journalMap.insert(4, DApplication::translate("Level", "Warning"));
    m_journalMap.insert(5, DApplication::translate("Level", "Notice"));
    m_journalMap.insert(6, DApplication::translate("Level", "Info"));
    m_journalMap.insert(7, DApplication::translate("Level", "Debug"));
}

/**
 * @brief journalAppWork::i2str 日志等级到等级显示文本的转换
 * @param prio 日志等级数字
 * @return 等级显示文
 */
QString LogApplicationParseThread::i2str(int prio)
{
    return m_journalMap.value(prio);
}

/**
 * @brief LogApplicationParseThread::initProccess 构造 QProcess成员指针
 */
void LogApplicationParseThread::initProccess()
{
    if (!m_process) {
        m_process = new QProcess;
    }
}

/**
 * @brief LogApplicationParseThread::run 线程执行虚函数重写逻辑
 */
void LogApplicationParseThread::run()
{
    qCDebug(logApp) << "threadrun";
    doWork();
}
