// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "journalbootwork.h"
#include "utils.h"

#include <DApplication>

#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QLoggingCategory>

#ifdef QT_DEBUG
Q_LOGGING_CATEGORY(logJournalboot, "org.deepin.log.viewer.parse.boot.journal.work")
#else
Q_LOGGING_CATEGORY(logJournalboot, "org.deepin.log.viewer.parse.boot.journal.work", QtInfoMsg)
#endif

DWIDGET_USE_NAMESPACE

std::atomic<JournalBootWork *> JournalBootWork::m_instance;
std::mutex JournalBootWork::m_mutex;
/**
 * @brief JournalBootWork::JournalBootWork 线程构造函数
 * @param arg 筛选参数
 * @param parent 父对象
 */
JournalBootWork::JournalBootWork(QStringList arg, QObject *parent)
    :  QObject(parent),
       QRunnable()
{
    //注册QList<LOG_MSG_JOURNAL>类型以让信号可以发出数据并能连接信号槽
    qRegisterMetaType<QList<LOG_MSG_JOURNAL> >("QList<LOG_MSG_JOURNAL>");
    //初始化等级数字对应显示文本的map
    initMap();
    //使用线程池启动该线程，跑完自己删自己
    setAutoDelete(true);
    //增加获取参数
    m_arg.append("-o");
    m_arg.append("json");
    if (!arg.isEmpty())
        m_arg.append(arg);
    //静态计数变量加一并赋值给本对象的成员变量，以供外部判断是否为最新线程发出的数据信号
    thread_index++;
    m_threadIndex = thread_index;
}

/**
 * @brief JournalBootWork::JournalBootWork 线程构造函数
 * @param parent 父对象
 */
JournalBootWork::JournalBootWork(QObject *parent)
    :  QObject(parent),
       QRunnable()
{
    qRegisterMetaType<QList<LOG_MSG_JOURNAL> >("QList<LOG_MSG_JOURNAL>");
    initMap();
    setAutoDelete(true);
    thread_index++;
    m_threadIndex = thread_index;
}

/**
 * @brief JournalBootWork::~JournalBootWork 析构时清空数据结构
 */
JournalBootWork::~JournalBootWork()
{
    logList.clear();
    m_map.clear();
}

/**
 * @brief JournalBootWork::stopWork 停止该线程
 */
void JournalBootWork::stopWork()
{
    m_canRun = false;
    qCDebug(logJournalboot) << "stopWork";
}

/**
 * @brief JournalBootWork::getIndex 获取当前对象的计数
 * @return 当前对象的计数标号
 */
int JournalBootWork::getIndex()
{
    return m_threadIndex;
}

/**
 * @brief JournalBootWork::getPublicIndex 获取现在此类产生对象的个数
 * @return 此类产生对象的个数，静态成员变量
 */
int JournalBootWork::getPublicIndex()
{
    return thread_index;
}

/**
 * @brief JournalBootWork::setArg 设置晒选参数
 * @param arg 筛选参数
 */
void JournalBootWork::setArg(QStringList arg)
{
    m_arg.clear();
    if (!arg.isEmpty())
        m_arg.append(arg);
}

/**
 * @brief JournalBootWork::run 线程执行函数
 */
void JournalBootWork::run()
{
    qCDebug(logJournalboot) << "threadrun";
    doWork();

}


/**
 * @brief JournalBootWork::doWork 实际的获取数据逻辑
 */
void JournalBootWork::doWork()
{
    //此线程刚开始把可以继续变量置true，不然下面没法跑
    m_canRun = true;
    mutex.lock();
    logList.clear();
    mutex.unlock();
    //如果线程外部被直接调用stopWork
    if ((!m_canRun)) {
        mutex.unlock();
        return;
    }

    int r;
    sd_journal *j ;
    if ((!m_canRun)) {
        mutex.unlock();
        return;
    }
    //打开日志文件
    r = sd_journal_open(&j, SD_JOURNAL_LOCAL_ONLY);
    if ((!m_canRun)) {
        mutex.unlock();
        sd_journal_close(j);
        return;
    }
    //r为系统借口返回值，小于0则表示失败，直接返回
    if (r < 0) {
        QString errostr = QString("Failed to open journal: %1").arg(r);
        qCWarning(logJournalboot) << errostr;
        emit  journalBootError(errostr);
        return;
    }
    //从尾部开始读，这样出来数据是倒叙，符合需求
    r = sd_journal_seek_tail(j);

    if (r < 0) {
        QString errostr = QString("Failed to seek tail journal: %1").arg(r);
        qCWarning(logJournalboot) << errostr;
        emit  journalBootError(errostr);
        return;
    }
    if ((!m_canRun)) {
        mutex.unlock();
        sd_journal_close(j);
        return;
    }

    if (!m_arg.isEmpty()) {
        //增加日志等级筛选
        QString _priority = m_arg.at(0);
        if (_priority != "all")
            r = sd_journal_add_match(j, m_arg.at(0).toStdString().c_str(), 0);
        if (r < 0) {
            QString errostr = QString("Failed to add match journal: %1").arg(r);
            qCWarning(logJournalboot) << errostr;
            emit  journalBootError(errostr);
            return;
        }
    }
    if ((!m_canRun)) {
        mutex.unlock();
        sd_journal_close(j);
        return;
    }

    char match[9 + 32 + 1] = "_BOOT_ID=";
    if ((!m_canRun)) {
        mutex.unlock();
        sd_journal_close(j);
        return;
    }
    sd_id128_t current_id;
    //获取当前最新的正在运行的bootid
    sd_id128_get_boot(&current_id);
    //拼接和把id转成字符串
    sd_id128_to_string(current_id, match + 9);
    if ((!m_canRun)) {
        mutex.unlock();
        sd_journal_close(j);
        return;
    }
    qCDebug(logJournalboot) << "journal match condition:" << match;
    //增加筛选条件
    r = sd_journal_add_match(j, match, sizeof(match) - 1);
    if (r < 0) {
        QString errostr = QString("Failed to add match journal: %1").arg(r);
        qCWarning(logJournalboot) << errostr;
        emit  journalBootError(errostr);
        return;
    }
    //合并以上两个筛选条件 (等级和bootid)
    r =   sd_journal_add_conjunction(j);
    if (r < 0) {
        QString errostr = QString("Failed to add conjunction journal: %1").arg(r);
        qCWarning(logJournalboot) << errostr;
        emit  journalBootError(errostr);
        return;
    }
    if ((!m_canRun)) {
        mutex.unlock();
        sd_journal_close(j);
        return;
    }
    int cnt = 0;
    //调用宏开始迭代
    SD_JOURNAL_FOREACH_BACKWARDS(j) {
        if ((!m_canRun)) {
            mutex.unlock();
            sd_journal_close(j);
            return;
        }
        const char *d;
        size_t l;

        LOG_MSG_JOURNAL logMsg;
        //获取时间
        r = sd_journal_get_data(j, "_SOURCE_REALTIME_TIMESTAMP", reinterpret_cast<const void **>(&d), &l);
        if (r < 0) {
            r = sd_journal_get_data(j, "__REALTIME_TIMESTAMP", reinterpret_cast<const void **>(&d), &l);
            if (r < 0) {
                continue;
            }
        }
        uint64_t t;
        sd_journal_get_realtime_usec(j, &t);
        //解锁返回字符串长度上限，默认是64k，写0为无限
        // sd_journal_set_data_threshold(j, 0);
        QString dt = getReplaceColorStr(d).split("=").value(1);
        if (m_arg.size() == 3) {
            if (t < static_cast<uint64_t>(m_arg.at(1).toLongLong()) || t > static_cast<uint64_t>(m_arg.at(2).toLongLong()))
                continue;
        }
        logMsg.dateTime = getDateTimeFromStamp(dt);
        //获取主机名
        r = sd_journal_get_data(j, "_HOSTNAME", reinterpret_cast<const void **>(&d), &l);
        if (r < 0)
            logMsg.hostName = "";
        else {
            QStringList strList =    getReplaceColorStr(d).split("=");
            strList.removeFirst();
            strList.join("=");
            logMsg.hostName = strList.join("=");
        }
        //获取进程号
        r = sd_journal_get_data(j, "_PID", reinterpret_cast<const void **>(&d), &l);
        if (r < 0)
            logMsg.daemonId = "";
        else {
            QStringList strList =    getReplaceColorStr(d).split("=");
            strList.removeFirst();
            logMsg.daemonId = strList.join("=");
        }
        //获取进程名
        r = sd_journal_get_data(j, "_COMM", reinterpret_cast<const void **>(&d), &l);
        if (r < 0) {
            logMsg.daemonName = "unknown";
            qCWarning(logJournalboot) << logMsg.daemonId << "error code" << r;
        } else {
            QStringList strList =    getReplaceColorStr(d).split("=");
            strList.removeFirst();
            logMsg.daemonName = strList.join("=");
        }

        //获取信息体
        r = sd_journal_get_data(j, "MESSAGE", reinterpret_cast<const void **>(&d), &l);
        if (r < 0) {
            logMsg.msg = "";
        } else {
            QStringList strList =    getReplaceColorStr(d).split("=");
            //出来的数据格式为 字段名= 信息体，但是因为信息体中也可能有=号，所以要把第一个去掉，后面的用=号拼起来
            strList.removeFirst();
            strList.join("=");
            logMsg.msg = strList.join("=");
        }
        //获取等级
        r = sd_journal_get_data(j, "PRIORITY", reinterpret_cast<const void **>(&d), &l);
        if (r < 0) {
            //有些时候的确会产生没有等级的日志，按照需求此时一律按调试处理，和journalctl 的筛选行为一致
            logMsg.level = i2str(7);
        } else {
            //获取等级为字段名= 数字 ，数字为0-7 ，对应紧急到调试，需要转换
            logMsg.level = i2str(getReplaceColorStr(d).split("=").value(1).toInt());
        }

        cnt++;
        mutex.lock();
        logList.append(logMsg);
        mutex.unlock();
        //每获得500个数据就发出信号给控件加载
        if (cnt % 500 == 0) {
            mutex.lock();
            emit journaBootlData(m_threadIndex, logList);
            logList.clear();
            //sleep(100);
            mutex.unlock();;
        }
    }
    //最后可能有余下不足500的数据
    if (logList.count() >= 0) {
        emit journaBootlData(m_threadIndex, logList);
    }

    emit journalBootFinished(m_threadIndex);
    //第一次加载时这个之后的代码都不执行?故放到最后
    sd_journal_close(j);
}

/**
 * @brief JournalBootWork::getReplaceColorStr 替换掉获取字符的颜色字符和特殊符号
 * @param d 原字符
 * @return  替换后的字符
 */
QString JournalBootWork::getReplaceColorStr(const char *d)
{
    QByteArray byteChar(d);
    byteChar = Utils::replaceEmptyByteArray(byteChar);
    QString d_str = QString(byteChar);
    d_str.replace(QRegExp("\\x1B\\[\\d+(;\\d+){0,2}m"), "");
    d_str.replace(QRegExp("\\002"), "");
    return  d_str;
}


/**
 * @brief JournalBootWork::getDateTimeFromStamp 通过获取的时间戳转换为格式化的时间显示文本
 * @param str 接口获取的原始时间字符
 * @return  格式化的时间显示文本
 */
QString JournalBootWork::getDateTimeFromStamp(const QString &str)
{
    QString ret = "";
    QString dtstr = str.left(str.length() - 6);
    QDateTime dt = QDateTime::fromTime_t(dtstr.toUInt());
    ret = dt.toString("yyyy-MM-dd hh:mm:ss");  // + QString(".%1").arg(ums);
    return ret;
}

/**
 * @brief JournalBootWork::initMap 初始化等级数字和等级显示文本的map
 */
void JournalBootWork::initMap()
{
    m_map.clear();
    m_map.insert(0, DApplication::translate("Level", "Emergency"));
    m_map.insert(1, DApplication::translate("Level", "Alert"));
    m_map.insert(2, DApplication::translate("Level", "Critical"));
    m_map.insert(3, DApplication::translate("Level", "Error"));
    m_map.insert(4, DApplication::translate("Level", "Warning"));
    m_map.insert(5, DApplication::translate("Level", "Notice"));
    m_map.insert(6, DApplication::translate("Level", "Info"));
    m_map.insert(7, DApplication::translate("Level", "Debug"));
}

/**
 * @brief JournalBootWork::i2str 日志等级到等级显示文本的转换
 * @param prio 日志等级数字
 * @return 等级显示文
 */
QString JournalBootWork::i2str(int prio)
{
    return m_map.value(prio);
}
