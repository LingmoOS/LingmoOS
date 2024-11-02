#include "cgroup-helper.h"
#include <QDir>
#include <QDebug>
#include <QtDBus/QDBusVariant>
#include <QtDBus/QDBusPendingReply>
#include <QCoreApplication>
#include "linux/magic.h"
#include "sys/statfs.h"

//cgroup根路径
const char *cgroup_path = "/sys/fs/cgroup/";
//lingmo-ProcessManagerDaemon
const char *daemon_dbus_service = "com.lingmo.ProcessManagerDaemon";
const char *daemon_dbus_path = "/com/lingmo/ProcessManagerDaemon";
const char *daemon_dbus_interface = "com.lingmo.ProcessManagerDaemon";
//lingmo-process-manager
const char *kpm_dbus_service = "com.lingmo.ProcessManager";
const char *kpm_dbus_path = "/com/lingmo/ProcessManager/AppWhitelist";
const char *kpm_dbus_interface = "com.lingmo.ProcessManager.AppWhitelist";
const char *desktop_path = "/etc/xdg/autostart/lingmo-search-service.desktop";

void uniquePtrDeleter(QDBusInterface* ptr) {
    if(ptr) {
        delete ptr;
        ptr = nullptr;
    }
}

//获取cgroup的版本，与内核版本有关，分为cgroupV1和cgroupV2
int CgroupHelper::getCgroupVerson()
{
    struct statfs fs;
    statfs(cgroup_path, &fs);

    if (fs.f_type == TMPFS_MAGIC) {
        return 1;
    } else if (fs.f_type == CGROUP2_SUPER_MAGIC) {
        return 2;
    }
    return -1;
}

QString CgroupHelper::getUserPathV2(QString groupPath)
{
    QDir dir(groupPath);
    QStringList lst = dir.entryList();
    for(QString str : lst) {
        if((str.contains("user.") || str.contains("user-") || str.contains("user@"))&& (str.endsWith(".slice") || str.endsWith(".service"))) {
            groupPath = groupPath + str + "/";
            return getUserPathV2(groupPath);
        }
    }
    return groupPath;
}

QString CgroupHelper::getPath()
{
    if(getCgroupVerson() == 1) {
        //cgroupV1下cgroup分组相对/sys/fs/cgroup/controller的相对路径，cgroupV1层次较为简单，所有controller均在cgroup层下
        //创建的cgroup新分组需要在controller层下，例如/sys/fs/cgroup/cpu/lingmo-search-service
        //controller为cpu,相对路径为cpu之后的/lingmo-search-service

        return "/lingmo-search-service";
    } else if (getCgroupVerson() == 2) {
        //cgroupV2下cgroup分组相对/sys/fs/cgroup的相对路径
        //通常情况下需要在/sys/fs/cgroup/user.slice/user-1000.slice/user@1000.service/session.slice之后创建新cgroup分组
        //通过cgroup根路径获取中间三层用户路径

        QString path = getUserPathV2(cgroup_path);
        path = path + "session.slice/lingmo-search-service";
        //将路径转换为绝对路径
        path.replace(cgroup_path, "/");
        return path;
    }
    return "";
}

bool CgroupHelper::createProcessGroup(QDBusInterface* interface)
{
    if(!interface) {
        qDebug()<<"get dbusinterface failed";
        return false;
    }
    QString path = getPath();
    QStringList controllers = {"cpu"};
    QList<int> pids;
    QList<QVariant> argumentList;
    argumentList<< QVariant::fromValue((path))//cgroup分组路径,传相对路径
                << QVariant::fromValue((controllers))//controller常用的有cpu,memory
                << QVariant::fromValue(pids);//进程pid，创建时可以不传
    QDBusPendingReply<bool> reply = interface->callWithArgumentList(QDBus::Block, "CreateProcessGroup", argumentList);
    if (!reply.isValid() || reply.isError()) {
        QDBusError error = reply.error();
        qDebug()<<"reply.isValid()" << reply.isValid() << "reply.error()" << reply.isError() << reply.error();
        return false;
    }
    return reply.value();
}

bool CgroupHelper::moveProcessToGroup(QDBusInterface *interface)
{
    if(!interface) {
        qDebug()<<"get dbusinterface failed";
        return false;
    }
    QString path = getPath();
    QStringList controllers = {"cpu"};
    int pid = QCoreApplication::applicationPid();
    QList<int> pids = {pid};
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue((path)) //cgroup分组路径，传相对路径
                 << QVariant::fromValue((controllers))//controller常用的有cpu,memory
                 << QVariant::fromValue((pids));//进程pid

    QDBusPendingReply<bool> reply = interface->callWithArgumentList(QDBus::Block, "MoveProcessToGroup", argumentList);
    if (!reply.isValid() || reply.isError()) {
        qDebug()<<"reply.isValid()" << reply.isValid() << "reply.error()" << reply.isError() << reply.error();
        return false;
    }
    //cgroupV1版本下lingmo-ProcessManagerDaemon没有GetGroupPids接口
    //cgroupV2版本下获取cgroup分组下的所有的进程pid，，判断是否包括上文中添加的进程pid
    if (getCgroupVerson() == 2) {
        argumentList.clear();
        argumentList<< QVariant::fromValue((CgroupHelper::getPath()));
        QDBusPendingReply<QList<int>> replyPid = interface->callWithArgumentList(QDBus::Block, "GetGroupPids", argumentList);
        if(!replyPid.isValid() || replyPid.isError()) {
            qDebug()<<"replyPid.isValid()" << replyPid.isValid() << "replyPid.error()" << replyPid.isError() << replyPid.error();
            return false;
        }
        pids.clear();
        pids = replyPid.value();
        if(!pids.contains(pid)) {
            qDebug()<<pids<<pid;
            return false;
        }
    }
    return true;
}

bool CgroupHelper::setProcessGroupCPULimit(QDBusInterface *interface)
{
    if(!interface) {
        qDebug()<<"get dbusinterface failed";
        return false;
    }
    if(getCgroupVerson() == 1) {
        return setProcessGroupCPULimitV1(interface);
    } else if (getCgroupVerson() == 2) {
        return setProcessGroupCPULimitV2(interface);
    }
    qDebug()<<"getCgroupVersionFailed!";
    return false;
}

bool CgroupHelper::setProcessGroupCPULimitV2(QDBusInterface *interface)
{
    if(!interface) {
        qDebug()<<"get dbusinterface failed";
        return false;
    }
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(getPath())//cgroup分组路径,传相对路径
                 << QVariant::fromValue(QString("cpu"))//controller,常用的有cpu,memory
                 << QVariant::fromValue(QString("cpu.max"))//cgroup分组下的配置文件名
                 << QVariant::fromValue(QString("100000"));//可以传"max"表示不限制
    //cgroup V2下限制CPU的逻辑，修改对应路径下的cpu.max文件
    //cpu.max中已经设置了CFS调度周期长度为100000，只需要设置进程周期内占用CPU使用时间即可，为1000的倍数，可以超过100000,也可以传"max"表示不限制
    QDBusPendingReply<bool> reply = interface->callWithArgumentList(QDBus::Block, "SetProcessGroupResourceLimit", argumentList);
    if (!reply.isValid() || reply.isError()) {        
        qDebug()<<"reply.isValid()" << reply.isValid() << "reply.error()" << reply.isError() << reply.error();
        return false;
    }
    return reply.value();
}

bool CgroupHelper::setProcessGroupCPULimitV1(QDBusInterface *interface)
{
    if(!interface) {
        qDebug()<<"get dbusinterface failed";
        return false;
    }
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(getPath())//cgroup分组路径,传相对路径
                 << QVariant::fromValue(QString("cpu"))//controller,常用的有cpu,memory
                 << QVariant::fromValue(QString("cpu.cfs_period_us"))//cgroup分组下的配置文件名
                 << QVariant::fromValue(QString("100000"));//1000-100000
    //cgroup V1下限制CPU的逻辑
    //cpu.cfs_period_us为CFS调度周期的长度，以微秒为单位，默认为100000
    //cpu.cfs_quota_us为每个周期内能够使用的CPU时间，以微秒为单位，通常为1000的倍数，可以超过100000，如果设置为200000，则进程的CPU被限制为200%
    QDBusPendingReply<bool> reply = interface->callWithArgumentList(QDBus::Block, "SetProcessGroupResourceLimit", argumentList);
    if (!reply.isValid() || reply.isError()) {
        qDebug()<<"reply.isValid()" << reply.isValid() << "reply.error()" << reply.isError() << reply.error();
        return false;
    }
    argumentList.clear();
    argumentList << QVariant::fromValue(getPath())//cgroup分组路径,传相对路径
                 << QVariant::fromValue(QString("cpu"))//controller,常用的有cpu,memory
                 << QVariant::fromValue(QString("cpu.cfs_quota_us"))//cgroup分组下的配置文件名
                 << QVariant::fromValue(QString("100000"));
    reply = interface->callWithArgumentList(QDBus::Block, "SetProcessGroupResourceLimit", argumentList);
    if (!reply.isValid() || reply.isError()) {
        qDebug()<<"reply.isValid()" << reply.isValid() << "reply.error()" << reply.isError() << reply.error();
        return false;
    }
    return reply.value();
}

bool CgroupHelper::addProcessManagerWhiteList()
{
    QDBusInterface processManagerInterface(kpm_dbus_service,
                                           kpm_dbus_path,
                                           kpm_dbus_interface,
                                           QDBusConnection::sessionBus());
    QList<QVariant> argumentList;
    QDBusPendingReply<QStringList> replyAppList = processManagerInterface.callWithArgumentList(QDBus::Block, "AppList", argumentList);
    if (!replyAppList.isValid() || replyAppList.isError()) {
        return false;
    }
    QStringList lst = replyAppList.value();
    if(lst.contains(desktop_path)) {
        return true;
    }
    //先获取lingmo-process-manager的白名单，如果不包含本进程，将本进程手动添加到白名单中避免被管控
    argumentList << QVariant::fromValue(QString(desktop_path));
    QDBusPendingReply<bool> reply = processManagerInterface.callWithArgumentList(QDBus::Block, "AddApp", argumentList);
    if (!reply.isValid() || reply.isError()) {
        qDebug()<<"reply.isValid()" << reply.isValid() << "reply.error()" << reply.isError() << reply.error();
        return false;
    }
    return reply.value();
}

bool CgroupHelper::setServiceCpuLimit()
{
    CgroupUniquePtr interface(new QDBusInterface(daemon_dbus_service,
                                                 daemon_dbus_path,
                                                 daemon_dbus_interface,
                                                 QDBusConnection::systemBus()),
                              uniquePtrDeleter);
    //创建cgroup分组
    if(!createProcessGroup(interface.get())) {
        qDebug()<<"createProcessGroup failed!";
        return false;
    }
    //将进程ID添加创建的cgroup分组下
    if(!moveProcessToGroup(interface.get())) {
        qDebug()<<"moveProcessToGroup failed!";
        return false;
    }
    //将进程的desktop路径添加到lingmo.ProcessManager的应用白名单中,防止lingmo.process.manager将lingmo-search-service进程纳入管控
    if(!addProcessManagerWhiteList()) {
        qDebug()<<"addProcessManagerWhiteList failed!";
        return false;
    }
    //将限制CPU写入cgroup分组
    if(!setProcessGroupCPULimit(interface.get())) {
        qDebug()<<"setProcessGroupCPULimit failed!";
        return false;
    }
    qDebug()<<"setServiceCpuLimit successed";
    return true;
}


