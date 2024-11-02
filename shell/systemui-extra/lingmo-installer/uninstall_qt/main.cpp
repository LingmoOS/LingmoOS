#include <QApplication>
#include <QDesktopWidget>
#include <QFileInfo>
#include <QString>
#include <QTranslator>
#include <QByteArray>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <syslog.h>
#include <QStringList>
#include <QFile>
#include <QHash>
#include <QDebug>
#include <glib.h>
#include "single.h"
#include "lib_fun.h"
#include <qlogging.h>
#include <stdio.h>
#include <stdlib.h>
#include <qdatetime.h>
#include <singleapplication.h>
#include <lingmostylehelper/lingmostylehelper.h>
#include <signal.h>
#include <QDBusPendingCall>

#include "main_window.h"
#include "single_window.h"
#include "xatom-helper.h"
#include "adaptscreeninfo.h"

#define IRREGULARPACKAGE_FILE_PATH "/usr/share/lingmo-installer/irregularPackage.list"
#define LOCAL_PACKAGE_FLAG "local_pkg"
#define LOCAL_ANDROID_PACKAGE_FLAG "local_Android_pkg"
#define ANDROID_DESKTOP_FILE_PATH ".local/share/applications"
#define KAIMING_PACKAGE "kaiming"
/*
 *now , lingmo-uninstall use command lock and uninstall lock
 *cache 35 signal , po#include <singleapplication.h>
#include <lingmostylehelper/lingmostylehelper.h>
#include <signal.h>p corresponding interface
 *command format : lingmo-uninstall desktop_path
 */



/*heap mem , process dead auto free*/
char *p_pkg_name;
char *p_desktop_path;

QHash <QString, QStringList> irregularPackageList;
void sendmessg_notifily()
{
    QString dbus_service = "org.freedesktop.Notifications";
    QString dbus_path = "/org/freedesktop/Notifications";
    QString dbus_interface = "org.freedesktop.Notifications";
    QString notify = "Notify";
    QList<QVariant> argsset;
    QStringList action;
    QMap<QString,QVariant> hint;
    QString name = QObject::tr("lingmo-uninstaller");
    QString title = QObject::tr("Software uninstallation");
    QString string = QObject::tr("There is software being uninstalled. Please try again later.");
    uint32_t replaceid = 0;
    QString icon = "lingmo-installer";
    qDebug() << name;
    argsset << name
            << replaceid
            << icon
            << title
            << string
            << action
            << hint
            << -1;
    QDBusInterface *iface = new QDBusInterface(dbus_service, dbus_path, dbus_interface, QDBusConnection::sessionBus());
    iface->asyncCallWithArgumentList(notify,argsset);


}


QString getKaimingPakckageInfo(char *desktopPath)
{

    QString ret = "";
    QSettings settings(QString(desktopPath), QSettings::IniFormat);
    QStringList list = settings.allKeys();

    if (list.contains("Desktop Entry/X-Kaiming")){
        ret = settings.value("Desktop Entry/X-Kaiming").toString();
    }

    return ret;

}



void readIrregularPackageListFile()
{
    QFile file(IRREGULARPACKAGE_FILE_PATH);
    if (!file.exists()) {
        qWarning() << "不规则包列表文件不存在";
        return;
    }
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "文件打开失败";
        return;
    }
    irregularPackageList.clear();
    QTextStream in(&file);
    while (1) {
        QString line = in.readLine();
        if (line.size() == 0) {
            break;
        }
        QStringList lineList = line.split(":");
        QStringList valueList = QString(lineList.at(1)).split(",");
        irregularPackageList.insert(lineList.at(0), valueList);
    }
    return;
}

QString iterPackageHash(char *desktopName)
{


    if (desktopName == NULL) {
        return nullptr;
    }
    /*遍历hash*/
    QString desktopFileName = QString(desktopName);
    QHash<QString, QStringList>::const_iterator iter1 = irregularPackageList.constBegin();
    while (iter1 != irregularPackageList.constEnd()) {
        qDebug()  << "." << iter1.key() << ": " << iter1.value();
        foreach (QString desktop, iter1.value()) {
            QString removePathDesktop = desktop.split("/").last();
            if (desktopFileName.contains(removePathDesktop)) {
                return iter1.key();
            }
        }
        ++iter1;
    }
    return nullptr;
}

int getLocalPackageDesktopField(char *desktopPath)
{
    GError **error = NULL;
    GKeyFileFlags flags = G_KEY_FILE_NONE;
    GKeyFile *keyfile = g_key_file_new();
    g_key_file_load_from_file(keyfile, desktopPath, flags, error);
    char *keywords = g_key_file_get_locale_string(keyfile, "Desktop Entry", "Keywords", NULL, error);
    QString s_keywords(keywords);
    if (s_keywords == "Android;App;Apk") {
        return 1;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    qDebug() << "77777777777'";
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    // QApplication a(argc, argv);


    QString id = QString("lingmo-uninstaller-" + QLatin1String(getenv("DISPLAY")));
    /* 服务端存在检测包名是否存在功能，此处不再做检测 */
    kdk::QtSingleApplication a(id,argc, argv);

    QFileInfo file("/usr/share/qt5/translations/qt_zh_CN.qm");
    QTranslator * translator = new QTranslator();
    if(file.exists()!=false){
        translator->load("/usr/share/qt5/translations/qt_zh_CN.qm");
        a.installTranslator(translator);

    }
    a.setStyle("lingmo");
    //    std::string strTmp = argv[1];
    //    QString str = QString::fromStdString(strTmp);
    QFile filein(argv[1]);
    if (!filein.exists()){
        qDebug() << argv[1] << "    文件路径不存在!!!!!!";
        return 0;
    }
    signal(SIGCHLD, SIG_IGN);

    QTranslator *p_translator = new QTranslator;
    if (p_translator->load(QLocale() , QLatin1String("kre_uninstall") , QLatin1String("_") , QLatin1String("/usr/share/kre/kre-install/translations")))
        QApplication::installTranslator(p_translator);
    else
        qDebug() << "cannot load translator kre_uninstall" << QLocale::system().name() << ".qm";

    if (a.isRunning()) {
        a.sendMessage(QApplication::arguments().length() > 1 ? QApplication::arguments().at(1) : a.applicationFilePath());
        qDebug() << QObject::tr("lingmo-uninstaller is already running!");
        sendmessg_notifily();
        return EXIT_SUCCESS;
    }


    /*test print*/
    for (int i = 0 ; i < argc ; i++) {
        syslog(LOG_USER | LOG_INFO , "Info : main args is [%d->%s]\n" , i , argv[i]);
    }

    int i_ret = -1;
    signal(SIGCHLD, SIG_IGN);
    p_pkg_name = (char *)malloc(sizeof(char) * 1024);
    p_desktop_path = (char *)malloc(sizeof(char) * 1024);

    if (p_pkg_name == NULL || p_desktop_path == NULL) {
        printf("Error : apply heap mem fail\n");
        return -1;
    }

    if (argc != 2) {
        printf("Error : cmd format error [lingmo-uninstall desktop_path]\n");
        return -1;
    }

    /* set env value */
    setenv("QT_QPA_PLATFORMTHEME" , "lingmo" , true);
    readIrregularPackageListFile();
    QString packageName = iterPackageHash(argv[1]);


    /*get pkg_name and runtime*/

    char* p_pkgname;
    p_pkgname = (char*)malloc(1024);
    char p_runtime[1024];
    memset(p_pkgname , 0x00 , sizeof(p_pkgname));
    memset(p_runtime , 0x00 , sizeof(p_runtime));

    QString zh_name;

    if (argv[1] != NULL) {
        if (strstr(argv[1] , "lingmo-v10") != NULL) {
            /* this is compatible environment pkg */
            if (snow_analysis_xml(argv[1] , p_pkgname, p_runtime)) {
                free(p_pkgname);
                p_pkgname = nullptr;
                return -1;
            }
            else
            {
                free(p_pkgname);
                p_pkgname = nullptr;
            }
        } else {
            /* this is local pkg */
            if (packageName != nullptr) {
                QByteArray by = packageName.toLatin1();
                free(p_pkgname);
                p_pkgname = by.data();
            } else if (QString(argv[1]).contains(ANDROID_DESKTOP_FILE_PATH)) {
                packageName = getAndroidPackageName(QString(argv[1]));
                zh_name =get_zhname(QString(argv[1]));
            } else if (QString(argv[1]).contains("/usr/share/applications/") || QString(argv[1]).contains("/opt/kaiming/") ||QString(argv[1]).contains("/opt/kare/")){
                /*先判断是否是开明包*/
                packageName =  getKaimingPakckageInfo(argv[1]);
                if (packageName == ""){
                    packageName = snow_get_local_pkg_name(QString(argv[1]));
                    strcpy(p_runtime, LOCAL_ANDROID_PACKAGE_FLAG);
                    if (getLocalPackageDesktopField(argv[1])) {
                        strcpy(p_runtime, LOCAL_ANDROID_PACKAGE_FLAG);
                    } else {
                        strcpy(p_runtime, LOCAL_PACKAGE_FLAG);
                    }
                    if (!packageName.compare("kare")){
                        strcpy(p_runtime, "kare");
                    }

                }else {
                    strcpy(p_runtime, KAIMING_PACKAGE);
                }
                zh_name =get_zhname(QString(argv[1]));
            }
        }
    } else {
        printf("Info : main fun arg is error\n");
        free(p_pkgname);
        p_pkgname = nullptr;
        return -1;
    }
    free(p_pkgname);
    p_pkgname = nullptr;

    a.setWindowIcon(QIcon::fromTheme("lingmo-installer"));
    main_window w(packageName , p_runtime , argv[1],zh_name);
    /* handle double screen problem */
    w.resize(420,180);
    w.setFixedSize(420,180);
    w.showWindow();
    return a.exec();
}
