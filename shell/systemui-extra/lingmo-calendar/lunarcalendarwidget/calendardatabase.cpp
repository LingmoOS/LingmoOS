#include "calendardatabase.h"
#include <QSqlRecord>
#include <QFile>
#include <QList>
#include <QTemporaryFile>
#include <QProcess>
#include "schedulewidget.h"
/**
 * crontab的格式为  *   *   *   *   *   command
 * @param parent  分钟 小时 天  月   周这个不要直接* * * *  指令 即可
 * Notify-send 有的没有关闭按钮先需要运行一下/usr/lib/lingmo-notification-daemon/lingmo-notifications
 * 因为Lingmo-calendar-plugin 已经是常驻进程 所以用QTimer类定时也可以 因为QTimer对象不会被delete 除非用户killall
 * lingmo-calendar可以参考一下的del add update函数
 */
CalendarDataBase::CalendarDataBase(QObject *parent) : QObject(parent)
{
    display = getenv("DISPLAY");
    addShell();
    userName = getenv("USER"); //获取当前用户名
    xdg_runtime_dir = getenv("XDG_RUNTIME_DIR");
    XorWayland = getenv("XDG_SESSION_TYPE");
    qDebug() << XorWayland; //wayland 下应该是WAYLAND_DISPLAY=wayland-0 和X11的DISPLAY=:0不一样
    qDebug() << QSqlDatabase::drivers(); //当前环境支持哪些数据库
    m_dataBase = QSqlDatabase::addDatabase("QSQLITE");
    QString dirPath = QString(getenv("HOME")) + "/.config/.lingmo_calendar_Schedule2.db";

    QTextCodec::setCodecForLocale(QTextCodec::codecForLocale()); //设置显示中文
    m_dataBase.setDatabaseName(dirPath);

    connect(this, &CalendarDataBase::insertDataSignal, this, &CalendarDataBase::insertDataSlot);
    connect(this, &CalendarDataBase::deleteDataSignal, this, &CalendarDataBase::deleteDataSlot);
    connect(this, &CalendarDataBase::searchDataSignal, this, &CalendarDataBase::searchDataSlot);
    connect(this, &CalendarDataBase::updateDataSignal, this, &CalendarDataBase::updateDataSlot);
}
/**
 * @brief CalendarDataBase::deleteschedule
 * 删除自定义日期过了的日程
 */
void CalendarDataBase::deleteschedule()
{
    QSqlQuery query;
    if (!query.exec("SELECT id FROM Schedule WHERE endrepeat < datetime('now','+1 day')"))
    {
        qDebug() << "Error: Failed to execute query.";
        return;
    }
    QString queryResult;
    while (query.next())
    {
        queryResult += query.value(0).toString() + " ";
    }
    if (!queryResult.isEmpty())
    {
        processExpiredEvents(queryResult.trimmed());
    }
    else
    {
        qDebug() << "No expired events found.";
    }
}
/**
 * @brief CalendarDataBase::processExpiredEvents
 * @param expiredIds
 * 用临时文件去保存已经删除过期日程的crontab,QTemporaryFile会在程序执行结束之后自动删除
 */
void CalendarDataBase::processExpiredEvents(const QString& expiredIds)
{
    QTemporaryFile tempFile;
    if (!tempFile.open())
    {
        qDebug() << "Error: Failed to create temporary file.";
        return;
    }

    QTextStream out(&tempFile);
    QStringList crontabLines = getCurrentCrontab();
    for (const QString& line : crontabLines)
    {
        if(line.trimmed().isEmpty())
        {
            continue;
        }
        QStringList parts = line.split('#');
        if (parts.size() > 1 && expiredIds.contains(parts.last().trimmed()))
        {
            continue;
        }
        out << line << "\n";
    }

    tempFile.close();
    updateCrontabFromFile(tempFile.fileName());
}
/**
 * @brief CalendarDataBase::getCurrentCrontab
 * @return
 * 获取当前的crontab -l
 */
QStringList CalendarDataBase::getCurrentCrontab()
{
    QStringList crontabLines;
    QProcess process;
    process.start("crontab", QStringList() << "-l");
    process.waitForFinished();
    if (process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0)
    {
        QString output = process.readAllStandardOutput();
        crontabLines = output.split('\n');
    }
    else
    {
        qDebug() << "Error: Failed to read current crontab.";
    }
    return crontabLines;
}
/**
 * @brief CalendarDataBase::updateCrontabFromFile
 * @param filename
 * 更新删除完毕的crontab
 *
 */
void CalendarDataBase::updateCrontabFromFile(const QString& filename)
{
    QProcess process;
    process.start("crontab", QStringList() << filename);
    process.waitForFinished();
    if (process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0)
    {
        qDebug() << "Crontab updated successfully.";
    }
    else
    {
        qDebug() << "Error: Failed to update crontab.";
    }
}
/**
 * @brief CalendarDataBase::add
 * @param info
 * 先在数据库里面把原来的 都记录在一个临时文件里面 然后在这个临时文件新写一行
 * info.minute info.hour info.day info.month *星期不要 DISPLAY=:0 /usr/bin/dbus
 */
void CalendarDataBase::add(const MarkInfo &info)
{
    QString id = info.m_markId;
    QProcess *c = new QProcess(this);
    int code = c->execute("crontab -l");
    //如果返回的是0，说明用户从来没用过crontab 这个东西，要先把临时文件里面写入原来应该有的注释 再写自己的cron内容
    c->start("crontab -l");
    c->waitForFinished();
    QString cronFilePath = "/var/spool/cron/crontabs/" + userName;
    QString tmpFilePath = "/tmp/" + userName + ".tmp";
    QString crontabContent = c->readAllStandardOutput();

    QFile tmpFile(tmpFilePath);
    QString targetId = "#" + id;
    if (tmpFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&tmpFile);
        QStringList lines = crontabContent.split('\n'); // 将crontab内容按行分割
        for (const QString &line : lines)
        {
            //            if (!line.contains(targetId)) { // 排除注释行
            //                out << line;
            //            }
        }
        qDebug() << "code=" << code;
        if(code == 1) //没成功
        {
            QString old = "# Edit this file to introduce tasks to be run by cron. \n"
                          "#\n"
                          "# Each task to run has to be defined through a single line\n"
                          "# indicating with different fields when the task will be run\n"
                          "# and what command to run for the task\n"
                          "#\n"
                          "# To define the time you can provide concrete values for\n"
                          "# minute (m), hour (h), day of month (dom), month (mon),\n"
                          "# and day of week (dow) or use '*' in these fields (for 'any').\n"
                          "#\n"
                          "# Notice that tasks will be started based on the cron's system\n"
                          "# daemon's notion of time and timezones.\n"
                          "#\n"
                          "# Output of the crontab jobs (including errors) is sent through\n"
                          "# email to the user the crontab file belongs to (unless redirected).\n"
                          "#\n"
                          "# For example, you can run a backup of all your user accounts\n"
                          "# at 5 a.m every week with:\n"
                          "# 0 5 * * 1 tar -zcf /var/backups/home.tgz /home/\n"
                          "#\n"
                          "# For more information see the manual pages of crontab(5) and cron(8)\n"
                          "#\n"
                          "# m h  dom mon dow   command\n";
            out << old << "\n";
        }
        QString cronLine;
        //不重复 处理完毕
        cronLine = QString("%1 %2 %3 %4 * XDG_RUNTIME_DIR=%5 DISPLAY=%6 /usr/bin/notify-send -i lingmo-calendar --app-name=日历 \"%7\" \"今天 %8 %9:%10\" #%11")
                   .arg(info.Alarm_end_minute < 10 ? "0" + QString::number(info.Alarm_end_minute) : QString::number(info.Alarm_end_minute))
                   .arg(info.Alarm_end_hour < 10 ? "0" + QString::number(info.Alarm_end_hour) : QString::number(info.Alarm_end_hour))
                   .arg(info.day < 10 ? "0" + QString::number(info.day) : QString::number(info.day))
                   .arg(info.month < 10 ? "0" + QString::number(info.month) : QString::number(info.month))
                   .arg(xdg_runtime_dir)
                   .arg(display)
                   .arg(info.m_descript).arg(info.shangwu)
                   .arg(info.hour < 10 ? "0" + QString::number(info.hour) : QString::number(info.hour))
                   .arg(info.minute < 10 ? "0" + QString::number(info.minute) : QString::number(info.minute))
                   .arg(info.m_markId);
        if(AlarmType == 0)
        {
            cronLine = "";
        }
        if(AlarmType == 7)
        {
            cronLine = QString("%1 %2 %3 %4 * XDG_RUNTIME_DIR=%5 DISPLAY=%6 /usr/bin/notify-send -i lingmo-calendar --app-name=日历 \"%7\" \"明天 %8 %9:%10\" #%11")
                       .arg(info.Alarm_end_minute < 10 ? "0" + QString::number(info.Alarm_end_minute) : QString::number(info.Alarm_end_minute))
                       .arg(info.Alarm_end_hour < 10 ? "0" + QString::number(info.Alarm_end_hour) : QString::number(info.Alarm_end_hour))
                       .arg(info.day < 10 ? "0" + QString::number(info.day) : QString::number(info.day))
                       .arg(info.month < 10 ? "0" + QString::number(info.month) : QString::number(info.month))
                       .arg(xdg_runtime_dir)
                       .arg(display)
                       .arg(info.m_descript).arg(info.shangwu)
                       .arg(info.hour < 10 ? "0" + QString::number(info.hour) : QString::number(info.hour))
                       .arg(info.minute < 10 ? "0" + QString::number(info.minute) : QString::number(info.minute))
                       .arg(info.m_markId);
        }
        if(AlarmType == 10)
        {
            cronLine = QString("00 09 %1 %2 * XDG_RUNTIME_DIR=%3 DISPLAY=%4 /usr/bin/notify-send -i lingmo-calendar --app-name=日历 \"%5\" \"今天 全天 \" #%6")
                       .arg(info.day < 10 ? "0" + QString::number(info.day) : QString::number(info.day))
                       .arg(info.month < 10 ? "0" + QString::number(info.month) : QString::number(info.month))
                       .arg(xdg_runtime_dir)
                       .arg(display)
                       .arg(info.m_descript)
                       .arg(info.m_markId);
        }
        if(AlarmType == 9)
        {
            cronLine = QString("00 09 %1 %2 * XDG_RUNTIME_DIR=%3 DISPLAY=%4 /usr/bin/notify-send -i lingmo-calendar --app-name=日历 \"%5\" \"明天 全天\" #%6")
                       .arg(info.day < 10 ? "0" + QString::number(info.day) : QString::number(info.day))
                       .arg(info.month < 10 ? "0" + QString::number(info.month) : QString::number(info.month))
                       .arg(xdg_runtime_dir)
                       .arg(display)
                       .arg(info.m_descript)
                       .arg(info.m_markId);
        }
        if(AlarmType == 8)
        {
            cronLine = QString("%1 %2 %3 %4 * XDG_RUNTIME_DIR=%5 DISPLAY=%6 /usr/bin/notify-send -i lingmo-calendar --app-name=日历 \"%7\" \" %8 %9 %10:%11\" #%12")
                       .arg(info.Alarm_end_minute < 10 ? "0" + QString::number(info.Alarm_end_minute) : QString::number(info.Alarm_end_minute))
                       .arg(info.Alarm_end_hour < 10 ? "0" + QString::number(info.Alarm_end_hour) : QString::number(info.Alarm_end_hour))
                       .arg(info.day < 10 ? "0" + QString::number(info.day) : QString::number(info.day))
                       .arg(info.month < 10 ? "0" + QString::number(info.month) : QString::number(info.month))
                       .arg(xdg_runtime_dir)
                       .arg(display)
                       .arg(info.m_descript).arg(info.m_markStartDate.toString("yyyy/MM/dd")).arg(info.shangwu)
                       .arg(info.hour < 10 ? "0" + QString::number(info.hour) : QString::number(info.hour))
                       .arg(info.minute < 10 ? "0" + QString::number(info.minute) : QString::number(info.minute))
                       .arg(info.m_markId);
        }
        if(AlarmType == 11 || AlarmType == 12)
        {
            cronLine = QString("00 09 %1 %2 * XDG_RUNTIME_DIR=%3 DISPLAY=%4 /usr/bin/notify-send -i lingmo-calendar --app-name=日历 \"%5\" \" %6 全天\" #%7")
                       .arg(info.day < 10 ? "0" + QString::number(info.day) : QString::number(info.day))
                       .arg(info.month < 10 ? "0" + QString::number(info.month) : QString::number(info.month))
                       .arg(xdg_runtime_dir)
                       .arg(display)
                       .arg(info.m_descript).arg(info.m_markStartDate.toString("yyyy/MM/dd"))
                       .arg(info.m_markId);
        }
        if(RRuleType == 1)
        {
            cronLine = QString("%1 %2 * * * XDG_RUNTIME_DIR=%3 DISPLAY=%4 /usr/bin/notify-send -i lingmo-calendar --app-name=日历 \"%5\" \" %6 %7 %8:%9\" #%10")
                       .arg(info.Alarm_end_minute < 10 ? "0" + QString::number(info.Alarm_end_minute) : QString::number(info.Alarm_end_minute))
                       .arg(info.Alarm_end_hour < 10 ? "0" + QString::number(info.Alarm_end_hour) : QString::number(info.Alarm_end_hour))
                       .arg(xdg_runtime_dir)
                       .arg(display)
                       .arg(info.m_descript).arg(info.m_markStartDate.toString("yyyy/MM/dd")).arg(info.shangwu)
                       .arg(info.hour < 10 ? "0" + QString::number(info.hour) : QString::number(info.hour))
                       .arg(info.minute < 10 ? "0" + QString::number(info.minute) : QString::number(info.minute))
                       .arg(info.m_markId);
        }
        else if(RRuleType == 3)
        {
            cronLine = QString("%1 %2 %3 * * XDG_RUNTIME_DIR=%4 DISPLAY=%5 /usr/bin/notify-send -i lingmo-calendar --app-name=日历 \"%6\" \" %7 %8 %9:%10\" #%11")
                       .arg(info.Alarm_end_minute < 10 ? "0" + QString::number(info.Alarm_end_minute) : QString::number(info.Alarm_end_minute))
                       .arg(info.Alarm_end_hour < 10 ? "0" + QString::number(info.Alarm_end_hour) : QString::number(info.Alarm_end_hour))
                       .arg(info.day < 10 ? "0" + QString::number(info.day) : QString::number(info.day))
                       .arg(xdg_runtime_dir)
                       .arg(display)
                       .arg(info.m_descript).arg(info.m_markStartDate.toString("yyyy/MM/dd")).arg(info.shangwu)
                       .arg(info.hour < 10 ? "0" + QString::number(info.hour) : QString::number(info.hour))
                       .arg(info.minute < 10 ? "0" + QString::number(info.minute) : QString::number(info.minute))
                       .arg(info.m_markId);
        }
        else if(RRuleType == 2)
        {
            cronLine = QString("%1 %2 * * %3 XDG_RUNTIME_DIR=%4 DISPLAY=%5 /usr/bin/notify-send -i lingmo-calendar --app-name=日历 \"%6\" \" %7 %8 %9:%10\" #%11")
                       .arg(info.Alarm_end_minute < 10 ? "0" + QString::number(info.Alarm_end_minute) : QString::number(info.Alarm_end_minute))
                       .arg(info.Alarm_end_hour < 10 ? "0" + QString::number(info.Alarm_end_hour) : QString::number(info.Alarm_end_hour))
                       .arg(info.week)
                       .arg(xdg_runtime_dir)
                       .arg(display)
                       .arg(info.m_descript).arg(info.m_markStartDate.toString("yyyy/MM/dd")).arg(info.shangwu)
                       .arg(info.hour < 10 ? "0" + QString::number(info.hour) : QString::number(info.hour))
                       .arg(info.minute < 10 ? "0" + QString::number(info.minute) : QString::number(info.minute))
                       .arg(info.m_markId);
        }
        else if(RRuleType == 4)
        {
            //如果2025的这个时候也提醒了 可以加一个info的年属性 判断一下 一年一换 crontab好像没有年的处理方式
            cronLine = QString("%1 %2 %3 %4 * XDG_RUNTIME_DIR=%5 DISPLAY=%6 /usr/bin/notify-send -i lingmo-calendar --app-name=日历 \"%7\" \" %8 %9 %10:%11\" #%12")
                       .arg(info.Alarm_end_minute < 10 ? "0" + QString::number(info.Alarm_end_minute) : QString::number(info.Alarm_end_minute))
                       .arg(info.Alarm_end_hour < 10 ? "0" + QString::number(info.Alarm_end_hour) : QString::number(info.Alarm_end_hour))
                       .arg(info.day < 10 ? "0" + QString::number(info.day) : QString::number(info.day))
                       .arg(info.month < 10 ? "0" + QString::number(info.month) : QString::number(info.month))
                       .arg(xdg_runtime_dir)
                       .arg(display)
                       .arg(info.m_descript).arg(info.m_markStartDate.toString("yyyy/MM/dd")).arg(info.shangwu)
                       .arg(info.hour < 10 ? "0" + QString::number(info.hour) : QString::number(info.hour))
                       .arg(info.minute < 10 ? "0" + QString::number(info.minute) : QString::number(info.minute))
                       .arg(info.m_markId);
        }
        out << cronLine << "\n";
        tmpFile.close();
    }
    else
    {
        qDebug() << "无法新建临时文件" << tmpFile.errorString();
    }
    // 步骤3: 使用crontab命令将tmp文件内容设置为当前用户的crontab
    QProcess *process = new QProcess(this);

    QString  command = QString("(crontab -l 2>/dev/null; cat %1) | crontab -").arg(tmpFilePath);
    QString program = "sh";
    QStringList arguments;
    arguments << "-c" << command;
    process->start(program, arguments);
    // 等待进程结束并检查其退出状态
    if (!process->waitForFinished())
    {
        qDebug() << "Error occurred while executing crontab command.";
        return;
    }
    else
    {
        int exitCode = process->exitCode();
        QString standardOutput = process->readAllStandardOutput();
        QString standardError = process->readAllStandardError();
        if (exitCode != 0)
        {
            qDebug() << "crontab command exited with code" << exitCode;
            qDebug() << "Standard Output:" << standardOutput;
            qDebug() << "Standard Error:" << standardError;
            return ;
        }
        else
        {
            qDebug() << "crontab command executed successfully.";
        }
        QFile::remove(tmpFilePath);
        delete process; //清对象
        delete c;
    }
}
/**
 * @brief CalendarDataBase::update
 * @param info
 * 因为保存的格式是 * * * * * DISPLAY=:0 /usr/bin/dbus #id 遍历数据库的#后面 匹配到日程id 就删除这一行
 * 然后加上新的一行 再把这个tmp临时文件crontab txt保存
 *
 */
void CalendarDataBase::update(const MarkInfo &info)
{
    QString id = info.m_markId;
    del(info);
    add(info);
}
/**
 * @brief CalendarDataBase::del
 * @param info
 * 遍历crontab 的#id后面的数字 如果相同 遍历每一行的#id
 */
void CalendarDataBase::del(const MarkInfo &info)
{
    QProcess *c = new QProcess(this);
    c->start("crontab -l");
    c->waitForFinished();
    QString cronFilePath = "/var/spool/cron/crontabs/" + userName;
    QString tmpFilePath = "/tmp/" + userName + ".tmp";
    QString crontabContent = c->readAllStandardOutput();
    QString id = info.m_markId;
    QString  fileName = "/var/spool/cron/crontabs/" + userName;
    //步骤一 打开文件
    QFile file(fileName);
    //步骤二 创建一个临时文件用于存储不包含指定ID的行
    QFile tempFile("/tmp/" + userName + ".tmp");
    if (!tempFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "无法创建临时文件";
        file.close();
        return;
    }
    //步骤三 准备遍历
    //   QTextStream in(&file);
    QTextStream out(&tempFile);
    //   QString line;
    QStringList lines = crontabContent.split('\n');
    QString targetId = "#" + id; // 加上#前缀来匹配文件中的格式
    //开始遍历
    for (const QString &line : lines)
    {
        if (!line.contains(targetId) || line.startsWith("#"))   // 排除注释行 并且保留最开始的注释
        {
            out << line << "\n";
        }
    }
    //   file.close();
    tempFile.close();
    // 删除原文件并将临时文件重命名为原文件名
    QProcess *process = new QProcess(this);
    QString  command = QString("crontab %1").arg(tmpFilePath);
    QString program = "sh";
    QStringList arguments;
    arguments << "-c" << command;
    process->start(program, arguments);
    // 等待进程结束并检查其退出状态
    if (!process->waitForFinished())
    {
        qDebug() << "Error occurred while executing crontab command.";
        return;
    }
    else
    {
        int exitCode = process->exitCode();
        QString standardOutput = process->readAllStandardOutput();
        QString standardError = process->readAllStandardError();
        if (exitCode != 0)
        {
            qDebug() << "crontab command exited with code" << exitCode;
            qDebug() << "Standard Output:" << standardOutput;
            qDebug() << "Standard Error:" << standardError;
            return ;
        }
        else
        {
            qDebug() << "crontab command executed successfully.";
        }
        QFile::remove(tmpFilePath);
        delete process; //清对象
    }
}
int CalendarDataBase::initDataBase()
{
    if (!m_dataBase.open())   //如果数据库打开失败，会弹出一个警告窗口;由于此类非指针类型因此是0
    {

        QMessageBox::warning(0, QObject::tr("Database Error"), m_dataBase.lastError().text());
        m_dbStatus = DB_UNCONNECT;
        return DB_UNCONNECT;

    }
    else
    {

        QSqlQuery query;
        QString creatTableStr = "CREATE TABLE IF NOT EXISTS Schedule  "
                                "(id          TEXT  NOT NULL,"
                                "start_day    TEXT  NOT NULL,"
                                "start_month  TEXT  NOT NULL,"
                                "start_hour   TEXT  NOT NULL,"
                                "start_minute TEXT  NOT NULL,"
                                "end_hour     TEXT  NOT NULL,"
                                "end_minute   TEXT  NOT NULL,"
                                "end_day      TEXT  NOT NULL,"
                                "end_month    TEXT  NOT NULL,"
                                "descript     TEXT  NOT NULL,"
                                "start_date   TEXT  NOT NULL,"
                                "end_date     TEXT  NOT NULL,"
                                "remind       TEXT  NOT NULL,"
                                "repeat       TEXT  NOT NULL,"
                                "beginrepeat  TEXT  NOT NULL,"
                                "endrepeat    DATETIME  NOT NULL,"
                                "beginDateTime DATETIME NOT NULL,"
                                "endDateTime   DATETIME NOT NULL,"
                                "isAllDay     BOOLEAN NOT NULL,"
                                "isLunar      BOOLEAN NOT NULL,"
                                "allDayremind TEXT   NOT NULL);";

        query.prepare(creatTableStr);
        if (!query.exec())
        {
            qDebug() << "建表失败!" << query.lastError();
            m_dbStatus = CREATE_TABLE_FAILED;
            return CREATE_TABLE_FAILED;
        }
        else
        {
            qDebug() << "建表成功!";
            m_dbStatus = DB_OP_SUCC;
            return DB_OP_SUCC;
        }
    }
}
int CalendarDataBase::insertDataSlot(const MarkInfo &info)
{
    QString uniqueId = info.m_markId;
    QString startDateStr = info.m_markStartDate.toString("yyyy-MM-dd");
    QString endDateStr = info.m_markEndDate.toString("yyyy-MM-dd");
    int day =  info.day;
    int month = info.month;
    int hour =  info.hour;

    int minute =  info.minute;
    int end_hour = info.end_hour;
    int end_minute = info.end_minute;
    int end_day = info.end_day;
    int end_month = info.end_month;
    QString longtime = info.timeLong;
    QString descript = info.m_descript;
    QString remind = info.remind;
    QString repeat = info.repeat;
    QString beginrepeat = info.beginrepeat;
    QDateTime endrepeat = info.endrepeat;
    QDateTime beginDateTime = info.beginDateTime;
    QDateTime endDateTime = info.endDateTime;
    bool isAllDay = info.m_isAllDay;
    bool isLunar = info.m_isLunar;
    QString alldayremind = info.AlldayRemind;
    QString cmd = QString("insert into Schedule (%1) values('%2','%3','%4','%5','%6','%7','%8','%9','%10','%11','%12','%13','%14','%15','%16','%17','%18','%19','%20','%21','%22')")
                  .arg(ITEM_FIELD)
                  .arg(uniqueId)
                  .arg(day)
                  .arg(month)
                  .arg(hour)
                  .arg(minute)
                  .arg(end_hour)
                  .arg(end_minute)
                  .arg(end_day)
                  .arg(end_month).arg(descript).arg(startDateStr).arg(endDateStr).arg(remind).arg(repeat).arg(beginrepeat).arg(endrepeat.toString(Qt::ISODate)).arg(beginDateTime.toString(Qt::ISODate)).arg(endDateTime.toString(Qt::ISODate)).arg(isAllDay).arg(isLunar).arg(alldayremind);
    QSqlQuery query;
    if (!query.exec(cmd))
    {
        qDebug() << "query error :" << query.lastError();
        m_dbStatus = DB_OP_ADD_FAILED;
        return DB_OP_ADD_FAILED;
    }
    else
    {
        qDebug() << "插入成功!";
        m_dbStatus = DB_OP_SUCC;
        return DB_OP_SUCC;
    }
    //    }
}
//当天查看当天的日程 然后把m_markInfoList 传出来 再把每个日程信息拿出来传给主窗口的日程信息
int CalendarDataBase::searchDataSlot(const QDate &date)
{
    QString startDateStr = date.toString("yyyy-MM-dd");
    //    int begin_day = ;
    QString searchId;
    QDate startDate;
    QDate endDate;
    int start_hour;
    int start_minute;
    int end_hour;
    int end_minute;
    QString timeLong; //
    QString descript;
    QString remind;
    QString repeat;
    QString beginrepeat;
    QDateTime endrepeat;
    QDateTime beginDateTime;
    QDateTime endDateTime;
    QString alldayremind;
    bool isAllDay;
    bool isLunar;
    MarkInfo resultInfo;
    if (!m_dataBase.open())   //如果数据库打开失败，会弹出一个警告窗口;由于此类非指针类型因此是0
    {

        QMessageBox::warning(0, QObject::tr("Database Error"), m_dataBase.lastError().text());
        m_dbStatus = DB_UNCONNECT;
        return DB_UNCONNECT;

    }
    else
    {
        //日程的展示需要的是开始时间-结束时间 时长 内容  例如09:30-10:00 30分钟 讨论会
        QString searchCmd =
            QString("select id,start_date,start_hour,start_minute,end_hour,end_minute,descript,remind,repeat,beginrepeat,endrepeat,beginDateTime,endDateTime,isAllDay,isLunar,end_date,allDayremind from Schedule where start_date = '%1'").arg(startDateStr);
        QSqlQuery searchQuery;
        searchQuery.exec(searchCmd);
        if (!searchQuery.exec(searchCmd))
        {
            qDebug() << "query error :" << searchQuery.lastError();
            m_dbStatus = DB_OP_GET_FAILED;
            return DB_OP_GET_FAILED;

        }
        else
        {
            m_markIdList.clear();
            m_markInfoList.clear();
            while (searchQuery.next())
            {
                searchId = searchQuery.value(0).toString();
                startDate = searchQuery.value(1).toDate();
                start_hour = searchQuery.value(2).toInt(); //9
                start_minute = searchQuery.value(3).toInt(); //30
                end_hour = searchQuery.value(4).toInt(); //10
                end_minute = searchQuery.value(5).toInt(); //0
                timeLong = handleTimeLong(intToTime(start_hour, start_minute), intToTime(end_hour, end_minute));
                //                endDate = searchQuery.value(2).toDate();
                descript = searchQuery.value(6).toString();
                remind = searchQuery.value(7).toString();
                repeat = searchQuery.value(8).toString();
                beginrepeat = searchQuery.value(9).toString();
                endrepeat = searchQuery.value(10).toDateTime();
                beginDateTime =  searchQuery.value(11).toDateTime();
                endDateTime =  searchQuery.value(12).toDateTime();
                isAllDay = searchQuery.value(13).toBool();
                isLunar = searchQuery.value(14).toBool();

                endDate = searchQuery.value(15).toDate();
                alldayremind = searchQuery.value(16).toString();
                resultInfo.m_markId = searchId;
                resultInfo.m_markStartDate = startDate;
                resultInfo.hour = start_hour;
                resultInfo.minute = start_minute;
                resultInfo.end_hour = end_hour;
                resultInfo.end_minute = end_minute;
                resultInfo.timeLong = timeLong;
                resultInfo.m_descript = descript;
                resultInfo.remind = remind;
                resultInfo.repeat = repeat;
                resultInfo.beginrepeat = beginrepeat;
                resultInfo.endrepeat = endrepeat;
                resultInfo.beginDateTime = beginDateTime;
                resultInfo.endDateTime = endDateTime;
                resultInfo.m_isAllDay = isAllDay;
                resultInfo.m_isLunar = isLunar;
                resultInfo.m_markEndDate = endDate;
                resultInfo.AlldayRemind = alldayremind;

                m_markIdList.append(searchId);
                m_markInfoList.append(resultInfo);
            }
            qDebug() << m_markIdList;
            if (searchId != "")
            {
                qDebug() << "找到符合条件的id一共有" << m_markIdList.count() << "条数据";
                m_dbStatus = DB_OP_SUCC;
                return DB_OP_SUCC;
            }
            else
            {
                m_dbStatus = DB_OP_GET_FAILED;
                return DB_OP_GET_FAILED;
            }
        }
    }
}

int CalendarDataBase::deleteDataSlot(const MarkInfo &info)
{
    QString markId = info.m_markId;
    if (!m_dataBase.open())   //如果数据库打开失败，会弹出一个警告窗口;由于此类非指针类型因此是0
    {

        QMessageBox::warning(0, QObject::tr("Database Error"), m_dataBase.lastError().text());
        m_dbStatus = DB_UNCONNECT;
        return DB_UNCONNECT;

    }
    else
    {

        if (markId != "")
        {

            QString cmd = QString("delete from Schedule where id = '%1'").arg(markId);
            QSqlQuery query;
            if (!query.exec(cmd))
            {
                qDebug() << "query error :" << query.lastError();
                m_dbStatus = DB_OP_DEL_FAILED;
                return DB_OP_DEL_FAILED;
            }
            else
            {
                qDebug() << "删除成功!";
                m_dbStatus = DB_OP_SUCC;
                return DB_OP_SUCC;
            }

        }
        else
        {

            m_dbStatus = SCHEDULE_UNFOUND;
            return SCHEDULE_UNFOUND;
        }
    }
}
/**
 * @brief CalendarDataBase::updateDataSlot
 * @param info
 * @return 更新主要改的是开始时间和结束时间，未来的lingmo-calendar 的所有日程的item 用的是数据库里面的开始时间和结束时间来计算长度的
 * 每一次更新 /删除 /添加都是重新 获取一遍数据库的内容来重新渲染当前页面 包括视图 切换 也是堆栈Widget再重新获取数据渲染
 */
int CalendarDataBase::updateDataSlot(const MarkInfo &info)
{
    QString uniqueId = info.m_markId;
    QString startDateStr = info.m_markStartDate.toString("yyyy-MM-dd");
    QString endDateStr = info.m_markEndDate.toString("yyyy-MM-dd");
    int day =  info.day;
    int month = info.month;
    int hour =  info.hour;
    int minute =  info.minute;
    int end_hour = info.end_hour;
    int end_minute = info.end_minute;
    int end_day = info.end_day;
    int end_month = info.end_month;
    QString descript = info.m_descript;
    QString longtime = info.timeLong;
    QString remind = info.remind;
    QString repeat = info.repeat;
    QString beginrepeat = info.beginrepeat;
    QDateTime endrepeat = info.endrepeat;
    QDateTime beginDateTime = info.beginDateTime;
    QDateTime endDateTime = info.endDateTime;
    bool isAllDay = info.m_isAllDay;
    bool isLunar = info.m_isLunar;

    QString alldayremind = info.AlldayRemind;
    if (!m_dataBase.open())   //如果数据库打开失败，会弹出一个警告窗口;由于此类非指针类型因此是0
    {
        QMessageBox::warning(0, QObject::tr("Database Error"), m_dataBase.lastError().text());
        m_dbStatus = DB_UNCONNECT;
        return DB_UNCONNECT;
    }
    else
    {
        if (uniqueId != "")
        {
            QString cmd =
                QString("update Schedule set start_day='%1',start_month='%2',start_hour='%3',start_minute='%4',end_hour='%5',end_minute='%6',end_day='%7',end_month='%8',descript='%9',start_date='%10',end_date='%11',remind='%12',repeat='%13',beginrepeat='%14',endrepeat='%15',beginDateTime='%16',endDateTime='%17',isAllDay='%18',isLunar='%19',allDayremind='%20' where id = '%21'")
                .arg(day)
                .arg(month)
                .arg(hour)
                .arg(minute)
                .arg(end_hour)
                .arg(end_minute)
                .arg(end_day)
                .arg(end_month)
                .arg(descript)
                .arg(startDateStr).arg(endDateStr).arg(remind).arg(repeat).arg(beginrepeat).arg(endrepeat.toString(Qt::ISODate)).arg(beginDateTime.toString(Qt::ISODate)).arg(endDateTime.toString(Qt::ISODate)).arg(isAllDay).arg(isLunar).arg(alldayremind)
                .arg(uniqueId);
            qDebug() << "cmd:" << cmd;
            QSqlQuery query;
            if (!query.exec(cmd))
            {
                qDebug() << "query error :" << query.lastError();
                m_dbStatus = DB_OP_ALTER_FAILED;
                return DB_OP_ALTER_FAILED;
            }
            else
            {
                qDebug() << "修改成功!";
                m_dbStatus = DB_OP_SUCC;
                return DB_OP_SUCC;
            }

        }
        else
        {

            m_dbStatus = SCHEDULE_UNFOUND;
            return SCHEDULE_UNFOUND;
        }
    }
}
/**
 * @brief CalendarDataBase::convertToCronFormat
 * @param record
 * @return 将得到的数据库内容转换成crontab格式 如果要添加日程id 可以在最后加一个#id
 * 类似于* * * * * command #123 这样123不会影响正常cron服务，以后直接判断#后面的id 就删除单行
 */
QString CalendarDataBase::convertToCronFormat(const QSqlRecord &record)
{
    QString minute = record.value("minute").toString();
    QString hour = record.value("hour").toString();
    QString dayOfMonth = record.value("day_of_month").toString();
    QString month = record.value("month").toString();
    QString dayOfWeek = record.value("day_of_week").toString();
    QString command = record.value("command").toString();

    // 如果某个字段为空，则使用星号(*)作为占位符
    if (minute.isEmpty()) minute = "*";
    if (hour.isEmpty()) hour = "*";
    if (dayOfMonth.isEmpty()) dayOfMonth = "*";
    if (month.isEmpty()) month = "*";
    if (dayOfWeek.isEmpty()) dayOfWeek = "*";

    // 构建cron格式的字符串
    QString cronString = QString("%1 %2 %3 %4 %5 %6")
                         .arg(minute)
                         .arg(hour)
                         .arg(dayOfMonth)
                         .arg(month)
                         .arg(dayOfWeek)
                         .arg(command);
    return cronString;
}
/**
 * @brief CalendarDataBase::getAllCronJobs
 * @return 获取所有的数据
 */
QStringList CalendarDataBase::getAllCronJobs()
{
    QSqlQuery query;
    QString selectCronJobsStr = "SELECT start_time, end_time, start_date, end_date, command FROM Schedule;";
    query.prepare(selectCronJobsStr);

    if (!query.exec())
    {
        qDebug() << "查询cron作业失败!";
        m_dbStatus = CREATE_TABLE_FAILED;
        return QStringList(); //返回一个空的
    }
    else
    {
        QStringList cronJobs;
        while (query.next())
        {
            QString cronJob = convertToCronFormat(query.record());
            cronJobs.append(cronJob);
        }
        qDebug() << "查询cron作业成功!";
        m_dbStatus = DB_OP_SUCC;
        return cronJobs;
    }
}
DB_RETURN_STATUS CalendarDataBase::getStatus()
{
    return m_dbStatus;
}

void CalendarDataBase::setMarkInfo(const MarkInfo &info)
{
    m_info = info;
}

MarkInfo CalendarDataBase::getMarkInfo()
{
    return m_info;
}

QStringList CalendarDataBase::getMarkIdList()
{
    return m_markIdList;
}

QList<MarkInfo> CalendarDataBase::getMarkInfoList()
{
    return m_markInfoList;
}

CalendarDataBase::~CalendarDataBase()
{
    qDebug() << "析构RecorderDataBase";
    if (m_dataBase.open())
    {
        m_dataBase.close();
    }
}
/**
 * @brief CalendarDataBase::intToTime
 * @param hour
 * @param minute
 * @return 时间和小时组合成时间
 */
QTime CalendarDataBase::intToTime(int hour, int minute)
{
    QTime time(hour, minute); //9,0 -> 9:00
    return time;
}
/**
 * @brief CalendarDataBase::handleTimeLong
 * @param time1
 * @param time2
 * @return
 */
QString CalendarDataBase::handleTimeLong(QTime time1, QTime time2)
{
    //确保结束时间大于开始时间
    if (time1 > time2)
    {
        QTime temp = time1;
        time1 = time2;
        time2 = temp;
    }
    // 计算时间差（以毫秒为单位）
    int msecs = time1.msecsTo(time2);
    // 转换为小时和分钟
    int hours = msecs / (1000 * 60 * 60);
    int mins = (msecs % (1000 * 60 * 60)) / (1000 * 60);
    QString hour = QString::number(hours);
    QString min = QString::number(mins);
    //       QString result = hour + "小时" + min +"分钟";
    //构造结果字符串
    QString result;
    if (hours > 0)
    {
        result = hour + tr("hour") + min + tr("minute");
    }
    else
    {
        result = min + tr("minute");
    }
    if(mins == 0)
    {
        result = hour + tr("hour");
    }
    return result;
}
QStringList CalendarDataBase::getIdList()
{
    return m_markIdList;
}
QList<MarkInfo> CalendarDataBase::getInfoList()
{
    return m_markInfoList;
}
void CalendarDataBase::addShell()
{
}
int CalendarDataBase::deleteDataSlotfromMarkID(QString markID)
{
    QString markId = markID;
    if (!m_dataBase.open())   //如果数据库打开失败，会弹出一个警告窗口;由于此类非指针类型因此是0
    {

        QMessageBox::warning(0, QObject::tr("Database Error"), m_dataBase.lastError().text());
        m_dbStatus = DB_UNCONNECT;
        return DB_UNCONNECT;

    }
    else
    {

        if (markId != "")
        {

            QString cmd = QString("delete from Schedule where id = '%1'").arg(markId);
            QSqlQuery query;
            if (!query.exec(cmd))
            {
                qDebug() << "query error :" << query.lastError();
                m_dbStatus = DB_OP_DEL_FAILED;
                return DB_OP_DEL_FAILED;
            }
            else
            {
                qDebug() << "删除成功!";
                m_dbStatus = DB_OP_SUCC;
                return DB_OP_SUCC;
            }

        }
        else
        {

            m_dbStatus = SCHEDULE_UNFOUND;
            return SCHEDULE_UNFOUND;
        }
    }
}
