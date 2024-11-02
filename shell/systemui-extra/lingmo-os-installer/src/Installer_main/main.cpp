
#include "installermainwidget.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QStandardPaths>
#include <QTranslator>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
#include <QDebug>
#include <QLibraryInfo>
#include "klogger.h"
#include "ksetting.h"
#include "ksystemenv.h"
#include "kcommand.h"
#include "kdir.h"
#include "ksystemsetting_unit.h"
#include "../PluginService/base_unit/file_unit.h"
#include "installermainwidget_auto.h"
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
//#include <X11/extensions/Xrandr.h>
using namespace KServer;
using namespace KInstaller;



void x11_get_screen_size(int *width,int *height)
{
    Display* display;

    display = XOpenDisplay(NULL);
    if (display == NULL) {
        fprintf(stderr, "Cannot connect to X server %s/n", "simey:0");
        exit (-1);
    }
    int screen_num;

    screen_num = DefaultScreen(display);

    *width = DisplayWidth(display, screen_num);
    *height = DisplayHeight(display, screen_num);
    XCloseDisplay(display);

}

void XsettingsHidpi()
{
    Display    *dpy;
    int w,h;
    x11_get_screen_size (&w, &h);
    dpy = XOpenDisplay (NULL);
    if(h >= 2000 || (w>2560 && h> 1500)){

        XChangeProperty(dpy, RootWindow (dpy, 0),
            XA_RESOURCE_MANAGER, XA_STRING, 8, PropModeReplace, (unsigned char *) "Xft.dpi:	192\n", 13);
    }else{
        XChangeProperty(dpy, RootWindow (dpy, 0),
            XA_RESOURCE_MANAGER, XA_STRING, 8, PropModeReplace, (unsigned char *) "Xft.dpi:	96\n", 12);
    }
    XCloseDisplay (dpy);
}






void loadSetConfFile(QString flagstr)
{
    if(flagstr == "oem1") {
        KInstaller::Ksetting::init(":/data/file/ksettingsoem1.conf");
    } else if(flagstr == "oem2") {
        qDebug() << "oem2";
        KInstaller::Ksetting::init(":/data/file/ksettingsoem2.conf");
    } else {
        KInstaller::Ksetting::init(":/data/file/ksettings.conf");
    }
}
int main(int argc, char *argv[])
{

     //添加虚拟键盘
//    qputenv("QT_IM_MODULE", QByteArray("qtvirtualkeyboard"));

    //如果系统中已有实例程序在运行，则退出
    QString exePath = "/usr/bin/lingmo-os-Installer";
    int fd = open(exePath.toUtf8().data(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

    if (fd < 0)
        exit(1);
    if (lockf(fd, F_LOCK, 0)) {
        syslog(LOG_ERR, "Can't lock single file, lingmo-os-installer is already running!");
        qDebug()<<"Can't lock single file, lingmo-os-installer is already running!";
        exit(0);
    }



    XsettingsHidpi();





#if 0
    //自适应高分屏
    Display *display = XOpenDisplay(NULL);
    Screen *scrn = DefaultScreenOfDisplay(display);

    if(scrn == nullptr) {
        return 0;
    }
    int width = scrn->width;
    if(width >= 2560) {
#if(QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
        QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
        QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

        XCloseDisplay(display);

#endif
    }
#endif
#if(QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
        QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
        QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    qputenv("QT_AUTO_SCREEN_SCALE_FACTOR", "1");
    QApplication a(argc, argv);
    //自适应高清屏后，设置字体大小的改变
    QFont font = a.font();
    //这个值根据自身情况自行调节，如果字体没变化，把setPointSize()内的值继续增大
    font.setPointSize(font.pointSize());
    a.setFont(font);

    //设置系统主题样式
//  a.setStyle("lingmo-default");



    KServer::setLogPath("/var/log/installer" + LOG_FILE_NAME);
    qInstallMessageHandler(KServer::customLogMessageHandle);
#if 0
    //添加日志,QCommandLineParser解析应用的命令行参数
    QCommandLineParser parser;
    QCommandLineOption debugOpention({"d", "debug"}, QObject::tr("Show debug informations"));

    parser.addOptions({debugOpention});
    parser.process(a);

    if(parser.isSet(debugOpention)) {
        KServer::setLogLevel(QtDebugMsg);
        KServer::setLogPath(QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + LOG_FILE_NAME);
        qInstallMessageHandler(KServer::customLogMessageHandle);
    } else {
        setLogLevel(QtWarningMsg);
    }
#endif

  QString lang=  getenv("LANG");
  if(lang.isEmpty()){
      lang="zh_CN.UTF-8";
    unsetenv("LANGUAGE");
    setenv("LANGUAGE", lang.toLatin1().data(),1);
    unsetenv("LANG");
    setenv("LANG", lang.toLatin1().data(),1);
  }
  else
  {
      unsetenv("LANGUAGE");
      setenv("LANGUAGE", lang.toLatin1().data(),1);
  }

  QTranslator* translator = new QTranslator;
  QString path = KServer::GetLingmoInstallPath() + "/language/" + lang.section('.',0,0) + ".qm";
  translator->load(path);
  qApp->installTranslator(translator);

  QTranslator* translator1 = new QTranslator;
  translator1->load(KServer::GetLingmoInstallPath() + "/language/qt"+lang.section('.',0,0)+".qm");
  qApp->installTranslator(translator1);






//    KInstaller::Ksetting::init(":/data/file/ksettings.conf");
    KServer::KSystemENV::init();

    //("sudo", {"-u", "lingmo", "gsettings", "set" ,"org.lingmo.flash-disk.autoload" ,"ifautoload", "false"});
    //root  ->  lingmo

    InstallerMainWidget* w = nullptr;
    InstallerMainWidget_auto* w_auto = nullptr;

    if(argc > 1) {
        if(QString(QLatin1String(argv[1])).contains("oem")) {
            QString strtype = "oem2";
            if(QString(QLatin1String(argv[2])).contains("automatic")) {
                w_auto = new InstallerMainWidget_auto(true);
                w_auto->show();
            } else {
                loadSetConfFile(strtype);
                w = new InstallerMainWidget;
                w->setInstallMode(strtype);
                w->show();
            }
        }
    } else {
        //2021-3-29 huawei swap file swapoff
        KServer::KCommand::initCmd()->getInstance()->RunScripCommand("swapoff", {"-a"});
        KServer::KCommand::initCmd()->getInstance()->RunScripCommand("sudo", {"-u", "lingmo", "gsettings", "set" ,"org.lingmo.flash-disk.autoload" ,"ifautoload", "false"});
        QString readfile = KServer::KReadFile("/proc/cmdline");
        if(readfile.contains("automatic") ||
                readfile.contains("ple-mode") ||
                readfile.contains("test-mode")) {
            loadSetConfFile("");
            w_auto = new InstallerMainWidget_auto(false);
            w_auto->show();

        } else if(KServer::KReadFile("/proc/cmdline").contains("oem-config/enable=true")) {
            QString strtype = "oem1";
            loadSetConfFile(strtype);
            w = new InstallerMainWidget;
            w->setInstallMode(strtype);
            w->show();
        } else {
            loadSetConfFile("");
            w = new InstallerMainWidget;
            w->show();
        }

    }



    int result = a.exec();
    if(w_auto) {
        delete w_auto;
    }
    if(w) {
        delete w;
    }

    return result;
}
