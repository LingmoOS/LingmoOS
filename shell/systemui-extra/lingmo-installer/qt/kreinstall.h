#ifndef KREINSTALL_H
#define KREINSTALL_H

#include "debinfowidget.h"
#include "batch-install.h"
#include "titlewidget.h"
#include "dbuscallthread.h"
#include "lingmoinstalldbuscallthread.h"
#include <sys/types.h>
#include <pwd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/file.h>
#include <syslog.h>
#include <unistd.h>
#include <QStringLiteral>
#include <QDesktopServices>
#include <QTextEdit>
#include <QListWidget>
#include <QTimer>
#include <QProgressBar>
#include <form.h>
#include <kaboutdialog.h>
#include <kmenubutton.h>
#include <QThreadPool>
#include <user_manual.hpp>
#include <QStorageInfo>
#include <unistd.h>
#include <stdio.h>
#include <KWindowSystem>
#include <QApt/DebFile>
#include <QMimeDatabase>
#include <lingmostylehelper/lingmostylehelper.h>
#include "ksoundeffects.h"
#include "kareclass.h"
#include <windowmanager/windowmanager.h>
#include <QScreen>


using namespace kdk;

#ifndef RUNTIME_XML_FILE_PATH
#define RUNTIME_XML_FILE_PATH "/opt/kre/config/runtimes.xml"
#endif

#define LOCK_FILE_PATH "/tmp/lock/lingmo-update.lock"
#define LOCK_PATH "/tmp/lock/"

#define PROGRAM_NAME "lingmo-installer"
#define PATH_MAX_LEN 1024
#define PID_STRING_LEN 64

#define ORG_LINGMO_STYLE            "org.lingmo.style"
#define GSETTING_KEY              "systemFontSize"
#define XSBC_LINGMO_RUNTIME_ENV       "XSBC-KyRuntimeEnv"
#define XSBC_PERMISSIONS_LIST     "XSBC-PermissionsList"
#define KRE_VALUE                 "v10"
#define OS_VERSION_FILE_PATH      "/etc/lsb-release"
#define OS_VERSION_DESCRIPTION    "DISTRIB_DESCRIPTION"
#define OS_V10_PRO_VERSION        "\"Lingmo V10 SP1\""
#define OS_V10_VERSION            "\"Lingmo V10\""
#define OS_V10_PRO_LOGO           "lingmo-10.1"
#define OS_V10_LOGO               "lingmo-v10"

#define XML_APPLICATIONS_PATH "/opt/kre/config/applications_v2.xml"


class kreInstall : public QWidget
{
    Q_OBJECT
public:
    kreInstall(QString debPath);
    ~kreInstall();

    kdk::KAboutDialog *userabout ;
    QVBoxLayout      *m_pallLayout;
    debInfoWidget    *m_pdebWidget;
    titleWidget      *m_ptitleWidget;

    QHBoxLayout      *m_pinstHLayout;
    QLabel           *m_pruntimeLabel;
    QWidget          *m_pruntimeWidget;

    QLabel           *m_pinstToLabel;
    QLabel           *m_pinstTypeLabel;

    QRadioButton     *m_pgeneralBtn;
    QRadioButton     *m_pseniorBtn;
    QWidget          *m_ptypeAndRadioWidget;
    QWidget          *m_pradioBtnWidget;
    QWidget          *m_pinstTypeWidget;
    QHBoxLayout      *m_ptypeAndRadioLayout;
    QVBoxLayout      *m_pradioBtnLayout;
    QVBoxLayout      *m_pinstTypeLayout;

    QWidget          *m_ppermissionsWidget;
    QWidget          *m_pblackWidget;
    QGridLayout      *m_ppermissionsLayout;
    QCheckBox        *m_px11Btn;
    QCheckBox        *m_psystemdbusBtn;
    QCheckBox        *m_psessiondbusBtn;
    QCheckBox        *m_pnetworkBtn;
    QCheckBox        *m_pipcBtn;
    QCheckBox        *m_pcupsBtn;
    QCheckBox        *m_ppulseaudioBtn;
    QStringList       m_permissionList;
    QHash<QString, QCheckBox*> m_CheckBoxValue;

    QStackedWidget   *m_pAllStackedWidget;
    QVBoxLayout      *m_pAllStackedLayout;

    QProgressBar     *pProgressBar;
    QLabel           *progress;
    QWidget          *m_pInstallBeforeWidget;

    QWidget          *m_pInstallBtnWidget;
    QHBoxLayout      *m_pInstallBtnLayout;
    QPushButton      *m_pInstallBtn;

    QWidget          *m_pInstallIngWidget;
    QVBoxLayout      *m_pInstallIngLayout;
    QWidget          *m_pInstallIngLabelWidget;
    QHBoxLayout      *m_pInstallIngLabelLayout;
    QLabel           *m_pInstallIngLabel;
    debInfoWidget    *m_pdebInstallIngWidget;
    titleWidget      *m_ptitleInstallIngWidget;

    QWidget          *m_pInstallFailWidget;
    QVBoxLayout      *m_pInstallFailLayout;
    QWidget          *m_pInstallFailLabelWidget;
    QWidget          *m_pInstallFailbuttonWidget;
    QVBoxLayout      *m_pInstallFailLabelLayout;
    QHBoxLayout      *m_pInstallFailbuttonLayout;
    QLabel           *m_pInstallFailLabel;
    QLabel           *m_pInstallFailIcon;

    titleWidget      *m_ptitleInstallFailWidget;

    QWidget          *m_pInstallSuccessWidget;
    QVBoxLayout      *m_pInstallSuccessLayout;
    QWidget          *m_pInstallSuccessLabelWidget;
    QVBoxLayout      *m_pInstallSuccessLabelLayout;
    QLabel           *m_pInstallSuccessLabel;
    QLabel           *m_pInstallSuccessIcon;
    QPushButton      *m_pInstallSuccessButton;
    titleWidget      *m_ptitleInstallSuccessWidget;

    /*安装日志界面*/
    QWidget          *m_pInstallLogWidget;
    QLabel           *m_pInstallLogLabel;
    QHBoxLayout      *m_pInstallLOGLabelLayout;
    QHBoxLayout      *m_pInstallLOGLabelTextLayout;
    QVBoxLayout      *m_fiallogtextlayout;
    QHBoxLayout      *m_pInstallLogbuttonlayout;
    QPushButton      *m_pInstallLogbutton;
    QLabel           *m_pInstallLogShowLabel;
    QTextEdit        *m_pInstallLogtextedit;
    QPushButton      *m_pInstallFaillog;
    QPushButton      *m_pInstallFailsure;
    QPushButton      *return_homepage_log1;

    QHBoxLayout      *m_pInstallLOGwidgetLayout;
    QWidget          *m_pInstallLogWidget_min;

    /*********批量安装界面******/
    QWidget          *Batch_installwidget;
    QVBoxLayout      *Batch_installlayout;
    QVBoxLayout      *Batch_installlayout_ALL;
    QLabel           *Batch_installIcon;
    QLabel           *Batch_installdescribe;
    QPushButton      *Batch_installselect;

    QListWidget      *batch_listwidget;


    QWidget          *Batch_installwidget_1;
    QWidget          *Batch_installwidget_1a;
    QWidget          *Batch_installwidget_1b;
    QPushButton      *Batch_addinstall;
    QPushButton      *Batch_install;
    QPushButton      *Batch_viewlog;
    QPushButton      *Batch_return;
    QHBoxLayout      *Batch_installlayout_batch;
    QHBoxLayout      *Batch_installlayout_add;

    QString          flagstring;

    /**********批量安装完成后的界面*******/
    QWidget          *Batch_installsuccesswidget;
    QWidget          *Batch_installsuccessiconwidget;
    QWidget          *Batch_installsuccesstextwidget;
    QVBoxLayout      *Batch_installsuccesslayout;
    QHBoxLayout      *Batch_installsuccessbuttonlayout;
    QVBoxLayout      *Batch_installsuccesslayout_ALL;
    QLabel           *Batch_installsuccessIcon;
    QLabel           *Batch_installsuccesstext;
    QPushButton      *Batch_installsuccessdetemin;
    QPushButton      *Batch_installsuccessreturn;

    Batchinstall     *batchwidget1;
    int bete = 0;

    bool SECENTSTART = false;  //判断是否是第二次覆盖启动
    bool EXITQMESSG = false;



    /* 安装到系统路径界面，之前的灵墨安装器重写 */
    // 主界面
    QWidget         *m_pOsInstallWidget;
    titleWidget     *m_pOstitleInstallWidget;
    debInfoWidget   *m_pOsDebMainWidget;
    QVBoxLayout     *m_pOsVboxLayoutWidget;
    QPushButton     *m_pOsInstallButton;
    QPushButton     *m_pOsInstallButton_test;
    QPushButton     *m_cansel_InstallButton;
    QLabel          *m_installed_tips;

    //安装中界面
    QWidget         *m_pOsInstallIngWidget;
    titleWidget     *m_pOstitleInstallIngWidget;
    debInfoWidget   *m_pOsDebInstallIngWidget;
    QVBoxLayout     *m_pOsVboxLayoutInstallIngWidget;
    QLabel          *m_pOsInstallIngLabel;


    QString          m_debPath;
    QString          m_runtime;
    QString          m_appName;
    QString          m_debArch;
    QString          m_appVersion;
    QString          m_application;
    QString          m_applicationZh;
    QString          m_installed_version;
    QString          lingmo_installerversion;
    QString          m_installed_status;
    QString          m_OsVersionLogo;
    int              m_lineNum = 0;
    QGSettings       *m_pGsettingFontSize;
    QStringList      m_lingmoRuntimeEnvList;

    QTimer *ftime;

    QTimer *icontime;

//    Menuworks* task;



    DbusCallThread   *m_pDbusCall;
    LingmoInstalldbusCallThread  *m_pLingmoInstalldbusCallThread;

    kareClass *kare;

   // QStringList      *debPath_all;
    QStringList * debPath_all1 = new QStringList();
    QStringList * debPath_all2 = new QStringList();

    QMimeType mime;
    //记住批量安装的下标
    int batch_dex = 0;

    int listwidgetcount = 0;

    int install_flag = 0;

    bool CACLINFO;

    int canceladd = 0;

    bool CANSCELISNTALL = false;

    QMap<QString, QString> maplist;

    QString path_deb;//用来记住包名相同版本不同时判断的包的路径选择；

    int path_debint = 0;

    bool appinstallstatus = true;

    QString change_installapp;

    long settime = 0;

    bool set_about = false;

    bool FilePath = true;

    int fontsize;

    bool isInstallKare = false;

    QString installKareImage;

    QString historyPath = QDir::homePath();


    bool polkit();

    void initDebInfo(QString debPath);

    void mainWidgetInit();
    void set_ui_actions();

    void gsettingInit();

    void debArchIsMatch();
    void deb_Damage_info(); //弹窗提示
    void snapArchIsMatch();

    bool BatchinstallArchIsMatch(QString depath,QString tmp); //将deb包导入到批量安装列表

    bool ArchIsMatch(QString depath_arch,QString tmp);

    void initOsMainWidget();

    void initOsLayoutWidget();

    bool eventFilter(QObject *watched, QEvent *event); //事件过滤器


    void initOsInstallIngWidget();

    void initOsLayoutIngWidget();


    void initInstallBtnWidget();
    void initPermissionsList(QString debPath);

    void getOsVersionValue();

    void getControlfield();

    QFont  getSystemFont(int size);

    bool   isOsTree();



    QString getDebInfoFromDeb(QString debPath,QString type);

      QString getDebInfoFromDeb_md5(QString debPath);
    QString      SpliteText(QFontMetrics font, QString text,int nlablesize);

    bool parseRuntimeXml(const QString &fileName);

    bool kreInStallLock();

    void kreInStallUnlock();

    void isFile();

    void resetDebInfo();

    void reset_button_lable();

    void getKreAppDesktopFileName();

    bool getAppStatus();

    QString getUserName();

    QString getUid();

    bool isAndroidReady(const QString &username, const QString &uid);

    int isAndroidEnvInstalled();

    void batch_installWidgeted(QStringList debpath);

    void deletewidgetitem(QString itemwidget);
    void changebuttonstatus(QString itemwidget,int dex);

    void adddebpathcompail(QString tmp);

    bool QMessageBox_listchanged(QString debpathstring);



    void thread_install_deb();

    bool app_installed(QString debpathstring);

    void app_installed_fast();


    // 弹窗ui
    // 执行安装命令的子进程
    QProcess* process;


    void showWindow(){
            kdk::LingmoUIStyleHelper::self()->removeHeader(this);
            this->show();
            QRect availableGeometry = qApp->primaryScreen()->availableGeometry();
            kdk::WindowManager::setGeometry(this->windowHandle(),QRect((availableGeometry.width() - this->width())/2, (availableGeometry.height() - this->height())/2,this->width(),this->height()));
        }

signals:
    void pushitemname(QString name);

protected:
    void paintEvent(QPaintEvent *event);

public slots:
    void installBtnClicked();

    void seniorBtnClicked();

    void generalBtnClicked();

    void minBtnClicked();

    void closeBtnClicked(bool status);
   // void closeBtnClicking();

    void installSuccessSlot();

    void installFailSlot();

    void InstalldingStatus(int status);

    void batchInstalldingStatus(int status,QString appname);

    void OsInstallButtonSlots();

    void Batch_OsInstallButtonSlots();

    int  installerdpkg();
    //单个安装完成信号槽
    void osInstallOverSoftwareAptSlots(bool status, QString msg);
    //批量安装完成信号槽
    void Batch_osInstallOverSoftwareAptSlots(bool status, QString msg);


    void secondaryInstallation(QString debPath);

    void showinstallfaillogtext();

    void return_homepage();

    void batchlist();

    void onAnalysisApkFile(QString debPath);

    void automatic_deletewidgetitem();

    void play_installingpicture();

    void cancel_installed();
    void continue_install();
    void canseaddapplication();
    void replace_application();


};
#endif // KREINSTALL_H
