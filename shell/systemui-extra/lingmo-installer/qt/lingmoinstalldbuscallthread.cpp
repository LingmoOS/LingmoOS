#include "lingmoinstalldbuscallthread.h"
#include <pwd.h>
#include <unistd.h>
#include <errno.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <stdio.h>
#include <dlfcn.h>
#include <qdir.h>
#include <qprocess.h>
#include <sys/stat.h>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QCryptographicHash>
#include <qtextstream.h>
#include <QSettings>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
LingmoInstalldbusCallThread::LingmoInstalldbusCallThread(QString debPath, QString m_appName, QString m_application, QString m_applicationZh, QString m_appVersion, QString kareImage) : m_debPath(debPath),m_appName(m_appName),m_application(m_application),m_applicationZh(m_applicationZh),m_appVersion(m_appVersion)
{
    m_kareImage = kareImage;
}

void LingmoInstalldbusCallThread::run()
{

    qDebug() << "===============kareIMager" << m_kareImage;

    if (m_kareImage == "Local System"){
        if(m_debPath.contains(".snap",Qt::CaseSensitive))
        {
            m_pOsServiceInterface = new QDBusInterface(LINGMO_OS_DBUS_SERVICE_NAME,
                                                       LINGMO_OS_DBUS_PATH,
                                                       LINGMO_OS_DBUS_INTERFACE_NAME,
                                                       QDBusConnection::systemBus());

            if (!m_pOsServiceInterface->isValid()) {
                qDebug() << qPrintable(QDBusConnection::systemBus().lastError().message());
                emit lingmoOsinstallStatus(false,"初始化dbus失败，dbus阻塞，请检查更新管理器服务是否正常");
                return ;
            }

            m_pOsServiceInterface->setTimeout(2147483647);

            QDBusConnection::systemBus().connect(LINGMO_OS_DBUS_SERVICE_NAME, LINGMO_OS_DBUS_PATH, \
                                                 LINGMO_OS_DBUS_INTERFACE_NAME, QString("KumSnapSignal"), \
                                                 this, SLOT(osInstallOverSoftwareSnapSlots(QString, QMap<QString, QVariant>)));


            m_pOsServiceInterface->asyncCall("install_snap", m_debPath);
        }
        else if (m_debPath.contains(".apk",Qt::CaseSensitive)){

            QFileInfo fi(m_debPath);
            QString fileName = fi.fileName();
            m_loginUserName = getUserName();
            m_loginUserId = getUid();
            QString destPath = QString("/var/lib/kmre/kmre-%1-%2/data/local/tmp").arg(m_loginUserId).arg(m_loginUserName);
            bool copy_status = copyFile(m_debPath,QString("%1/%2").arg(destPath).arg(fileName));
            if (copy_status) {
                //install apk file
                installApp(fileName, m_appName, m_application, m_applicationZh, m_appVersion);
            }
            else {
                emit lingmoOsinstallStatus(false,"拷贝apk文件到指定目录失败");
            }




        }
        else{
            m_pOsServiceInterface = new QDBusInterface(LINGMO_OS_DBUS_SERVICE_NAME,
                                                       LINGMO_OS_DBUS_PATH,
                                                       LINGMO_OS_DBUS_INTERFACE_NAME,
                                                       QDBusConnection::systemBus());

            if (!m_pOsServiceInterface->isValid()) {
                qDebug() << qPrintable(QDBusConnection::systemBus().lastError().message());
                emit lingmoOsinstallStatus(false,"初始化dbus失败，dbus阻塞，请检查更新管理器服务是否正常");
                return ;
            }

            m_pOsServiceInterface->setTimeout(2147483647);



            //        QDBusConnection::systemBus().connect(LINGMO_OS_DBUS_SERVICE_NAME, LINGMO_OS_DBUS_PATH, \
            //                                             LINGMO_OS_DBUS_INTERFACE_NAME, QString("kum_deb_signal"), \
            //                                             this, SLOT(osInstallOverSoftwareAptSlots(QString, QMap<QString, QVariant>)));

            QDBusConnection::systemBus().connect(LINGMO_OS_DBUS_SERVICE_NAME, LINGMO_OS_DBUS_PATH, \
                                                 LINGMO_OS_DBUS_INTERFACE_NAME, QString("InstalldebFinished"), \
                                                 this, SLOT(osInstallOverSoftwareAptSlots(bool,QString,QString)));


            QDBusConnection::systemBus().connect(LINGMO_OS_DBUS_SERVICE_NAME, LINGMO_OS_DBUS_PATH, \
                                                 LINGMO_OS_DBUS_INTERFACE_NAME, QString("InstalldebStatusChanged"), \
                                                 this, SLOT(osInstallOverstatus(int,QString,QString)));

            QString lang = getenv("LANG");
            m_pOsServiceInterface->asyncCall("InstallDebFile", "lingmo-installer",m_debPath,true,true,lang);
        }
    }else {
        /*安装到kare中*/
        QProcess * process = new QProcess;
        QString cmd = "pkexec --user root kare -e " + m_kareImage + " -i " + m_debPath;
        qDebug() <<"安装到kare中的命令" << cmd;

        process->start(cmd);

        process->waitForFinished(-1);

        int code = process->exitCode();
        qDebug() << "coide==========" << code;
        if (code == 0){
            emit lingmoOsinstallStatus(true,"kare软件安装成功");
        }else {
            emit lingmoOsinstallStatus(false,"kare软件安装失败");
        }

    }

}

/*返回安装的进度值*/
void LingmoInstalldbusCallThread::osInstallOverstatus(int status,QString erro_string,QString erro_desc)
{
    emit lingmoOsinstalldingStatus(status);
    emit lingmobatchOsinstalldingStatus(status,m_debPath);
}

/* 安装完成信号，根据参数确认安装状态 */

void LingmoInstalldbusCallThread::osInstallOverSoftwareAptSlots(bool status,QString erro_string,QString erro_desc)
{

    QString map_string = erro_string + erro_desc;

    if (status == true) {
        emit lingmoOsinstallStatus(true,map_string);
    } else {
        emit lingmoOsinstallStatus(false,map_string);
    }
    return;
}

void LingmoInstalldbusCallThread::osInstallOverSoftwareSnapSlots(QString string, QMap<QString, QVariant> map)
{
    qDebug() << string << "返回的map表" << map;

    QVariant map_test =  map["error_message"];
    QString map_string = map_test.toString();
    if (string == nullptr) {
        emit lingmoOsinstallStatus(false,map_string);
        return;
    }
    if (string == "snap_finish") {
        emit lingmoOsinstallStatus(true,map_string);
    } else {
        emit lingmoOsinstallStatus(false,map_string);
    }
    return;
}

/********getusername******/
QString LingmoInstalldbusCallThread::getUserName()
{
    std::string user_name;

    struct passwd *pwd = NULL;
    pwd = getpwuid(getuid());
    if (pwd && pwd->pw_name) {
        char* _user = pwd->pw_name;
        struct passwd *q = NULL;
        q = getpwnam(_user);
        if (q && (q->pw_uid != getuid())) {
            fprintf(stderr, "User name doesn't match uid.\n");
            // syslog(LOG_ERR, "utils: User name doesn't maych uid.");
            return QString("");
        }

        return QString::fromStdString(std::string(pwd->pw_name));
    }
    else {
        user_name = std::getenv("USER");
        if (user_name.empty()) {
            user_name = std::getenv("USERNAME");
        }
        if (user_name.empty()) {
            char name[16];
            snprintf(name, sizeof(name), "%u", getuid());
            user_name = std::string(name);
        }
    }

    return QString::fromStdString(user_name);
}

/********getusrid********/
QString LingmoInstalldbusCallThread::getUid()
{
    uid_t uid = getuid();
    return QString::number(uid);
}

QString LingmoInstalldbusCallThread::osVersion()
{
    QString distribDescription;
    if (QFile::exists("/etc/lsb-release")) {
        QSettings lsbSetting("/etc/lsb-release", QSettings::IniFormat);
        distribDescription = lsbSetting.value("DISTRIB_DESCRIPTION").toString().replace("\"", "");
    }

    return distribDescription;
}

/********拷贝apk文件到指定目录下*****/
bool LingmoInstalldbusCallThread::copyFile(const QString &srcFile, const QString &destFile)
{

    qDebug() << "Utils::copyFile:" << srcFile << destFile;
    QFile fp(destFile);
    if (fp.exists()) {
        if (!fp.remove()) {
            qCritical() << "Failed to remove:" << destFile;
            //return false;
        }
    }

    return QFile::copy(srcFile, destFile);
}
/*********拷贝结果*******/
/*void LingmoInstalldbusCallThread::onCopyFinished(const QString &fileName, bool success)
{
    if (success) {
        //install apk file
       ;// installApp(fileName, m_pkgName, m_application, m_applicationZh, m_version);
    }
    else {
         emit lingmoOsinstallStatus(false,"拷贝apk文件到指定目录失败");
    }
}

/*********安装apk文件**********/
void LingmoInstalldbusCallThread::installApp(const QString &fileName, const QString &pkgName, const QString &application, const QString &applicationZh, const QString &version)
{
    //    m_iconFilePath.clear();
    // QThread::sleep(1);
    if (fileName.isEmpty()) {
        emit lingmoOsinstallStatus(false,"安装包包名为空，无法安装");
        return;
    }

    QString libPath = "/usr/lib/libkmre.so";
    if (!QFile::exists(libPath)) {
        emit lingmoOsinstallStatus(false,"安装安卓包的库为空，请检查/usr/lib/libkmre.so库是否存在");
        return;
    }

    void *module_handle;
    char *module_error;
    module_handle = dlopen(libPath.toStdString().c_str(), RTLD_LAZY);
    if (!module_handle) {
        emit lingmoOsinstallStatus(false,"module_handle 等于空");
        return;
    }
    bool (*install_app)(char *filename, char *appname, char *pkgname);
    install_app = (bool(*)(char *, char*, char *))dlsym(module_handle, "install_app");
    if ((module_error = dlerror()) != NULL) {
        dlclose(module_handle);
        emit lingmoOsinstallStatus(false,"module_handle获取的到值有误");
        return;
    }
    //qDebug() << "BackendWorker::installApp " << fileName;
    //filename:apk名称，如 com.tencent.mm_8.0.0.apk
    //appname:应用名,如 微信
    //pkgname:包名，如 com.tencent.mm
    //qDebug() << "###install_app fileName:" << fileName <<  ",pkgName:" << pkgName << ", application:" << application;
    bool nRes = install_app(const_cast<char *>(fileName.toStdString().c_str()), const_cast<char *>("test"), pkgName.isEmpty() ? const_cast<char *>("test") : const_cast<char *>(pkgName.toStdString().c_str()));
    if (nRes) {
        //qDebug() << "BackendWorker::installApp success pkgName:" << pkgName;
        // QThread::sleep(1);
        updateDekstopAndIcon(pkgName, application, applicationZh, version);
        emit lingmoOsinstallStatus(true,"sucess");
    }
    else {
        qDebug() << "BackendWorker::installApp failed: " << pkgName;
        emit lingmoOsinstallStatus(false,"安装失败此apk文件并未适配操作系统");
    }
    dlclose(module_handle);
}
/*************設置dekstop文件************/
void LingmoInstalldbusCallThread::updateDekstopAndIcon(const QString &pkgName, const QString &application, const QString &applicationZh, const QString &version)
{
    QString cpuType = QString("");
    QString gpuVendor = QString("");
    getCpuAndGpuInfo(cpuType, gpuVendor);
    m_osVersion = osVersion();

    //qDebug() << "updateDekstopAndIcon pkgName:" << pkgName << ",application:" << application << ",applicationZh:" << applicationZh;
    if (!pkgName.isEmpty() && !application.isEmpty() && !applicationZh.isEmpty()) {
        // sendApkInfoToServer(m_osVersion, cpuType, gpuVendor, pkgName, application);
        if (pkgName != "com.antutu.benchmark.full" && pkgName != "com.antutu.benchmark.full.lite") {
            generateDesktop(pkgName, application, applicationZh, version);
            generateIcon(pkgName);
        }
    }
    else {
        QByteArray array = getInstalledAppListJsonStr();
        //qDebug() << "Installed: " << QString(array);
        QJsonParseError err;
        QJsonDocument document = QJsonDocument::fromJson(array, &err);//QJsonDocument document = QJsonDocument::fromJson(QString::fromStdString(std::string(running_info)).toLocal8Bit().data(), &err);
        if (err.error == QJsonParseError::NoError && !document.isNull() && !document.isEmpty() && document.isArray()) {
            QJsonArray jsonArray = document.array();
            foreach (QJsonValue val, jsonArray) {
                QJsonObject subObject = val.toObject();
                QString name = subObject.value("package_name").toString();
                if (name != "com.antutu.benchmark.full" && name != "com.antutu.benchmark.full.lite") {
                    //qDebug() << "Test App: " << subObject.value("app_name").toString() << subObject.value("package_name").toString() << subObject.value("version_name").toString();
                    bool b = generateDesktop(name, subObject.value("app_name").toString(), subObject.value("app_name").toString(), subObject.value("version_name").toString());
                    if (b) {
                        qDebug() << subObject.value("package_name").toString() << "installed!";
                        // sendApkInfoToServer(m_osVersion, cpuType, gpuVendor, subObject.value("package_name").toString(), subObject.value("app_name").toString());
                    }
                    this->generateIcon(subObject.value("package_name").toString());
                }
            }
        }
    }
}

/*********設置dektop文件*****/
bool LingmoInstalldbusCallThread::generateDesktop(const QString &pkgName, const QString &application, const QString &applicationZh, const QString &version)
{
    if (pkgName.isEmpty()) {
        return false;
    }

    const QString desktopsPath = QString("%1/.local/share/applications").arg(QDir::homePath());
    if (!QDir().exists(desktopsPath)) {
        QDir().mkdir(desktopsPath);
    }

    //generate desktop file
    // ~/.local/share/applications
    const QString destDesktopFile = QString("%1/.local/share/applications/%2.desktop").arg(QDir::homePath()).arg(pkgName);
    QFile desktopFp(destDesktopFile);
    if (desktopFp.exists()) {
        //qDebug() << "destDesktopFile is exists already!";
        return false;
    }
    QFile file(destDesktopFile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream out(&file);
    if (version.isNull() || version.isEmpty()) {
        out << "[Desktop Entry]\n"
               "Terminal=false\n"
               "Type=Application\n"
               "StartupNotify=false\n"
               "Keywords=Android;App;Apk\n"
               "Categories=Android;App;Apk\n"
               "Name=" + application + "\n"
                                       "Name[zh_CN]=" + applicationZh + "\n"
                                                                        "Comment=" + application + "\n"
                                                                                                   "Comment[zh_CN]=" + applicationZh + "\n"
                                                                                                                                       "Exec=/usr/bin/startapp " + pkgName + "\n"
                                                                                                                                                                             "Icon=" + QDir::homePath() + "/.local/share/icons/" + pkgName + ".png"
            << endl;
    }
    else {
        out << "[Desktop Entry]\n"
               "Terminal=false\n"
               "Type=Application\n"
               "StartupNotify=false\n"
               "Keywords=Android;App;Apk\n"
               "Categories=Android;App;Apk\n"
               "Name=" + application + "\n"
                                       "Name[zh_CN]=" + applicationZh + "\n"
                                                                        "Comment=" + application + "\n"
                                                                                                   "Comment[zh_CN]=" + applicationZh + "\n"
                                                                                                                                       "Exec=/usr/bin/startapp " + pkgName + " " + version + "\n"
                                                                                                                                                                                             "Icon=" + QDir::homePath() + "/.local/share/icons/" + pkgName + ".png"
            << endl;
    }
    out.flush();
    file.close();

    chmod(destDesktopFile.toStdString().c_str(), 0777);
    QProcess::startDetached(QString("/usr/bin/update-desktop-database %1").arg(desktopsPath));

    return true;
}
//设置icon
void LingmoInstalldbusCallThread::generateIcon(const QString &pkgName)
{
    if (pkgName.isEmpty()) {
        return;
    }

    const QString iconsPath = QString("%1/.local/share/icons").arg(QDir::homePath());
    if (!QDir().exists(iconsPath)) {
        QDir().mkdir(iconsPath);
    }

    //copy icon file from /tmp
    // ~/.local/share/icons
    const QString linuxIconPath = QString("%1/%2.png").arg(iconsPath).arg(pkgName);
    const QString linuxSvgPath = QString("%1/%2.svg").arg(iconsPath).arg(pkgName);
    const QString containerIconPath = QString("/var/lib/kmre/kmre-%1-%2/data/local/icons/%3.png").arg(m_loginUserId).arg(m_loginUserName).arg(pkgName);
    /*const QFileInfo fi(m_iconFilePath);
    QFile dfp(linuxIconPath);
    if (fi.isFile() && fi.suffix().toLower() != "xml" && !dfp.exists()) {//有的apk文件解析出来的图片为.xml格式
        utils::copyFile(m_iconFilePath, linuxIconPath);
        QProcess::startDetached(QString("/usr/sbin/update-icon-caches %1").arg(iconsPath));
    }*/
    //如果沒有从apk解压到icon，则在安装完成后从android目录讲对应apk的icon拷贝出来使用（注意：安装完成后，需要等待一些时间再做拷贝操作，因为测试发现android里面图片没有在安装完成后立即生成）
    QFile cfp(containerIconPath);
    QFile fp(linuxIconPath);
    if (cfp.exists() && !fp.exists()) {
        copyFile(containerIconPath, linuxIconPath);

        /*QFileInfo fi("/usr/share/lingmo-software-center/foot.txt");
        if (fi.exists()) {
            QString readline = utils::readFileContent("/usr/share/lingmo-software-center/foot.txt");
            qDebug() << "readline:" << readline;
            QString cmd = QString("convert -resize 96x96 %1 %2").arg(linuxIconPath).arg(linuxIconPath);
            qDebug() << "cmd:" << cmd;
            QString cmd2 = QString("convert %1 %2").arg(linuxIconPath).arg(linuxSvgPath);
            qDebug() << "cmd2:" << cmd2;
            QString cmd3 = QString("sed -i '$i %1' %2").arg(readline).arg(linuxSvgPath);
            qDebug() << "cmd3:" << cmd3;
            QString cmd4 = QString("sed -i 's/[[:space:]]href/xlink\:href/' %1").arg(linuxSvgPath);
            qDebug() << "cmd4:" << cmd4;
            QString cmd5 = QString("sed -i 's/\"96\"/\"83\"/g' %1").arg(linuxSvgPath);
            qDebug() << "cmd5:" << cmd5;
            QProcess::startDetached(cmd);
            QProcess::startDetached(cmd2);
            QProcess::startDetached(cmd3);
            QProcess::startDetached(cmd4);
            QProcess::startDetached(cmd5);
        }*/

        QProcess::startDetached(QString("/usr/sbin/update-icon-caches %1").arg(iconsPath));
    }
}

void LingmoInstalldbusCallThread::getCpuAndGpuInfo(QString &cpuType, QString &gpuVendor)
{
    m_loginUserName = getUserName();
    m_loginUserId = getUid();
    bool androidReady = isAndroidReady(m_loginUserName, m_loginUserId);
    if (androidReady) {
        QDBusInterface mSessionBusInterface("cn.lingmoos.Kmre.Manager", "/cn/lingmoos/Kmre/Manager", "cn.lingmoos.Kmre.Manager", QDBusConnection::sessionBus());
        QString displayInfo;
        QDBusMessage response = mSessionBusInterface.call("getDisplayInformation");
        if (response.type() == QDBusMessage::ReplyMessage) {
            displayInfo = response.arguments().takeFirst().toString();
            if (!displayInfo.isEmpty()) {
                QJsonDocument jsonDocument = QJsonDocument::fromJson(displayInfo.toLocal8Bit().data());
                if (!jsonDocument.isNull()) {
                    QJsonObject jsonObject = jsonDocument.object();
                    if (!jsonObject.isEmpty() && jsonObject.size() > 0) {
                        if (jsonObject.contains("cpu_type")) {
                            cpuType = jsonObject.value("cpu_type").toString();
                        }
                        if (jsonObject.contains("gpu_vendor")) {
                            gpuVendor = jsonObject.value("gpu_vendor").toString();
                        }
                    }
                }
            }
        }
    }

    /*   if (cpuType.isNull() || cpuType.isEmpty()) {
#if defined(__aarch64__)
        if (utils::isFt1500aCpu()) {
            cpuType = "FT1500A";
        }
        else {
            cpuType = "unknown";
        }
#else
        cpuType = "unknown";
#endif
    }*/

    if (gpuVendor.isNull() || gpuVendor.isEmpty()) {
        gpuVendor = "unknown";
    }
}

bool LingmoInstalldbusCallThread::isAndroidReady(const QString &username, const QString &uid)
{
    QString value;
    QDBusInterface *interface;
    interface = new QDBusInterface("cn.lingmoos.Kmre", "/cn/lingmoos/Kmre", "cn.lingmoos.Kmre", QDBusConnection::systemBus());
    QDBusMessage response = interface->call("GetPropOfContainer", username, uid.toInt(), "sys.kmre.boot_completed");
    if (response.type() == QDBusMessage::ReplyMessage) {
        value = response.arguments().takeFirst().toString();
    }
    delete interface;
    if (value == "1") {
        return true;
    }
    else {
        return false;
    }
}


/*void LingmoInstalldbusCallThread::sendApkInfoToServer(const QString &os, const QString &cpuType, const QString &gpuVendor, const QString &pkgName, const QString &appName)
{
    QByteArray publicKey(
        "LS0tLS1CRUdJTiBQVUJMSUMgS0VZLS0tLS0KTUlJQklqQU5CZ2txaGtpRzl3MEJBUUVGQUFPQ0FR\
        OEFNSUlCQ2dLQ0FRRUFzdW1NTFJEdlFNb0tEQkRJODRqSgpqc1A0Mk55V0pWVEZob2Jra3ZiT05j\
        dExYTXVzRmo2TzJUblZYU3Z6VlVLSjRqZkpwT2l2WEphOVB5Z2wzYTRnClBzSU40enNCMEdOY0tr\
        R3VsS2RrV2x6S3lWQ2xlTzhiQnN6SjkwbTc3cWF6YWg3a1A0TUl0WTVFczBpSkpiR0oKN1MxcERj\
        MlJkNnVFQWJLaXJyRTFlNzlFTEd4am5VN2V5NWkyRDE2WWJoZEQwZ2lNa2RHR3piQXBKTWZWRVJR\
        TQo1NXorMFVqdS8zSFJhNFY3b3p2TGRPRE5HUURaeWNJU0l3VHBLbFR3RjBxazdCNjVhTUlJenQ1\
        dnhOK1lxYU1GClppZFRLNzcxNjdqNEExZ3F3MG45bjlybWVXUGRWZ3dudnRtVXp4Q1krNk05SXpK\
        TDI3eWpRUTV1WGQ3RVdMT3IKbndJREFRQUIKLS0tLS1FTkQgUFVCTElDIEtFWS0tLS0tCg==");

    // 将需要发送的信息以json格式保存
    QJsonObject jsonObj;
    jsonObj.insert("OS", QJsonValue(os));
    jsonObj.insert("CPU", QJsonValue(cpuType));
    jsonObj.insert("GPU", QJsonValue(gpuVendor));
    jsonObj.insert("pkgName", QJsonValue(pkgName));
    jsonObj.insert("appName", QJsonValue(appName));

    //  将数据转化为QString
    QString informationData(QJsonDocument(jsonObj).toJson(QJsonDocument::Compact));
    //  计算发送信息的MD5
    QCryptographicHash cryHash(QCryptographicHash::Algorithm::Md5);
    cryHash.addData(informationData.toLatin1());
    QByteArray md5(cryHash.result());

    //  加密发送信息的MD5
    QByteArray enMd5 = encrypt(md5, publicKey);
    //  将加密的MD5转化为BASE64格式
    QString enMd5Base64(enMd5.toBase64());
    //  填入需要调用的参数
    QList<QVariant> tempVar;
    //  填入应用名
    tempVar.push_back(QString("kmre-apk-installer"));
    //  填入信息标识(记录此种信息为哪一类信息)
    tempVar.push_back(QString("apk"));
    //  填入具体信息
    tempVar.push_back(QString(QJsonDocument(jsonObj).toJson(QJsonDocument::Compact)));
    //  填入加密并转为base64的MD5码
    tempVar.push_back(enMd5Base64);

    //  对sendInfo进行调用
    /*QDBusMessage tempMessage = daqInterface.callWithArgumentList(QDBus::Block, QString("sendInfo"), tempVar);
    if (tempMessage.type() == QDBusMessage::MessageType::ReplyMessage) {
       if (!tempMessage.arguments().isEmpty()) {
            //查看返回值
            qDebug() << "Return status" << tempMessage.arguments().first().toString();
       }
       else {
           qDebug() << "Error Name:" << tempMessage.errorName() << " errorMessage:" << tempMessage.errorMessage();
       }
    }
    else {
       qDebug() << "###Error Name: "<< tempMessage.errorName() << " errorMessage:" << tempMessage.errorMessage();
    }*/

/*  QDBusInterface daqInterface("com.lingmo.daq", "/com/lingmo/daq", "com.lingmo.daq.interface", QDBusConnection::systemBus());
    QDBusMessage response = daqInterface.callWithArgumentList(QDBus::Block, QString("sendInfo"), tempVar);//QDBusMessage response = daqInterface.call("sendInfo", tempVar);
    if (response.type() == QDBusMessage::MessageType::ReplyMessage) {//QDBusMessage::ReplyMessage
        if (!response.arguments().isEmpty()) {
            int ret = response.arguments().takeFirst().toInt();
            qDebug() << "ret:" << ret;
//            switch (ret) {
//            case 0:
//                qDebug() << "success";
//                break;
//            case -1:
//                qDebug() << "information参数无法转化为json格式";
//                break;
//            case -2:
//                qDebug() << "网络异常";
//                break;
//            case -3:
//                qDebug() << "服务器异常";
//                break;
//            case -4:
//                qDebug() << "md5加密异常";
//                break;
//            case -5:
//                qDebug() << "md5异常";
//                break;
//            case -6:
//                qDebug() << "md5解密异常";
//                break;
//            default:
//                break;
//            }
        }
        else {
            qDebug() << "###Error Name: " << response.errorName() << " errorMessage:" << response.errorMessage();
        }
    }
}*/
QByteArray LingmoInstalldbusCallThread::getInstalledAppListJsonStr()
{
    QString libPath = "/usr/lib/libkmre.so";
    if (!QFile::exists(libPath)) {
        return QString("[]").toUtf8();
    }

    void *module_handle;
    char *module_error;
    module_handle = dlopen(libPath.toStdString().c_str(), RTLD_LAZY);
    if (!module_handle) {
        return QString("[]").toUtf8();
    }

    char *(*get_installed_applist)();
    get_installed_applist = (char *(*)())dlsym(module_handle, "get_installed_applist");
    if ((module_error = dlerror()) != NULL) {
        dlclose(module_handle);
        return QString("[]").toUtf8();
    }

    char *list_info = NULL;
    list_info = get_installed_applist();
    if (list_info) {
        std::string runningInfo = std::string(list_info);
        QByteArray byteArray(list_info, runningInfo.length());
        dlclose(module_handle);
        return byteArray;
    }

    dlclose(module_handle);
    return QString("[]").toUtf8();
}

/*void LingmoInstalldbusCallThread::evpError(void)
{
    int n = ERR_get_error();
    char szErr[512];
    char errStr[1024];
    ERR_error_string(n, szErr);
    sprintf(errStr, "error code = %d,code string = %s", n, szErr);
    qWarning() << "kmre-apk-installer:" << errStr;
}

// 使用RSA公钥进行加密
QByteArray LingmoInstalldbusCallThread::encrypt(const QByteArray &md5, const QByteArray &keyBase64)
{
    BIO *bio = NULL;
    RSA *p_rsa = NULL;
    EVP_PKEY *key = NULL;
    EVP_PKEY_CTX *ctx = NULL;
    unsigned char *out;
    size_t outlen;
    QByteArray tempKey = QByteArray::fromBase64(keyBase64);
    if ((bio = BIO_new_mem_buf(tempKey.data(),tempKey.size())) == NULL) {
        evpError();
        return QByteArray();
    }

    if ((p_rsa=PEM_read_bio_RSA_PUBKEY(bio,NULL,NULL,NULL))==NULL) {
        evpError();
        BIO_free(bio);
        return QByteArray();
    }

    key = EVP_PKEY_new();
    if (key == NULL) {
        evpError();
        BIO_free(bio);
        RSA_free(p_rsa);
        return QByteArray();
    }

    if (EVP_PKEY_set1_RSA(key,p_rsa)<=0) {
        evpError();
        BIO_free(bio);
        RSA_free(p_rsa);
        EVP_PKEY_free(key);
        return QByteArray();
    }

    ctx = EVP_PKEY_CTX_new(key,NULL);
    if (ctx == NULL) {
        evpError();
        BIO_free(bio);
        RSA_free(p_rsa);
        EVP_PKEY_free(key);
        return QByteArray();
    }

    if (EVP_PKEY_encrypt_init(ctx) <=0) {
        evpError();
        BIO_free(bio);
        RSA_free(p_rsa);
        EVP_PKEY_free(key);
        EVP_PKEY_CTX_free(ctx);
        return QByteArray();
    }

    //  设置填充方式为OAEP
    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0) {
        evpError();
        BIO_free(bio);
        RSA_free(p_rsa);
        EVP_PKEY_free(key);
        EVP_PKEY_CTX_free(ctx);
        return QByteArray();
    }

    //  确定加密buf长度
    if (EVP_PKEY_encrypt(ctx, NULL, &outlen, (const unsigned char *)md5.data(), md5.size()) <= 0) {
        evpError();
        BIO_free(bio);
        RSA_free(p_rsa);
        EVP_PKEY_free(key);
        EVP_PKEY_CTX_free(ctx);
        return QByteArray();
    }

    out = (unsigned char *) OPENSSL_malloc(outlen);
    if (!out) {
        evpError();
        BIO_free(bio);
        RSA_free(p_rsa);
        EVP_PKEY_free(key);
        EVP_PKEY_CTX_free(ctx);
        return QByteArray();
    }

    //  进行加密
    if (EVP_PKEY_encrypt(ctx, out, &outlen, (const unsigned char *)md5.data(), md5.size()) <= 0) {
        evpError();
        BIO_free(bio);
        RSA_free(p_rsa);
        EVP_PKEY_free(key);
        EVP_PKEY_CTX_free(ctx);
        OPENSSL_free(out);
        return QByteArray();
    }

    QByteArray retByteArray((const char *)out,outlen);
    OPENSSL_free(out);
    BIO_free(bio);
    RSA_free(p_rsa);
    EVP_PKEY_free(key);
    EVP_PKEY_CTX_free(ctx);

    return retByteArray;
}*/


