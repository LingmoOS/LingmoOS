#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QDebug>
#include <QDBusReply>
#include <QString>
#include <QMap>
#include "tool_thread.h"
#include "single.h"
#include "lib_fun.h"
#include <QCoreApplication>
#include <dlfcn.h>


tool_thread::tool_thread(QString runtime, QString app_name, QString version, QString zh_name, QString desktop_name)
{
    this->m_runtime = runtime;
    this->m_app_name = app_name;
    this->m_version = version;
    this->m_zh_name = zh_name;
    this->m_desktop_name = desktop_name;
    /*测试代码*/
    qDebug() << "Info : uninstall information is ...";
    qDebug() << "Info : runtime   ======  "<< m_runtime;
    qDebug() << "Info : app_name ======   "<< m_app_name;
    qDebug() << "Info : version ======  "<< m_version;
}

bool tool_thread::getAndroidEnvWetherStart()
{
    QString userName = QString(getenv("LOGNAME"));
    int usetId = getuid();
    QFile file("/usr/share/kmre/kmre.conf");
    if (file.exists()) {
        QDBusInterface *p_kmreServiceInterface = new QDBusInterface(LINGMOOS_KMRE_SERVICE_NAME,
                                                                    LINGMOOS_KMRE_SERVICE_PATH,
                                                                    LINGMOOS_KMRE_INTERFACE_NAME,
                                                                    QDBusConnection::systemBus());

        if (!p_kmreServiceInterface->isValid()) {
            qWarning() << "dbus接口设置失败";
            delete p_kmreServiceInterface;
            p_kmreServiceInterface = nullptr;
            return false;
        }

        QDBusReply<QString> reply = p_kmreServiceInterface->call("GetPropOfContainer",
                                                                 userName,
                                                                 usetId,
                                                                 SYS_KMRE_BOOT_COMPLETED);
        qDebug() << userName << usetId << "sys.kmre.boot.completed" ;
        delete p_kmreServiceInterface;
        p_kmreServiceInterface = nullptr;
        if (reply.value() == "1") {
            return true;
        }
    } else {
        QDBusInterface *p_kyDroidServiceInterface = new QDBusInterface(LINGMOOS_KMRE_SERVICE_NAME,
                                                                       LINGMOOS_KMRE_SERVICE_PATH,
                                                                       LINGMOOS_KMRE_INTERFACE_NAME,
                                                                       QDBusConnection::systemBus());
        if (!p_kyDroidServiceInterface->isValid()) {
            qWarning() << "dbus接口设置失败";
            delete p_kyDroidServiceInterface;
            p_kyDroidServiceInterface = nullptr;
            return false;
        }
        QDBusReply<QString> reply = p_kyDroidServiceInterface->call("GetPropOfContainer",
                                                                    userName,
                                                                    usetId,
                                                                    SYS_KYDROID_BOOT_COMPLETED);
        delete p_kyDroidServiceInterface;
        p_kyDroidServiceInterface = nullptr;
        if (reply.value() == "1") {
            return true;
        }
    }
    return false;
}

/*void tool_thread::uninstallAndroidInterface()
{
    QLibrary library(ANDROID_SHARE_UNINSTALL_LIB);
    if (!library.load()) {
        qWarning() << "卸载安卓兼容动态库加载失败，直接退出";
        emit sig_uninstall_result(-1);
    }
    typedef bool (*p_unInstallInterface)(char *);
    p_unInstallInterface unInstallInterface = NULL;
    unInstallInterface = (p_unInstallInterface)library.resolve("uninstall_app");
    bool androidEnvWeStart = getAndroidEnvWetherStart();
    if (unInstallInterface != NULL && androidEnvWeStart) {
        printf("Info : start uninstall local pkg...\n");
        if (judge_single(1) == -1) {
            emit sig_uninstall_result(-1);
            return;
        }
        printf("Info : lock uninstall lock success\n");
        if (lock_extra_update_lock()) {
            emit sig_uninstall_result(-1);
            return;
        }
        printf("Info : lock extra update lock success\n");
        bool installResult = false;
        qDebug() << "------------------------------->" << this->m_app_name;
        installResult = unInstallInterface(this->m_app_name.toLatin1().data());
        if (installResult) {
            emit sig_uninstall_result(0);
        } else {
            emit sig_uninstall_result(-1);
        }
        if (unlock_uninstall_file()) {
            printf("Error : unlock uninstall file fail\n");
        }
        printf("Info : unlock uninstall lock success\n");
        if (unlock_extra_update_lock()) {
           printf("Error : unlock extra update lock fail\n");
        }
        printf("Info : unlock extra update lock success\n");
    } else {
        emit sig_uninstall_result(-1);
    }
    return;
}*/

void tool_thread::uninstallAndroidInterface()
{
    if(QFile::exists("/usr/lib/libkmre.so"))
    {
        this->libPath = "/usr/lib/libkmre.so";
    }
    else if(QFile::exists("/usr/lib/libkydroidrequest.so"))
    {
        this->libPath = "/usr/lib/libkydroidrequest.so";
    }
    QThread::sleep(1);
    qDebug() << libPath;

    if (!QFile::exists(libPath)) {
        Slot_Message_Notification(-1,"安卓库报错，卸载失败");
        emit sig_uninstall_result(-1);
    }

    void *module_handle;
    char *module_error;
    module_handle = dlopen(libPath.toStdString().c_str(), RTLD_LAZY);
    if (!module_handle) {
        dlclose(module_handle);
        Slot_Message_Notification(-1,"安卓库报错，卸载失败");
        emit sig_uninstall_result(-1);
    }
    bool (*uninstall_app)(char *appname);
    uninstall_app = (bool(*)(char *))dlsym(module_handle, "uninstall_app");
    if ((module_error = dlerror()) != NULL) {
        dlclose(module_handle);
        Slot_Message_Notification(-1,"安卓库报错，卸载失败");
        emit sig_uninstall_result(-1);
    }
    //pkgname:包名，如 com.tencent.mm
    bool nRes = uninstall_app(const_cast<char *>(m_app_name.toStdString().c_str()));
    if (nRes) {
        //qDebug() << "BackendWorker::installApp success pkgName:" << pkgName;
        QThread::sleep(1);
        dlclose(module_handle);
        Slot_Message_Notification(0,"Success");
        QDBusInterface *server_uninstall = NULL;
        server_uninstall = new QDBusInterface("com.lingmo.softwarestore.server",
                                              "/com/lingmo/softwarestore/server", \
                                              "com.lingmo.softwarestore.server",
                                              QDBusConnection::sessionBus());
        if (!server_uninstall->isValid())
        {
            qInfo() << "DBus service not  start....";
        }
        else
        {
            server_uninstall->asyncCall("acceptUinstallApk",this->m_app_name);

        }
        emit sig_uninstall_result(0);
    }
    else {
        dlclose(module_handle);
        Slot_Message_Notification(-1,"安卓库报错，卸载失败");
        emit sig_uninstall_result(-1);
    }
}

void tool_thread::run()
{
    QString username = QString(getenv("LOGNAME"));
    /* 是否成功卸载标志 ， -1: 失败 0：成功 */
    int i_result_flag = -2;
    QDBusInterface *server_fun = NULL;
    int i_ret = -1;

    /* 判断runtime与app_name是否为空 */
    if (this->m_runtime.isEmpty() || this->m_app_name.isEmpty()) {
        qDebug() << "runtime为空";
        i_result_flag = -1;
        goto FLAG;
    }
    /* 当为本地包时 */
    if (this->m_runtime == LOCAL_PACKAGE_FLAG) {
        qDebug() << "Info : this is local pkg";
        QDBusReply<bool> reply;
        /*创建接口映射*/
        server_fun = new QDBusInterface("com.lingmo.systemupgrade",
                                        "/com/lingmo/systemupgrade", \
                                        "com.lingmo.systemupgrade.interface",
                                        QDBusConnection::systemBus());
        if (!server_fun->isValid()) {
            qDebug() << "创建dbus接口映射失败：" << QDBusConnection::systemBus().lastError().message();
            i_result_flag = -1;
            goto FLAG;
        }

        /*绑定本地卸载接口信号*/
        QDBusConnection::systemBus().connect("com.lingmo.systemupgrade",
                                             "/com/lingmo/systemupgrade", \
                                             "com.lingmo.systemupgrade.interface",
                                             QString("PurgePackagesFinished"), \
                                             this,
                                             SLOT(slot_local_package_uninstall_result(bool,QString,QString)));


        /*调用服务端卸载函数*/
        QStringList applist;
        applist.append(this->m_app_name);
        server_fun->asyncCall("PurgePackages" , applist,username,getenv("LANG"));


    } else if (this->m_runtime == LOCAL_ANDROID_PACKAGE_FLAG) {
        uninstallAndroidInterface();
    }else if (this->m_runtime == KAIMING_PACKAGE){
        qDebug() << "Info : this is kaiming pkg";
        QDBusReply<bool> reply;
        /*创建接口映射*/
        server_fun = new QDBusInterface("org.lingmo.kaiming",
                                        "/org/lingmo/kaiming", \
                                        "org.lingmo.kaiming.interface",
                                        QDBusConnection::systemBus());
        if (!server_fun->isValid()) {
            qDebug() << "创建dbus接口映射失败：" << QDBusConnection::systemBus().lastError().message();
            i_result_flag = -1;
            goto FLAG;
        }

        /*绑定本地卸载接口信号*/
        QDBusConnection::systemBus().connect("org.lingmo.kaiming",
                                             "/org/lingmo/kaiming", \
                                             "org.lingmo.kaiming.interface",
                                             QString("finishsignal"), \
                                             this,
                                             SLOT(slot_kaiming_package_uninstall_result(QString,int,int,QString)));


        /*调用服务端卸载函数*/
        server_fun->asyncCall("uninstall",this->m_app_name,"");
    } else {
        //kare删除对应的desktop文件
        if (!m_app_name.compare("kare")){

            QString cmd = "pkexec --user root rm " + m_desktop_name + " -f";
            qDebug() << "file <<<<<" << cmd;
            system(cmd.toLocal8Bit().data());
            Slot_Message_Notification(0,"Success");
            i_result_flag = 0;
            goto FLAG;
        }
        /*为兼容环境中的包*/
        /*创建接口映射*/
        qDebug() << "Info : this is runtim  e environment pkg";
        QDBusReply<int> reply;
        server_fun = new QDBusInterface("com.lingmo.runtime" , "/com/lingmo/runtime" , "com.lingmo.runtime.interface" , QDBusConnection::systemBus());
        if (!server_fun->isValid()) {
            qDebug() << "创建dbus接口映射失败：" << QDBusConnection::systemBus().lastError().message();
            i_result_flag = -1;
            goto FLAG;
        }

        printf("Info : start uninstall compatible environment pkg...\n");


        printf("Info : lock uninstall lock success\n");

        /*调用服务端卸载函数*/
        server_fun->asyncCall("kre_uninstall_app" , this->m_runtime , this->m_app_name);

        /*判断是否卸载成功*/
        server_fun->asyncCall("kre_install_status" , this->m_runtime , this->m_app_name);
        if (!reply) {
            /*卸载成功*/
            i_result_flag = 0;
            printf("Info : uninstall compatible environment pkg success\n");
            /*删除json配置文件*/
            server_fun->asyncCall("delete_json_file" , this->m_runtime , this->m_app_name);
        } else {
            /*卸载失败*/
            i_result_flag = -1;
            printf("Error : uninstall compatibel enviroment pkg fail\n");
        }

        /*unlock file*/
        //        i_ret =  unlock_uninstall_file();
        //        if (i_ret == -1) {
        //           printf("Error : unlock uninstall lock fail\n");
        //        }
        //        printf("Info : unlock uninstall lock success\n");
    }

    delete server_fun;
    server_fun = nullptr;

FLAG:
    if (i_result_flag == -1) {
        emit sig_uninstall_result(-1);
    } else if(i_result_flag == 0) {
        emit sig_uninstall_result(0);
    } else {
        ;
    }
    return;
}

void tool_thread::slot_local_package_uninstall_result(bool status,QString error_string,QString error_desc)
{
    int i_ret = -1;
    if (status == true) {
        printf("Info : local pkg uninstall success\n");
        Slot_Message_Notification(0,"Success");
        emit sig_uninstall_result(0);
    }
    else {
        Slot_Message_Notification(-1,error_string);
        emit sig_uninstall_result(-1);
    }


    /*unlock file*/
    //i_ret =  unlock_uninstall_file();
    //     if (i_ret == -1) {
    //         printf("Error : unlock uninstall file fail\n");
    //     }

    //     printf("Info : unlock uninstall lock success\n");

    //     if (unlock_extra_update_lock()) {
    //        printf("Error : unlock extra update lock fail\n");
    //     }

    //     printf("Info : unlock extra update lock success\n");

    return;
}

void tool_thread::slot_local_package()
{
    emit sig_uninstall_status(0);
}

void tool_thread::Slot_Message_Notification(int result,QString errorinfo)
{

    QString dbus_service = "org.freedesktop.Notifications";
    QString dbus_path = "/org/freedesktop/Notifications";
    QString dbus_interface = "org.freedesktop.Notifications";
    QString notify = "Notify";
    QList<QVariant> args;
    QStringList action;
    QMap<QString,QVariant> hint;
    QString name = QObject::tr("lingmo-uninstaller");
    QString title;
    QString string ;

    if (result == 0){
        title = QObject::tr("Software uninstallation");
        string = QObject::tr("Software uninstalled successfully");
    }
    else{
        title = QObject::tr("Software uninstallation failed");
        string = errorinfo;
    }

    uint32_t replaceid = 0;
    QString icon = "lingmo-installer";
    args << name
         << replaceid
         << icon
         << title
         << string
         << action
         << hint
         << -1;
    QDBusInterface *iface = new QDBusInterface(dbus_service, dbus_path, dbus_interface, QDBusConnection::sessionBus());
    iface->asyncCallWithArgumentList(notify,args);

    return;

}
void tool_thread::slot_kaiming_package_uninstall_result(QString appName, int type, int errorCode, QString error_desc)
{
    int i_ret = -1;
    if (type == 2) {
        if (errorCode == 0){
            printf("Info : kaiming pkg uninstall success\n");
            Slot_Message_Notification(0,"Success");
            emit sig_uninstall_result(0);
        }else {
            QString str = error_desc;
            Slot_Message_Notification(-1,str);
            emit sig_uninstall_result(-1);
        }

    }
    return;

}
