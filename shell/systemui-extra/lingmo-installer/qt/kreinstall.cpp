#include <unistd.h>
#include "kreinstall.h"
#include <QPainterPath>
#include <dlfcn.h>
#include <QCoreApplication>
#include <QApt/Backend>
#include <QDesktopWidget>
#include "widbox.h"
#include <QShortcut>



/* export value save lock file fd */
int gi_fd;
bool install_status = false;

kreInstall::kreInstall(QString deb_path)
{

    qDebug() << "=-=============123123123";

    kare = new kareClass;
    if (!isOsTree()){

        this->hide();

        if(kare->kareImageList.size() != 0){
            qDebug()<< "存在kare";
            kare->show();
            QEventLoop loop;
            connect(kare->okButton, &QPushButton::clicked, &loop, &QEventLoop::quit);
            loop.exec();
            kdk::LingmoUIStyleHelper::self()->removeHeader(this);

            this->show();
        }
    }

    qDebug() << "=====================" << kare->kareImage;


    QFile filepath(deb_path);
    //  this->installEventFilter(this);
    if(filepath.exists()){
        FilePath = true;
    }
    else
        FilePath = false;

    this->setAttribute(Qt::WA_AlwaysShowToolTips);

    QDBusMessage message = QDBusMessage::createMethodCall("com.lingmo.systemupgrade", "/com/lingmo/systemupgrade", "com.lingmo.systemupgrade", "InstallDebFile");
    QList<QVariant> args = message.arguments();


    m_ptitleWidget = new titleWidget(this);
    m_ptitleWidget->show();

    connect(m_ptitleWidget->m_pMinBtn,SIGNAL(clicked(bool)),this,SLOT(minBtnClicked()));
    connect(m_ptitleWidget->m_pClostBtn,SIGNAL(clicked(bool)),this,SLOT(closeBtnClicked(bool)));

    historyPath = QDir::homePath();
    /* 初始化deb信号，获取deb信息 */

    //debArchIsMatch();
    initDebInfo(deb_path);

    /*判断文件是否存在*/
    isFile();
    /*判断包架构*/
    //debArchIsMatch();
    /*系统字体大小gsetting初始化*/
    gsettingInit();
    // 灵墨安装器启动界面
    initOsMainWidget();
    initOsLayoutWidget();


    // 灵墨安装器安装中界面
    initOsInstallIngWidget();

    initOsLayoutIngWidget();

    // 往兼容环境安装界面初始化
    //  initPermissionsWidget();

    // 初始化权限链表hash表
    //  initCheckBoxHash();
    // 初始化选择界面和安装按钮界面
    // initSelectWidget();

    initInstallBtnWidget();

    // 所有界面初始化
    mainWidgetInit();

    // 根据deb包中扩展属性字段，来设置应用权限列表
    // initPermissionsList(m_debPath);

    // 根据系统下的lsb-release文件，来判别当前系统版本
    getOsVersionValue();

    // 根据control获取是兼容环境的包还是系统环境的包
    //getControlfield();

    m_pAllStackedWidget->setCurrentIndex(4);

    //    int a debArchIsMatch();
    //    this->show();
    //F1按键显示用户手册
    QShortcut *shortcut = new QShortcut(QKeySequence(Qt::Key_F1), this);
    connect(shortcut, &QShortcut::activated, this, [=]{
        //显示用户手册
        kdk::kabase::UserManualManagement userManualTest;
        if (!userManualTest.callUserManual("lingmo-installer")) {
            qCritical() << "user manual call fail!";
        }
    });


}

kreInstall::~kreInstall()
{
    delete batchwidget1;
    kreInStallUnlock();

}

void kreInstall::isFile()
{
    if(FilePath == false){
        return;
    }
    QFileInfo file(m_debPath);
    if (file.exists()==false) {
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("open file error"));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText(tr(" file does not exist "));
        QPushButton *btn = new QPushButton();
        btn->setText(tr("OK"));
        msgBox.addButton(btn, QMessageBox::AcceptRole);
        msgBox.exec();
        exit(0);
    }
}

void kreInstall::deb_Damage_info()
{
    //弹窗提示
    // QString  errroinfo = tr("The software package is corrupted and cannot be installed!");
    QString  errroinfo = QObject::tr("The software package is corrupted and cannot be installed!");

    QScopedPointer<QWidget> widget(new widbox(errroinfo));
    widget->show();
}

void kreInstall::debArchIsMatch()
{

    QProcess *process = new QProcess();
    process->start("dpkg --print-architecture");
    process->waitForFinished(-1);
    QString arch = process->readLine();
    arch.remove("\n");
    if (arch.compare(m_debArch) && m_debArch.compare("all")) {
        if (m_debArch != ""){
            QString  errroinfo = tr("The software package architecture does not match the native architecture");
            QScopedPointer<QWidget> widget(new widbox(errroinfo));
            //a.setActivationWindow(widget);
            widget->show();
        }
        else
        {
            QString  errroinfo = tr("The package format is not Debian format and cannot be opened.");
            QScopedPointer<QWidget> widget(new widbox(errroinfo));
            //a.setActivationWindow(widget);
            widget->show();
        }
    }
    return ;
}

//snap包架构不符时调用。
void kreInstall::snapArchIsMatch()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle(tr("open file error"));
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(tr("The software package architecture does not match the native architecture"));
    QPushButton *btn = new QPushButton();
    btn->setText(tr("OK"));
    msgBox.addButton(btn, QMessageBox::AcceptRole);
    msgBox.exec();
    msgBox.setContextMenuPolicy(Qt::NoContextMenu);
    exit(0);
}

//批量安裝時架構不符調用
bool kreInstall::ArchIsMatch(QString depath_arch,QString tmp){
    QProcess *process = new QProcess();
    process->start("dpkg --print-architecture");
    process->waitForFinished(-1);
    QString arch = process->readLine();
    arch.remove("\n");

    if (arch.compare(depath_arch) && depath_arch.compare("all")) {
        if (arch != ""){
            QMessageBox msgBox;
            msgBox.setWindowTitle(tr("open file error"));
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.setText(tr("The software package architecture does not match the native architecture"));
            QPushButton *btn = new QPushButton();
            btn->setText(tr("OK"));
            msgBox.addButton(btn, QMessageBox::AcceptRole);
            msgBox.exec();
            msgBox.setContextMenuPolicy(Qt::NoContextMenu);
            return false;
            exit(0);

        }else{
            QMessageBox msgBox;
            msgBox.setWindowTitle(tr("open file error"));
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.setText(tr("The package format is not Debian format and cannot be opened."));
            QPushButton *btn = new QPushButton();
            btn->setText(tr("OK"));
            msgBox.addButton(btn, QMessageBox::AcceptRole);
            msgBox.exec();
            msgBox.setContextMenuPolicy(Qt::NoContextMenu);
            return false;
            exit(0);

        }
    }
    return true;
}

void kreInstall::gsettingInit()
{
    const QByteArray style_id(ORG_LINGMO_STYLE);
    m_pGsettingFontSize = new QGSettings(style_id);
    connect(m_pGsettingFontSize, &QGSettings::changed, this, [=] (const QString &key){
        if (key==GSETTING_KEY) {

            m_pInstallSuccessLabel->setFont(getSystemFont(18));
            m_pInstallFailLabel->setFont(getSystemFont(18));
            m_pOsDebInstallIngWidget->debAppNameLabel->setFont(getSystemFont(14));
            m_pOsInstallButton->setFont(getSystemFont(14));
            m_pInstallFaillog->setFont(getSystemFont(14));
            m_pInstallFailsure->setFont(getSystemFont(14));
            if(FilePath == false){
                Batch_installselect->setFont(getSystemFont(14));
                Batch_installdescribe->setFont(getSystemFont(14));
                Batch_install->setFont(getSystemFont(14));
                Batch_addinstall->setFont(getSystemFont(14));
                Batch_viewlog->setFont(getSystemFont(14));
                Batch_return->setFont(getSystemFont(14));
                // QString appname_test = getDebInfoFromDeb(flagstring, "Package");
                // QString zh_nameapp =  m_pdebWidget->getAppCnName(appname_test);
                for(int i = 0; i< batch_listwidget->count();++i){
                    QListWidgetItem *item1 = batch_listwidget->item(i);
                    QWidget * widget2 = batch_listwidget->itemWidget(item1);
                    if(widget2 !=NULL){
                        QList <QLabel*> labellist = widget2->findChildren<QLabel*>();
                        foreach(QLabel *label,labellist){
                            label->setFont(getSystemFont(14));
                        }
                    }
                }
            }
            /*重设安装到系统的界面数据*/
            m_pOsDebMainWidget->resetAppInfo(m_appName, m_appVersion);
            m_pOsDebInstallIngWidget->resetAppInfo(m_appName, m_appVersion);
            /*重设安装到兼容环境的界面数据*/
            m_pdebWidget->resetAppInfo(m_appName, m_appVersion);
            m_pdebInstallIngWidget->resetAppInfo(m_appName, m_appVersion);

        }
        if (key==ORG_LINGMO_STYLE) {
            qDebug() << "llllllllll" <<  key;
        }
        else{
            //主题变化
            if(FilePath == false){
                QString themeNow = m_pGsettingFontSize->get("styleName").toString();
                if (themeNow == "lingmo-dark")
                {
                    Batch_installIcon->clear();
                    Batch_installIcon->setPixmap(QPixmap(":/image/batch-install-dark.png"));
                    //  Batch_installIcon->setPixmap(QIcon::fromTheme(":/image/batch-install-dark.svg").pixmap(QSize(216,216)));
                }
                else{
                    Batch_installIcon->setPixmap(QPixmap(":/image/batch-install.png"));
                }
            }
        }
    });
}

void kreInstall::initDebInfo(QString debPath)
{
    QProcess terminal;
    QProcess terminal_1;
    QProcess terminal_2;
    QStringList cmdList;
    QStringList cmdList1;
    QStringList cmdList2;
    QString m_app_path;
    m_debPath = debPath;

    if(FilePath == false){
        m_debPath = debPath;
        return;
    }

    QString debPathname = m_debPath;
    if (debPathname.contains("`",Qt::CaseSensitive) || debPathname.contains("$(",Qt::CaseSensitive) || debPathname.contains(";",Qt::CaseSensitive)
            || debPathname.contains("&",Qt::CaseSensitive) || debPathname.contains("|",Qt::CaseSensitive) ){
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText(tr("Open file error\n"
                          "Naming contains nonstandard characters '` $( & | '"));
        QPushButton *btn = new QPushButton();
        btn->setText(tr("OK"));
        msgBox.addButton(btn, QMessageBox::AcceptRole);
        msgBox.exec();
        exit(0);
    }

    QMimeDatabase db;
    mime = db.mimeTypeForFile(m_debPath);
    if(mime.name().contains("vnd.snap",Qt::CaseSensitive))
    {
        cmdList << "-c" << QString("snap info "+ debPath + " |grep name");
        terminal.start("/bin/bash",cmdList);
        terminal.waitForFinished();
        m_appName = terminal.readAll();
        m_appName = m_appName.left(m_appName.length() - 1);
        QStringList list = m_appName.split(":");
        m_appName = list[list.count()-1];
        m_appName =m_appName.simplified();

        cmdList1 << "-c" << QString("snap info "+ debPath + " |grep version");
        terminal.start("/bin/bash",cmdList1);
        terminal.waitForFinished();
        m_appVersion = terminal.readAll();
        m_appVersion = m_appVersion.left(m_appVersion.length() - 2);
        list = m_appVersion.split(":");
        m_appVersion = list[list.count()-1];
        m_appVersion = m_appVersion.simplified();
        m_appVersion = m_appVersion.trimmed();

        m_debArch ="";
        m_debPath = debPath;
        return;
    }
    else if(mime.name().contains("vnd.android.package-archive",Qt::CaseSensitive)){
        onAnalysisApkFile(debPath);
        m_debPath = debPath;
    }
    else{
        m_debPath = debPath;
        m_appName = getDebInfoFromDeb(m_debPath, "Package");
        
        if(m_appName == "The software package is damaged"){
            deb_Damage_info();
            return;
        }
        m_appName = m_appName.toLower();


        m_appVersion = getDebInfoFromDeb(m_debPath, "Version");
        m_debArch = getDebInfoFromDeb(m_debPath,"Architecture");
        m_app_path = m_appName;
        m_app_path.push_front(' ');
        m_app_path.push_front('"');
        m_app_path.push_back(' ');
        m_app_path.push_back('"');
        cmdList << "-c" << QString("dpkg -l | grep "+ m_app_path + "| awk '{print $3}' | sed -n '1p'").toStdString().c_str();
        terminal.start("/bin/bash",cmdList);
        terminal.waitForFinished();


        m_installed_version = terminal.readAll();//获取此包已安装的版本号

        m_installed_version = m_installed_version.left(m_installed_version.length() - 1);

        cmdList2 << "-c" << QString("dpkg -l | grep "+ m_app_path + "| awk '{print $1}' | sed -n '1p'").toStdString().c_str();
        terminal_2.start("/bin/bash",cmdList2);
        terminal_2.waitForFinished();
        QString install_status = terminal_2.readAll();
        install_status = install_status.left(install_status.length() - 1);
        if (install_status != "ii")
            m_installed_version = "";

        cmdList1 << "-c" << "dpkg -l | grep "+ m_app_path + "| awk '{print $1}' | sed -n '1p'";
        terminal_1.start("/bin/bash",cmdList1);
        terminal_1.waitForFinished();
        m_installed_status = terminal_1.readAll();//获取此包已安装的版本号
        m_installed_status = m_installed_status.left(m_installed_status.length() - 1);



    }
    //  debArchIsMatch();



    return;
}


void kreInstall::mainWidgetInit()
{



    this->setFixedSize(500,600);
    this->setWindowTitle(QObject::tr("Installer"));
    /********************安装前界面********************/


    m_pInstallBeforeWidget = new QWidget();
    m_pallLayout = new QVBoxLayout();
    m_pallLayout->setContentsMargins(0, 0, 0, 0);
    m_pallLayout->setSpacing(0);
    m_pdebWidget = new debInfoWidget(m_appName,m_appVersion);

    // m_ptitleWidget->setStyleSheet("background-color:red;");

    //    connect(m_ptitleWidget->m_pMinBtn,SIGNAL(clicked(bool)),this,SLOT(minBtnClicked()));
    //    connect(m_ptitleWidget->m_pClostBtn,SIGNAL(clicked(bool)),this,SLOT(closeBtnClicked(bool)));

    m_pruntimeLabel = new QLabel();
    m_pruntimeLabel->setFixedSize(300,31);
    m_pinstToLabel  = new QLabel();
    m_pinstToLabel->setText(tr("install:"));
    m_pinstToLabel->adjustSize();

    m_pruntimeWidget = new QWidget();
    m_pruntimeWidget->setFixedHeight(31);
    m_pinstHLayout = new QHBoxLayout();
    m_pinstHLayout->setContentsMargins(0, 0, 0, 0);
    m_pinstHLayout->setSpacing(0);
    m_pinstHLayout->addItem(new QSpacerItem(83, 10, QSizePolicy::Fixed));
    m_pinstHLayout->addWidget(m_pinstToLabel);
    m_pinstHLayout->addWidget(m_pruntimeLabel);
    m_pinstHLayout->addItem(new QSpacerItem(106, 10, QSizePolicy::Fixed));
    m_pruntimeWidget->setLayout(m_pinstHLayout);



    m_pallLayout->addItem(new QSpacerItem(1, 4, QSizePolicy::Fixed));

    m_pallLayout->addItem(new QSpacerItem(1, 30, QSizePolicy::Expanding));
    m_pallLayout->addWidget(m_pdebWidget);

    m_pallLayout->addWidget(m_pruntimeWidget);

    m_pallLayout->addItem(new QSpacerItem(1, 8, QSizePolicy::Expanding));
    //   m_pallLayout->addWidget(m_ptypeAndRadioWidget);

    m_pallLayout->addItem(new QSpacerItem(10,12,QSizePolicy::Fixed));
    m_pallLayout->addWidget(m_pblackWidget);
    m_pallLayout->addItem(new QSpacerItem(10,75,QSizePolicy::Fixed));
    m_pallLayout->addWidget(m_pInstallBtnWidget);
    m_pallLayout->addItem(new QSpacerItem(10, 24, QSizePolicy::Fixed));

    m_pInstallBeforeWidget->setLayout(m_pallLayout);

    /*****************安装中界面**********************/
    m_pInstallIngWidget = new QWidget();
    m_pInstallIngLayout = new QVBoxLayout();
    m_pInstallIngLayout->setContentsMargins(0, 0, 0, 0);
    m_pInstallIngLayout->setSpacing(0);

    m_pInstallIngLabelWidget = new QWidget();
    m_pInstallIngLabelWidget->setFixedHeight(20);
    m_pInstallIngLabelLayout = new QHBoxLayout();
    m_pInstallIngLabelLayout->setContentsMargins(0, 0, 0, 0);
    m_pInstallIngLabelLayout->setSpacing(0);
    m_pInstallIngLabel = new QLabel();
    m_pInstallIngLabel->setText(tr("installing!"));

    m_pInstallIngLabelLayout->addWidget(m_pInstallIngLabel);
    m_pInstallIngLabelWidget->setLayout(m_pInstallIngLabelLayout);

    m_pdebInstallIngWidget = new debInfoWidget(m_appName,m_appVersion);

    m_pInstallIngLayout->addWidget(m_pdebInstallIngWidget);
    m_pInstallIngLayout->addSpacing(0);
    m_pInstallIngLayout->addWidget(m_pInstallIngLabelWidget);


    connect(m_ptitleWidget->m_menu,&QMenu::triggered,this, [=](QAction *  act){
        if (act->text() == tr("About")){
            QStringList cmdList1;
            QProcess terminal_1;
            cmdList1 << "-c" << "dpkg -l | grep lingmo-installer | awk '{print $3}' | sed -n '1p'";
            terminal_1.start("/bin/bash",cmdList1);
            terminal_1.waitForFinished();
            lingmo_installerversion = terminal_1.readAll();//获取此包已安装的版本号
            lingmo_installerversion = lingmo_installerversion.left(lingmo_installerversion.length() - 1);
            const QIcon icon = QIcon::fromTheme("lingmo-installer");
            QString appname;
            QString version;
            appname = tr("lingmo-installer");
            version = tr("version") + "：" +lingmo_installerversion;
            const QString appinfo = "";
            kdk::KAboutDialog *userabout = new kdk::KAboutDialog(nullptr,icon,appname,version,appinfo);
            userabout->setWindowModality(Qt::WindowModality::ApplicationModal);
            userabout->show();
        }
        else if (act->text() == tr("Help")){
            kdk::kabase::UserManualManagement userManualTest;
            if (!userManualTest.callUserManual("lingmo-installer")) {
                qCritical() << "user manual call fail!";
            }
        }
        else{
            exit(0);
        }
    });



    m_pInstallIngWidget->setLayout(m_pInstallIngLayout);

    /*****************安装成功界面********************/
    m_pInstallSuccessWidget = new QWidget();
    m_pInstallSuccessLayout = new QVBoxLayout();
    m_pInstallSuccessLayout->setContentsMargins(0, 0, 0, 0);
    m_pInstallSuccessLayout->setSpacing(0);

    m_pInstallSuccessLabelWidget = new QWidget();
    m_pInstallSuccessLabelLayout = new QVBoxLayout();
    m_pInstallSuccessLabelLayout->setContentsMargins(0, 0, 0, 0);
    m_pInstallSuccessLabelLayout->setSpacing(0);
    m_pInstallSuccessIcon = new QLabel();
    m_pInstallSuccessIcon->setPixmap(QPixmap(":/image/install-success.png"));
    m_pInstallSuccessIcon->setAlignment(Qt::AlignCenter);
    m_pInstallSuccessLabel = new QLabel();
    m_pInstallSuccessLabel->adjustSize();
    m_pInstallSuccessLabel->setText(tr("install success"));

    m_pInstallSuccessLabel->setFont(getSystemFont(18));
    m_pInstallSuccessLabel->adjustSize();
    m_pInstallSuccessLabel->setAlignment(Qt::AlignCenter);

    m_pInstallSuccessButton = new QPushButton;
    m_pInstallSuccessButton->setText(QObject::tr("determine"));
    m_pInstallSuccessButton->adjustSize();
    m_pInstallSuccessButton->setMinimumWidth(160);





    m_pInstallSuccessLabelLayout->addWidget(m_pInstallSuccessIcon,0, Qt::AlignHCenter);
    m_pInstallSuccessLabelLayout->addStretch();
    m_pInstallSuccessLabelLayout->addWidget(m_pInstallSuccessLabel, 0, Qt::AlignHCenter);
    m_pInstallSuccessLabelLayout->addSpacing(80);
    m_pInstallSuccessLabelLayout->addWidget(m_pInstallSuccessButton,0, Qt::AlignHCenter);
    m_pInstallSuccessLabelWidget->setLayout(m_pInstallSuccessLabelLayout);
    connect(m_pInstallSuccessButton,SIGNAL(clicked(bool)),this,SLOT(closeBtnClicked(bool)));
    m_pInstallSuccessLayout->addItem(new QSpacerItem(1, 180, QSizePolicy::Fixed));
    m_pInstallSuccessLayout->addWidget(m_pInstallSuccessLabelWidget);
    m_pInstallSuccessLayout->addItem(new QSpacerItem(1, 157, QSizePolicy::Expanding));
    m_pInstallSuccessWidget->setLayout(m_pInstallSuccessLayout);


    /*****************安装失败界面********************/

    m_pInstallFailWidget = new QWidget();
    m_pInstallFailLayout = new QVBoxLayout();
    m_pInstallFailLayout->setContentsMargins(0, 0, 0, 0);
    m_pInstallFailLayout->setSpacing(0);

    /*安装失败的icon和安装失败字样*/
    m_pInstallFailLabelWidget = new QWidget();
    //m_pInstallFailLabelWidget->setFixedHeight(48);
    m_pInstallFailLabelLayout = new QVBoxLayout();
    m_pInstallFailLabelLayout->setContentsMargins(0, 0, 0, 0);
    m_pInstallFailLabelLayout->setSpacing(0);

    /*查看日志按钮 和 确认按钮*/
    m_pInstallFailbuttonWidget = new QWidget();
    m_pInstallFailbuttonWidget->setFixedHeight(48);
    m_pInstallFailbuttonLayout = new QHBoxLayout();
    m_pInstallFailbuttonLayout->setContentsMargins(0, 0, 0, 0);
    m_pInstallFailbuttonLayout->setSpacing(0);

    m_pInstallFailIcon = new QLabel();
    m_pInstallFailIcon->setPixmap(QPixmap(":/image/install-fail.png"));
    m_pInstallFailLabel = new QLabel();
    m_pInstallFailLabel->adjustSize();
    m_pInstallFailLabel->setText(tr("install Fail"));

    m_pInstallFaillog = new QPushButton(this);
    m_pInstallFailsure = new QPushButton(this);
    m_pInstallFaillog->setText(QObject::tr("view log"));
    m_pInstallFaillog->adjustSize();
    m_pInstallFaillog->setFixedSize(160,36);
    m_pInstallFailsure->setText(QObject::tr("determine"));
    m_pInstallFailsure->adjustSize();
    m_pInstallFailsure->setFixedSize(160,36);


    m_pInstallFailLabel->setFont(getSystemFont(18));

    m_pInstallFaillog->setFont(getSystemFont(14));

    m_pInstallFaillog->setFont(getSystemFont(14));


    m_pInstallFailLabelLayout->addWidget(m_pInstallFailIcon,0, Qt::AlignHCenter);
    m_pInstallFailLabelLayout->addWidget(m_pInstallFailLabel,0, Qt::AlignHCenter);
    m_pInstallFailLabelLayout->addSpacing(120);
    m_pInstallFailLabelWidget->setLayout(m_pInstallFailLabelLayout);


    m_pInstallFailbuttonLayout->addSpacing(82);
    m_pInstallFailbuttonLayout->addWidget(m_pInstallFailsure);
    m_pInstallFailbuttonLayout->setSpacing(16);
    m_pInstallFailbuttonLayout->addWidget(m_pInstallFaillog);
    m_pInstallFailbuttonLayout->addSpacing(82);


    m_pInstallFailbuttonWidget->setLayout(m_pInstallFailbuttonLayout);


    connect(m_pInstallFaillog,SIGNAL(clicked(bool)),this,SLOT(showinstallfaillogtext()));
    connect(m_pInstallFailsure,SIGNAL(clicked(bool)),this,SLOT(closeBtnClicked(bool)));

    m_pInstallFailLayout->addSpacing(180);
    m_pInstallFailLayout->addWidget(m_pInstallFailLabelWidget);
    m_pInstallFailLayout->addWidget(m_pInstallFailbuttonWidget);
    m_pInstallFailLayout->addSpacing(148);
    m_pInstallFailWidget->setLayout(m_pInstallFailLayout);


    /***********************查看日志界面**********************/

    m_pInstallLogWidget = new QWidget();
    m_pInstallLOGLabelLayout = new QHBoxLayout();
    m_pInstallLOGLabelLayout->setContentsMargins(0, 0, 0, 0);
    m_pInstallLOGLabelLayout->setSpacing(0);

    m_pInstallLogLabel = new QLabel();
    m_pInstallLogLabel->setText(QObject::tr("Installation log"));
    m_pInstallLogLabel->adjustSize();
    //     QFont m_pInstallLogLabelFont = m_pInstallFailLabel->font();
    //     m_pInstallLogLabelFont.setPixelSize(getSystemFont());
    m_pInstallLogLabel->setFont(getSystemFont(14));

    m_pInstallLOGLabelLayout->addItem(new QSpacerItem(30, 30, QSizePolicy::Fixed));
    m_pInstallLOGLabelLayout->addWidget(m_pInstallLogLabel);
    m_pInstallLOGLabelLayout->addItem(new QSpacerItem(1, 20, QSizePolicy::Fixed));



    m_pInstallLogWidget_min = new QWidget();
    m_pInstallLOGwidgetLayout = new QHBoxLayout();
    m_pInstallLOGwidgetLayout->setContentsMargins(0, 0, 0, 0);
    m_pInstallLOGwidgetLayout->setSpacing(0);


    m_pInstallLOGLabelTextLayout = new QHBoxLayout();
    m_pInstallLOGLabelTextLayout->setContentsMargins(0, 0, 0, 0);
    m_pInstallLOGLabelTextLayout->setSpacing(0);

    m_pInstallLogtextedit = new QTextEdit(this);

    m_pInstallLogtextedit->setReadOnly(true);
    m_pInstallLogtextedit->setFrameShape(QFrame::NoFrame);
    m_pInstallLogtextedit->setAutoFillBackground(false);

    m_pInstallLOGLabelTextLayout->addItem(new QSpacerItem(0, 20, QSizePolicy::Fixed));
    m_pInstallLOGLabelTextLayout->addWidget(m_pInstallLogtextedit);
    m_pInstallLOGLabelTextLayout->addItem(new QSpacerItem(0, 20, QSizePolicy::Fixed));

    m_pInstallLogWidget_min->setLayout(m_pInstallLOGLabelTextLayout);
    m_pInstallLogWidget_min->setAutoFillBackground(true);
    m_pInstallLogWidget_min->setBackgroundRole(QPalette::Window);

    m_pInstallLOGwidgetLayout->addItem(new QSpacerItem(30, 0, QSizePolicy::Fixed));
    m_pInstallLOGwidgetLayout->addWidget(m_pInstallLogWidget_min);
    m_pInstallLOGwidgetLayout->addItem(new QSpacerItem(30, 0, QSizePolicy::Fixed));




    m_pInstallLogbuttonlayout = new QHBoxLayout();
    m_pInstallLogbuttonlayout->setContentsMargins(0, 0, 0, 0);
    m_pInstallLogbuttonlayout->setSpacing(0);

    m_pInstallLogbutton = new QPushButton(this);
    m_pInstallLogbutton->setText(QObject::tr("determine"));
    m_pInstallLogbutton->adjustSize();
    m_pInstallLogbutton->setFont(getSystemFont(14));
    m_pInstallLogbutton->setMaximumWidth(180);

    return_homepage_log1 = new QPushButton();
    return_homepage_log1->setText(tr("Return home"));
    return_homepage_log1->adjustSize();
    return_homepage_log1->setFont(getSystemFont(14));
    return_homepage_log1->setMaximumWidth(180);
    return_homepage_log1->hide();

    connect(return_homepage_log1,SIGNAL(clicked(bool)),this,SLOT(return_homepage()));
    m_pInstallLogbuttonlayout->addItem(new QSpacerItem((550 - (m_pInstallFaillog->width()*2))/2+60, 10, QSizePolicy::Fixed));
    m_pInstallLogbuttonlayout->addWidget(return_homepage_log1);
    m_pInstallLogbuttonlayout->addSpacing(20);
    m_pInstallLogbuttonlayout->addWidget(m_pInstallLogbutton);
    m_pInstallLogbuttonlayout->addItem(new QSpacerItem(30, 0, QSizePolicy::Fixed));
    m_fiallogtextlayout = new QVBoxLayout();
    m_fiallogtextlayout->setContentsMargins(0, 0, 0, 0);
    m_fiallogtextlayout->setSpacing(0);

    connect(m_pInstallLogbutton,SIGNAL(clicked(bool)),this,SLOT(batchlist()));


    m_fiallogtextlayout->addItem(new QSpacerItem(0, 30, QSizePolicy::Fixed));
    m_fiallogtextlayout->addLayout(m_pInstallLOGLabelLayout);
    m_fiallogtextlayout->addItem(new QSpacerItem(0, 15, QSizePolicy::Fixed));
    m_fiallogtextlayout->addLayout(m_pInstallLOGwidgetLayout);
    m_fiallogtextlayout->addItem(new QSpacerItem(0, 30, QSizePolicy::Fixed));
    m_fiallogtextlayout->addLayout(m_pInstallLogbuttonlayout);
    m_fiallogtextlayout->addItem(new QSpacerItem(0, 30, QSizePolicy::Fixed));

    m_pInstallLogWidget->setLayout(m_fiallogtextlayout);


    /******************************批量安装的批量界面*****************/
    Batch_installwidget_1 = new QWidget(this);
    batch_listwidget = new QListWidget(this);
    batch_listwidget->setFrameShape(QFrame::NoFrame);
    Batch_installlayout_batch = new QHBoxLayout();

    Batch_install = new QPushButton();
    Batch_addinstall = new QPushButton();
    Batch_viewlog = new QPushButton();
    Batch_return  = new QPushButton();
    Batch_install->setText(tr("Install"));
    Batch_addinstall->setText(tr("Add"));
    Batch_viewlog->setText(tr("View log"));
    Batch_return->setText(tr("Return"));

    //     Batch_install->setFont(getSystemFont(14));
    //     Batch_addinstall->setFont(getSystemFont(14));
    //     Batch_viewlog->setFont(getSystemFont(14));
    //     Batch_return->setFont(getSystemFont(14));

    Batch_install->setMinimumWidth(160);
    Batch_addinstall->setMinimumWidth(160);
    Batch_viewlog->setMaximumHeight(160);
    Batch_viewlog->hide();
    Batch_return->hide();
    connect(Batch_viewlog,SIGNAL(clicked(bool)),this,SLOT(showinstallfaillogtext()));
    connect(Batch_return,SIGNAL(clicked(bool)),this,SLOT(return_homepage()));
    connect(Batch_addinstall, &QPushButton::clicked, this, [=] () {
        QString historyDir = historyPath;
        QFileDialog dialog;
        dialog.setFileMode(QFileDialog::ExistingFiles);
        dialog.setNameFilter("Debian Package Files (*.deb)");
        dialog.setDirectory(historyDir);

        const int mode = dialog.exec();
        if (mode != QDialog::Accepted) {
            return;
        }

        const QStringList selected_files = dialog.selectedFiles();
        if (selected_files.size() > 0){
            QFileInfo info(selected_files.at(0));
            historyPath = info.absolutePath();
        }
        batch_installWidgeted(selected_files);
    });

    ftime = new QTimer();
    ftime->setInterval(1000);
    connect( ftime,SIGNAL(timeout()), this, SLOT(automatic_deletewidgetitem()));

    icontime = new QTimer();
    icontime->setInterval(100);
    connect( icontime,SIGNAL(timeout()), this, SLOT(play_installingpicture()));
    connect(Batch_install, &QPushButton::clicked, this, &kreInstall::Batch_OsInstallButtonSlots);
    Batch_installwidget_1a = new QWidget(this);
    Batch_installlayout_batch = new QHBoxLayout();
    Batch_installlayout_batch->setContentsMargins(24, 0, 24, 0);
    Batch_installlayout_batch->setSpacing(0);
    Batch_installlayout_batch->addWidget(batch_listwidget);
    Batch_installwidget_1a->setLayout(Batch_installlayout_batch);

    Batch_installwidget_1b = new QWidget(this);
    Batch_installlayout_add = new QHBoxLayout();
    Batch_installlayout_add->setContentsMargins(82, 0, 82, 0);
    Batch_installlayout_add->setSpacing(0);
    Batch_installlayout_add->addWidget(Batch_addinstall);
    Batch_installlayout_add->setSpacing(16);
    Batch_installlayout_add->addWidget(Batch_install);
    Batch_installlayout_add->setSpacing(16);
    Batch_installlayout_add->addWidget(Batch_return);
    Batch_installlayout_add->setSpacing(16);
    Batch_installlayout_add->addWidget(Batch_viewlog);

    Batch_installwidget_1b->setLayout(Batch_installlayout_add);

    Batch_installlayout_ALL = new QVBoxLayout();
    Batch_installlayout_ALL->setContentsMargins(0, 0, 0, 0);
    Batch_installlayout_ALL->setSpacing(0);


    Batch_installlayout_ALL->addItem(new QSpacerItem(24, 5, QSizePolicy::Fixed));

    Batch_installlayout_ALL->addWidget(Batch_installwidget_1a);

    Batch_installlayout_ALL->addItem(new QSpacerItem(92, 10, QSizePolicy::Fixed));

    Batch_installlayout_ALL->addWidget(Batch_installwidget_1b);

    Batch_installlayout_ALL->addItem(new QSpacerItem(0, 48, QSizePolicy::Fixed));

    Batch_installwidget_1->setLayout(Batch_installlayout_ALL);

    /*******************批量安装完成后的界面***********/

    Batch_installsuccesswidget = new QWidget(this);

    Batch_installsuccessiconwidget = new QWidget();

    Batch_installsuccesstextwidget = new QWidget();

    Batch_installsuccessdetemin = new QPushButton();
    Batch_installsuccessreturn  = new QPushButton();

    Batch_installsuccessreturn->setText(tr("Return"));
    Batch_installsuccessdetemin->setText(tr("Determine"));
    Batch_installsuccessreturn->setMinimumWidth(160);
    Batch_installsuccessreturn->setMinimumWidth(160);
    connect(Batch_installsuccessreturn,SIGNAL(clicked(bool)),this,SLOT(return_homepage()));
    connect(Batch_installsuccessdetemin,SIGNAL(clicked(bool)),this,SLOT(closeBtnClicked(bool)));

    Batch_installsuccesslayout = new QVBoxLayout();
    Batch_installsuccesslayout->setContentsMargins(0, 0, 0, 0);
    Batch_installsuccesslayout->setSpacing(0);


    QHBoxLayout *Batch_installsuccesslayout1 = new QHBoxLayout();
    Batch_installsuccesslayout1->setContentsMargins(0, 200, 0, 0);
    Batch_installsuccesslayout1->setSpacing(0);

    Batch_installsuccessIcon = new QLabel();
    Batch_installsuccessIcon->setPixmap(QIcon::fromTheme("lingmo-dialog-success").pixmap(QSize(64,64)));
    Batch_installsuccessIcon->adjustSize();

    Batch_installsuccesslayout1->addWidget(Batch_installsuccessIcon);
    Batch_installsuccesslayout1->setAlignment(Qt::AlignHCenter);

    QHBoxLayout *Batch_installsuccesslayout2 = new QHBoxLayout();
    Batch_installsuccesslayout2->setContentsMargins(0, 0, 0, 0);
    Batch_installsuccesslayout2->setSpacing(0);

    Batch_installsuccesstext = new QLabel();
    Batch_installsuccesstext->adjustSize();
    Batch_installsuccesstext->setText(tr("Installation is complete"));


    //     QFont font2_sucess =  Batch_installsuccesstext->font();
    //     font2_sucess.setPixelSize(getSystemFont());
    Batch_installsuccesstext->setFont(getSystemFont(14));

    Batch_installsuccesslayout2->addWidget(Batch_installsuccesstext);
    Batch_installsuccesslayout2->setAlignment(Qt::AlignHCenter);


    Batch_installsuccesslayout->addLayout(Batch_installsuccesslayout1);
    Batch_installsuccesslayout->setSpacing(16);
    Batch_installsuccesslayout->addLayout(Batch_installsuccesslayout2);

    Batch_installsuccessiconwidget->setLayout(Batch_installsuccesslayout);

    Batch_installsuccessbuttonlayout = new QHBoxLayout();
    Batch_installsuccessbuttonlayout->setContentsMargins(82, 0, 82, 0);
    Batch_installsuccessbuttonlayout->setSpacing(0);
    Batch_installsuccessbuttonlayout->addWidget(Batch_installsuccessreturn);
    Batch_installsuccessbuttonlayout->setSpacing(16);
    Batch_installsuccessbuttonlayout->addWidget(Batch_installsuccessdetemin);
    Batch_installsuccesstextwidget->setLayout(Batch_installsuccessbuttonlayout);

    Batch_installsuccesslayout_ALL = new QVBoxLayout();
    Batch_installsuccesslayout_ALL->setContentsMargins(0, 0, 0, 0);
    Batch_installsuccesslayout_ALL->setSpacing(0);

    Batch_installsuccesslayout_ALL->addWidget(Batch_installsuccessiconwidget);
    Batch_installsuccesslayout_ALL->addWidget(Batch_installsuccesstextwidget);

    Batch_installsuccesswidget->setLayout(Batch_installsuccesslayout_ALL);



    /*QStackedWidget将所有界面排序起来。用 m_pAllStackedWidget->setCurrentIndex(?)去显示，首页从0开始*/
    m_pAllStackedWidget = new QStackedWidget();
    m_pAllStackedWidget->addWidget(m_pInstallBeforeWidget);
    m_pAllStackedWidget->addWidget(m_pInstallIngWidget);
    m_pAllStackedWidget->addWidget(m_pInstallSuccessWidget);
    m_pAllStackedWidget->addWidget(m_pInstallFailWidget);
    m_pAllStackedWidget->addWidget(m_pOsInstallWidget);
    m_pAllStackedWidget->addWidget(m_pOsInstallIngWidget);
    m_pAllStackedWidget->addWidget(m_pInstallLogWidget);
    m_pAllStackedWidget->addWidget(Batch_installwidget_1);
    m_pAllStackedWidget->addWidget(Batch_installsuccesswidget);

    m_pAllStackedLayout = new QVBoxLayout();
    m_pAllStackedLayout->setContentsMargins(0, 0, 0, 0);
    m_pAllStackedLayout->setSpacing(0);
    m_pAllStackedLayout->addSpacing(3);
    m_pAllStackedLayout->addWidget(m_ptitleWidget);

    m_pAllStackedLayout->addSpacing(10);

    m_pAllStackedLayout->addWidget(m_pAllStackedWidget);
    this->setLayout(m_pAllStackedLayout);
}

/* 初始化安装到系统的安装器界面 */
void kreInstall::initOsMainWidget()
{
    m_pOsInstallWidget = new QWidget();
    m_pOsInstallWidget->setContentsMargins(0, 0, 0, 0);

    m_pOsDebMainWidget = new debInfoWidget(m_appName, m_appVersion);
    //    QFont font2 =  m_pOsDebMainWidget->debAppNameLabel->font();
    //    font2.setPixelSize(getSystemFont());
    //    m_pOsDebMainWidget->debAppNameLabel->setFont(getSystemFont(18));



    m_pOsVboxLayoutWidget = new QVBoxLayout();
    m_pOsVboxLayoutWidget->setContentsMargins(0, 0, 0, 0);
    m_pOsVboxLayoutWidget->setSpacing(0);


    m_pOsInstallButton = new QPushButton();
    //    QFont font = m_pOsInstallButton->font();
    //    font.setPixelSize(getSystemFont());
    //    m_pOsInstallButton->setFont(getSystemFont(14));
    m_pOsInstallButton->setText(QObject::tr("one-click install"));
    m_pOsInstallButton->setMaximumHeight(36);
    m_pOsInstallButton->setMinimumSize(130,36);


    connect(m_pOsInstallButton, &QPushButton::clicked, this, &kreInstall::OsInstallButtonSlots);

    return;
}

/*处理QT中纯英文在QLable中的换行问题*/
QString kreInstall::SpliteText(QFontMetrics font, QString text,int nlablesize)
{
    int nTextSize = font.width(text);
    if(nTextSize > nlablesize){
        int nPos =0;
        long nOffset =0;
        for (int i =0;i < text.size();i++){
            nOffset +=font.width(text.at(i));
            if(nOffset >= nlablesize){
                nPos = i;
                break;
            }
        }
        nPos = (nPos -1< 0)? 0:nPos -1;
        QString qstrLeftData =text.left(nPos);
        QString qstrMidData = text.mid(nPos);
        return qstrLeftData + "\n" +SpliteText(font,qstrMidData,nlablesize);
    }

    return text;

}

/* 初始化布局安装到系统的安装器界面 */
void kreInstall::initOsLayoutWidget()
{
    if(FilePath == false){
        /******************************批量安装的初始界面*****************/
        Batch_installwidget = new QWidget();
        Batch_installlayout = new QVBoxLayout();
        Batch_installlayout->setContentsMargins(0, 0, 0, 0);
        Batch_installlayout->setSpacing(0);
        Batch_installIcon = new QLabel();
        //点击"添加"按钮选择软件包进行安装
        QString themeNow = m_pGsettingFontSize->get("styleName").toString();
        if (themeNow == "lingmo-dark")
        {
            Batch_installIcon->clear();
            Batch_installIcon->setPixmap(QPixmap(":/image/batch-install-dark.png"));
            // Batch_installIcon->setPixmap(QIcon::fromTheme(":/image/batch-install-dark.svg").pixmap(QSize(216,216)));
        }
        else{
            Batch_installIcon->setPixmap(QPixmap(":/image/batch-install.png"));
        }
        Batch_installdescribe = new QLabel();
        QString info = tr("Click the \"add\" button to select the");
        QString info1 = tr("software package for installation");
        QLocale locale;
        if (locale.language() == QLocale::Chinese)
            Batch_installdescribe->setText(info +info1);
        else
            Batch_installdescribe->setText(info +"\n"+ info1);

        Batch_installdescribe->setWordWrap(true);
        Batch_installdescribe->setAlignment(Qt::AlignCenter);
        Batch_installdescribe->adjustSize();
        Batch_installselect = new QPushButton();
        Batch_installselect->setText(QObject::tr("Add"));
        Batch_installselect->adjustSize();
        //        QFont Batch_installselectfont = Batch_installselect->font();
        //        Batch_installselectfont.setPixelSize(getSystemFont());
        Batch_installselect->setFont(getSystemFont(14));
        Batch_installdescribe->setFont(getSystemFont(14));
        Batch_installselect->setMinimumWidth(180);
        Batch_installselect->setMinimumHeight(36);



        QHBoxLayout *ButtonHBoxLayout = new QHBoxLayout();
        ButtonHBoxLayout->setContentsMargins(0, 0, 0, 0);
        ButtonHBoxLayout->setSpacing(0);

        ButtonHBoxLayout->addWidget(Batch_installIcon, 0, Qt::AlignHCenter);



        QHBoxLayout *ButtonlabelLayout = new QHBoxLayout();
        ButtonlabelLayout->setContentsMargins(0, 0, 0, 0);
        ButtonlabelLayout->setSpacing(0);
        ButtonlabelLayout->addWidget(Batch_installdescribe, 0, Qt::AlignHCenter);


        QHBoxLayout *ButtonlabelLayout_select = new QHBoxLayout();
        ButtonlabelLayout_select->setContentsMargins(0, 0, 0, 0);
        ButtonlabelLayout_select->setSpacing(0);
        ButtonlabelLayout_select->addWidget(Batch_installselect, 0, Qt::AlignHCenter);


        Batch_installlayout = new QVBoxLayout();
        Batch_installlayout->setContentsMargins(0, 0, 0, 0);
        Batch_installlayout->setSpacing(0);

        Batch_installlayout->addStretch();
        Batch_installlayout->addWidget(Batch_installIcon,0, Qt::AlignHCenter);
        Batch_installlayout->addSpacing(10);
        Batch_installlayout->addWidget(Batch_installdescribe);
        Batch_installlayout->addSpacing(40);
        Batch_installlayout->addWidget(Batch_installselect,0, Qt::AlignHCenter);
        Batch_installlayout->addStretch();


        m_pOsInstallWidget->setLayout(Batch_installlayout);

        connect(Batch_installselect, &QPushButton::clicked, this, [=] () {
            QString historyDir = historyPath;
            QFileDialog dialog;
            dialog.setFileMode(QFileDialog::ExistingFiles);
            dialog.setNameFilter("Debian Package Files (*.deb)");
            dialog.setDirectory(historyDir);

            const int mode = dialog.exec();
            if (mode != QDialog::Accepted) {
                return;
            }

            const QStringList selected_files = dialog.selectedFiles();
            if (selected_files.size() > 0){
                QFileInfo info(selected_files.at(0));
                historyPath = info.absolutePath();
            }
            batch_installWidgeted(selected_files);

        });
    }

    else{
        // deb信息界面布局
        QWidget *debWidget = new QWidget();
        debWidget->setContentsMargins(0, 0, 0, 0);

        QHBoxLayout *debHboxLayout = new QHBoxLayout();
        debHboxLayout->setContentsMargins(0, 0, 0, 0);
        debHboxLayout->setSpacing(0);
        debHboxLayout->addItem(new QSpacerItem(0, 10, QSizePolicy::Fixed));
        debHboxLayout->addWidget(m_pOsDebMainWidget);
        debHboxLayout->setSpacing(0);
        debWidget->setLayout(debHboxLayout);

        // 安装按钮布局
        QWidget *buttonWidget = new QWidget();
        buttonWidget->setContentsMargins(0, 0, 0, 0);
        QVBoxLayout *ButtonHBoxLayout = new QVBoxLayout();
        ButtonHBoxLayout->setContentsMargins(0, 0, 0, 0);
        ButtonHBoxLayout->setSpacing(0);
        ButtonHBoxLayout->addWidget(m_pOsInstallButton);
        ButtonHBoxLayout->setAlignment(Qt::AlignHCenter);
        ButtonHBoxLayout->addSpacing(110);
        buttonWidget->setLayout(ButtonHBoxLayout);



        m_pOsVboxLayoutWidget->addWidget(debWidget);
        m_pOsVboxLayoutWidget->addWidget(buttonWidget);
        m_pOsInstallWidget->setLayout(m_pOsVboxLayoutWidget);
    }
    return;
}

/* 初始化变量安装中变量 */
void kreInstall::initOsInstallIngWidget()
{
    m_pOsInstallIngWidget = new QWidget();
    m_pOsInstallIngWidget->setContentsMargins(0, 0, 0, 0);


    m_pOsDebInstallIngWidget = new debInfoWidget(m_appName, m_appVersion);
    //    QFont font2 =  m_pOsDebInstallIngWidget->debAppNameLabel->font();
    //    font2.setPixelSize(getSystemFont());
    m_pOsDebInstallIngWidget->debAppNameLabel->setFont(getSystemFont(18));


    m_pOsVboxLayoutInstallIngWidget = new QVBoxLayout();
    m_pOsVboxLayoutInstallIngWidget->setContentsMargins(0, 0, 0, 0);
    m_pOsVboxLayoutInstallIngWidget->setSpacing(0);

    m_pOsInstallIngLabel = new QLabel();
    m_pOsInstallIngLabel->setText(QObject::tr("In the installation..."));

    pProgressBar = new QProgressBar();
    pProgressBar->setOrientation(Qt::Horizontal);  // 水平方向
    pProgressBar->setMinimum(0);  // 最小值
    pProgressBar->setMaximum(100);  // 最大值
    pProgressBar->setValue(1);  // 当前进度
    pProgressBar->setFixedWidth(240);
    pProgressBar->setFixedHeight(6);
    pProgressBar->setTextVisible(false);

    progress = new QLabel();
    progress->setText("1%");
    progress->adjustSize();

    return;
}

/* 初始化变量安装中布局 */
void kreInstall::initOsLayoutIngWidget()
{
    // deb信息界面布局
    QWidget *debIngWidget = new QWidget();
    debIngWidget->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *debHboxLayout = new QHBoxLayout();
    debHboxLayout->setContentsMargins(0, 0, 0, 0);
    debHboxLayout->setSpacing(0);
    debHboxLayout->addItem(new QSpacerItem(0, 10, QSizePolicy::Fixed));
    debHboxLayout->addWidget(m_pOsDebInstallIngWidget);
    debHboxLayout->setAlignment(Qt::AlignCenter);
    debHboxLayout->setSpacing(0);
    // debHboxLayout->addItem(new QSpacerItem(0, 10, QSizePolicy::Expanding));
    debIngWidget->setLayout(debHboxLayout);

    // 安装按钮布局
    QWidget *LabelWidget = new QWidget();
    LabelWidget->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *LabelHBoxLayout = new QVBoxLayout();
    LabelHBoxLayout->setContentsMargins(0, 0, 0, 0);
    LabelHBoxLayout->setSpacing(0);

    QVBoxLayout *allVBoxLayout = new QVBoxLayout();
    allVBoxLayout->setContentsMargins(0, 0, 0, 0);
    allVBoxLayout->setSpacing(0);
    allVBoxLayout->setAlignment(Qt::AlignCenter);

    LabelHBoxLayout->addWidget(m_pOsInstallIngLabel);
    LabelHBoxLayout->setAlignment(Qt::AlignHCenter);

    QHBoxLayout *qprogressbarHBoxLayout = new QHBoxLayout();
    qprogressbarHBoxLayout->setContentsMargins(0, 0, 0, 0);
    qprogressbarHBoxLayout->setSpacing(0);
    qprogressbarHBoxLayout->setAlignment(Qt::AlignCenter);
    qprogressbarHBoxLayout->addSpacing(110);
    qprogressbarHBoxLayout->addWidget(pProgressBar);
    qprogressbarHBoxLayout->addSpacing(25);
    qprogressbarHBoxLayout->addWidget(progress);
    qprogressbarHBoxLayout->addStretch();

    allVBoxLayout->setAlignment(Qt::AlignCenter);
    allVBoxLayout->addLayout(LabelHBoxLayout);
    allVBoxLayout->addSpacing(15);
    allVBoxLayout->addLayout(qprogressbarHBoxLayout);
    allVBoxLayout->addSpacing(145);
    LabelWidget->setLayout(allVBoxLayout);

    m_pOsVboxLayoutInstallIngWidget->addWidget(debIngWidget);
    m_pOsVboxLayoutInstallIngWidget->addWidget(LabelWidget);
    m_pOsInstallIngWidget->setLayout(m_pOsVboxLayoutInstallIngWidget);

    return;
}

void kreInstall::initInstallBtnWidget()
{
    m_pblackWidget = new QWidget();
    m_pblackWidget->setContentsMargins(0, 0, 0, 0);
    m_pblackWidget->setFixedHeight(100);

    m_pInstallBtnWidget = new QWidget();
    m_pInstallBtnWidget->setFixedHeight(36);
    m_pInstallBtnLayout = new QHBoxLayout();
    m_pInstallBtnLayout->setContentsMargins(0, 0, 0, 0);
    m_pInstallBtnLayout->setSpacing(0);

    m_pInstallBtn = new QPushButton();
    m_pInstallBtn->setFixedSize(100,36);
    m_pInstallBtn->setText(tr("install"));
    connect(m_pInstallBtn,SIGNAL(clicked(bool)),this,SLOT(installBtnClicked()));

    m_pInstallBtnLayout->addItem(new QSpacerItem(426,10,QSizePolicy::Expanding));
    m_pInstallBtnLayout->addWidget(m_pInstallBtn);
    m_pInstallBtnLayout->addItem(new QSpacerItem(24, 10, QSizePolicy::Fixed));
    m_pInstallBtnWidget->setLayout(m_pInstallBtnLayout);
}

/* 初始化权限列表，以及权限展示CheckBox UI界面 */
void kreInstall::initPermissionsList(QString debPath)
{
    QString s_Permissions = getDebInfoFromDeb(debPath, XSBC_PERMISSIONS_LIST);
    m_permissionList.clear();
    if (s_Permissions.isNull()) {
        m_permissionList << "x11" << "system-dbus" << "session-dbus" << "ipc" \
                         << "network" << "pulseaudio" << "cups";
    } else {
        m_permissionList = s_Permissions.split(',');
    }

    for(int i = 0; i < m_permissionList.count(); i++) {
        QCheckBox *value = m_CheckBoxValue.value(m_permissionList.at(i));
        if (value != nullptr) {
            value->setChecked(true);
        }
    }
    return;
}


void kreInstall::getOsVersionValue()
{
    QFile file(OS_VERSION_FILE_PATH);
    QString line;
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);  //用文件构造流
        line = in.readLine();//读取一行放到字符串里
        while (!line.isNull()) {
            line = in.readLine();//循环读取下行
            if (line.contains(OS_VERSION_DESCRIPTION)) {
                break;
            }
        }
    }
    file.close();

    QStringList lineStringList = line.split("=");
    QString OsVersionLogo = lineStringList.at(1);
    if (OsVersionLogo == OS_V10_PRO_VERSION) {
        m_OsVersionLogo = OS_V10_PRO_LOGO;
    } else if (OsVersionLogo == OS_V10_VERSION) {
        m_OsVersionLogo = OS_V10_LOGO;
    }
    return;
}

QFont kreInstall::getSystemFont(int size)
{
    double fontsize = (m_pGsettingFontSize->get(GSETTING_FONT_KEY).toFloat());

    double lableBaseFontSize = size * 9/12;//魔鬼数字，自行处理
    double nowFontSize = lableBaseFontSize * double(fontsize) / 11;//11为系统默认大小，魔鬼数字，自行处理
    QFont font;
    if (size == 18){
        font.setFamily("Noto Sans CJK SC");
        font.setWeight(500);

    }
    font.setBold(false);
    font.setPointSizeF(nowFontSize);


    return font;
}

void kreInstall::getControlfield()
{
    QString env = getDebInfoFromDeb(m_debPath, XSBC_LINGMO_RUNTIME_ENV);
    QStringList envStringList = env.split(",");

    // 当没有扩展属性字段时，直接往系统里面装，不进行其余操作
    if (envStringList.count() == 0) {
        m_pAllStackedWidget->setCurrentIndex(4);
        return;
    }

    // 根据扩展属性字段去匹配，当有多个字段时，是否能匹配到与本机系统相匹配的字段
    for (int i = 0; i < envStringList.count(); i++) {
        if (envStringList.at(i) == m_OsVersionLogo) {
            // 包的版本和系统已匹配，直接往系统里面装
            m_pAllStackedWidget->setCurrentIndex(4);
            return;
        }
    }

    if(!parseRuntimeXml(RUNTIME_XML_FILE_PATH))
    {
        m_pAllStackedWidget->setCurrentIndex(4);
        return;
    }

    bool MatchResults = false;
    for (int i = 0; i < envStringList.count(); i++) {
        for (int j = 0; j < m_lingmoRuntimeEnvList.count(); j++) {
            // 当兼容环境的名称和包的名称匹配上了时，则直接往该兼容中安装，如果有俩个字段匹配时，则选择后者
            if (m_lingmoRuntimeEnvList.at(j) == envStringList.at(i)) {
                m_runtime = m_lingmoRuntimeEnvList.at(j);
                m_pAllStackedWidget->setCurrentIndex(0);
                MatchResults = true;
            }
        }
    }

    // 说明什么都没有匹配到，直接往系统里面装
    if (!MatchResults) {
        m_pAllStackedWidget->setCurrentIndex(4);
    }
    return;
}
/*批量安装进度返回显示*/
void kreInstall::batchInstalldingStatus(int status, QString appinstall_name)
{
    disconnect(m_pLingmoInstalldbusCallThread, &LingmoInstalldbusCallThread::lingmobatchOsinstalldingStatus, \
               this, &kreInstall::batchInstalldingStatus);

    QString appname_test = getDebInfoFromDeb(appinstall_name, "Package");
    QString zh_nameapp =  m_pdebWidget->getAppCnName(appname_test);
    for(int i = 0; i< batch_listwidget->count();++i){
        QListWidgetItem *item1 = batch_listwidget->item(i);
        QWidget * widget2 = batch_listwidget->itemWidget(item1);
        if(widget2 !=NULL){
            QList <QLabel*> labellist = widget2->findChildren<QLabel*>();
            foreach(QLabel *label,labellist){
                if(label->text().toLower() == appname_test.toLower() || (label->text() == zh_nameapp && zh_nameapp!="")){
                    QList <QProgressBar*> qprolist = widget2->findChildren<QProgressBar*>();
                    foreach(QProgressBar *progress,qprolist)
                        progress->setValue(status);
                    foreach(QLabel *label1,labellist){
                        if (label1->objectName() == "progress")
                            label1->setText(QString::number(status) + "%");
                    }
                }


            }
        }
    }
    connect(m_pLingmoInstalldbusCallThread, &LingmoInstalldbusCallThread::lingmobatchOsinstalldingStatus, \
            this, &kreInstall::batchInstalldingStatus);

}

/*单个安装进度返回显示*/
void kreInstall::InstalldingStatus(int status)
{
    disconnect(m_pLingmoInstalldbusCallThread, &LingmoInstalldbusCallThread::lingmoOsinstalldingStatus, \
               this, &kreInstall::InstalldingStatus);
    pProgressBar->setValue(status);
    progress->setText(QString::number(status) + "%");

    connect(m_pLingmoInstalldbusCallThread, &LingmoInstalldbusCallThread::lingmoOsinstalldingStatus, \
            this, &kreInstall::InstalldingStatus);

}


void kreInstall::seniorBtnClicked()
{
    m_ppermissionsWidget->show();
    m_pblackWidget->hide();
}

void kreInstall::generalBtnClicked()
{
    m_ppermissionsWidget->hide();
    m_pblackWidget->show();
}

QString kreInstall::getDebInfoFromDeb(QString m_debPath, QString type)
{

    QStorageInfo SD;
    SD.setPath("/");
    SD.refresh();
    int disk_space = SD.bytesAvailable()/1024/1024;
    if (disk_space <=10)
    {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText(tr("Insufficient disk space!"));//磁盘空间不足！！！！
        QPushButton *btn = new QPushButton();
        btn->setText(tr("OK"));
        msgBox.addButton(btn, QMessageBox::AcceptRole);
        msgBox.exec();
        msgBox.setContextMenuPolicy(Qt::NoContextMenu);
        exit(0);
    }
    QProcess terminal;
    QStringList cmdList;
    m_debPath = QDir::toNativeSeparators(m_debPath);


    QApt::DebFile debfile(m_debPath);
    if (debfile.isValid()){
        if (type == "Package"){
            return debfile.packageName();}
        else if (type == "Version"){
            return debfile.version();
        }
        else{
            return debfile.architecture();
        }
    }
    else{
        return "The software package is damaged";
    }


    QRegExp rx("[^a-zA-Z0-9]");
    int pos = rx.indexIn(m_debPath);
    if (pos > -1)
        m_debPath.toHtmlEscaped();

    QFileInfo file(m_debPath);

    cmdList << "-c" << QString("dpkg -I " + m_debPath);
    terminal.start("/bin/bash",cmdList);
    terminal.waitForFinished();

    QString debInfo = terminal.readAll();

    QString debinfoerro = terminal.readAllStandardError().data();
    if ((debInfo.contains(type) || debInfo.contains(type.toLower())) && debinfoerro=="" ){
        QString ret;
        QStringList list = debInfo.split("\n");
        foreach (QString str, list) {
            if(type == "Package"){
                if (str.left(9) == " Package:"){
                    ret = str.remove(" Package:").remove("\n").remove(" ");
                    break;
                }
            } else if(type == "Version"){
                if (str.left(9) == " Version:"){
                    ret = str.remove(" Version:").remove("\n").remove(" ");
                    break;
                }
            } else if(type == "Architecture"){
                if (str.left(14) == " Architecture:"){
                    ret = str.remove(" Architecture:").remove("\n").remove(" ");
                    break;
                }
            }
        }
        if ( debInfo!="" && debInfo.contains(type)){
            qInfo() << ret << "=======================";
           return ret;
        }
    }
    else if (debInfo!="" && debinfoerro.contains("压缩数据已损坏")|| debinfoerro.contains("གནོན་སྐྱུང་གཞི་གྲངས་ལ་སྐྱོན་ཤོར་བ།"))
    {
        //deb包已损坏，无法解析
        return "The software package is damaged";
    }
    else if (file.exists() && debInfo=="")
    {
        //路径包含特殊字符，需要debfile解析
        //QApt::DebFile::isValid();
        QApt::DebFile debfile(m_debPath);
        if (debfile.isValid()){
            if (type == "Package"){
                return debfile.packageName();}
            else if (type == "Version"){
                return debfile.version();
            }
            else{
                return debfile.architecture();
            }
        }
        else{
            return "The software package is damaged";
        }
    }



}


/***********获取md5值********/
QString kreInstall::getDebInfoFromDeb_md5(QString m_debPath)
{
    QProcess terminal;
    QStringList cmdList;
    cmdList << "-c" << QString("md5sum " + m_debPath).toStdString().c_str();
    terminal.start("/bin/bash",cmdList);
    terminal.waitForFinished();

    QString md5sum = terminal.readAll();
    QStringList list = md5sum.split(" ");
    if (list.count() >0)
        return list[0].simplified();
    else
        return NULL;
}

bool kreInstall::parseRuntimeXml(const QString &fileName)
{
    QDomDocument doc;
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QFile::Text)) {
        return false;
    }

    if (!doc.setContent(&file)) {
        file.close();
        return false;
    }

    file.close();
    QDomElement docElem = doc.documentElement();
    QDomNode docNode = docElem.firstChild();

    while (!docNode.isNull()) {
        if (docNode.nodeName() != "runtime") {
            docNode = docNode.nextSibling();
        }
        if (docNode.isElement()) {
            QDomElement ele = docNode.toElement();
            QDomNodeList list = ele.childNodes();
            m_pruntimeLabel->setText(ele.attribute("name"));
            m_lingmoRuntimeEnvList.append(ele.attribute("name"));
        }
        docNode = docNode.nextSibling();
    }
    return true;
}

/* 通过polkit提权 */
bool kreInstall::polkit()
{
    PolkitQt1::Authority::Result result;
    result = PolkitQt1::Authority::instance()->checkAuthorizationSync(
                "org.lingmo.installer.action",
                PolkitQt1::UnixProcessSubject(QCoreApplication::applicationPid()),
                PolkitQt1::Authority::AllowUserInteraction);
    if (result == PolkitQt1::Authority::Yes) { //认证通过
        return true;
    } else {
        return false;
    }
}

void kreInstall::installBtnClicked()
{
    QVariantList jsonInfo;
    m_runtime  = m_pruntimeLabel->text();


    jsonInfo<<QVariant::fromValue(m_runtime);
    jsonInfo<<QVariant::fromValue(m_appName);
    jsonInfo<<QVariant::fromValue(m_permissionList);
    if (!m_runtime.isEmpty()) {
        if (!polkit()) {
            return ;
        } else {
            ;
        }

        m_pDbusCall = new DbusCallThread(jsonInfo, m_runtime, m_appName, m_debPath);
        m_pDbusCall->start();
        connect(m_pDbusCall, SIGNAL(installFail()), this, SLOT(installFailSlot()), Qt::QueuedConnection);
        connect(m_pDbusCall, SIGNAL(installSuccess()), this, SLOT(installSuccessSlot()), Qt::QueuedConnection);

    } else {
        exit(0);
    }
}

/*查看日志函数*/

void kreInstall::showinstallfaillogtext()
{
    return_homepage_log1->show();
    m_pAllStackedWidget->setCurrentIndex(6);
}

/*返回主页*****/

void kreInstall::return_homepage()
{
    debPath_all1->clear();
    debPath_all2->clear();
    m_pInstallLogtextedit->clear();
    maplist.clear();
    int count = batch_listwidget->count();
    Batch_addinstall->show();
    Batch_return->hide();
    Batch_install->show();
    Batch_viewlog->hide();
    return_homepage_log1->hide();
    Batch_install->setEnabled(true);
    appinstallstatus = true;
    for(int i = 0; i< count;i++){
        QListWidgetItem *item1 = batch_listwidget->takeItem(0);
        batch_listwidget->removeItemWidget(item1);
        delete item1;
    }
    m_pAllStackedWidget->setCurrentIndex(4);

}
// 灵墨安装器单个安装完成信号槽
void kreInstall::osInstallOverSoftwareAptSlots(bool status, QString msg)
{

    kreInStallUnlock();
    disconnect(m_pLingmoInstalldbusCallThread, &LingmoInstalldbusCallThread::lingmoOsinstallStatus, \
               this, &kreInstall::osInstallOverSoftwareAptSlots);
    if(mime.name().contains("vnd.snap",Qt::CaseSensitive)){
        if (status == true) {
            kdk::KSoundEffects::playSound(SoundType::COMPLETE);
            m_pAllStackedWidget->setCurrentIndex(2);

        } else {
            m_pInstallLogtextedit->setText("\n"+msg);
            m_pAllStackedWidget->setCurrentIndex(3);
            if(msg.contains("架构不符",Qt::CaseSensitive))
                m_pAllStackedWidget->setCurrentIndex(3);
        }
    }
    else if(mime.name().contains("vnd.android.package-archive",Qt::CaseSensitive)){
        if (status == true) {
            kdk::KSoundEffects::playSound(SoundType::COMPLETE);
            m_pAllStackedWidget->setCurrentIndex(2);

        } else {
            m_pInstallLogtextedit->setText("\n"+msg);
            m_pAllStackedWidget->setCurrentIndex(3);
        }
    }
    else{
        if (status) {
            pProgressBar->setValue(1);
            progress->setText("1%");
            kdk::KSoundEffects::playSound(SoundType::COMPLETE);
            m_pAllStackedWidget->setCurrentIndex(2);

        } else {
            pProgressBar->setValue(1);
            progress->setText("1%");
            m_pInstallLogtextedit->setText("\n"+msg);
            m_pAllStackedWidget->setCurrentIndex(3);
        }
    }
    install_status =false;
    return;
}
/***********批量安装完成信号槽函数********/
void kreInstall::Batch_osInstallOverSoftwareAptSlots(bool status, QString msg){


    icontime->stop();
    QString deskstr = debPath_all1->at(0);
    disconnect(m_pLingmoInstalldbusCallThread, &LingmoInstalldbusCallThread::lingmoOsinstallStatus, \
               this, &kreInstall::Batch_osInstallOverSoftwareAptSlots);
    if (status && getAppStatus()) {
        changebuttonstatus(deskstr,0);
        if (debPath_all1->length()>0){
            flagstring = debPath_all1->at(0);
            thread_install_deb();
            automatic_deletewidgetitem();
        }
        else{
            kreInStallUnlock();
        }
    } else {
        QString appname_test = getDebInfoFromDeb(debPath_all1->at(0), "Package");
        QString zh_nameapp =  m_pdebWidget->getAppCnName(appname_test);
        QString info = "";
        if(zh_nameapp.isEmpty())
            info = appname_test + ":";
        else
            info = zh_nameapp + ":";
        m_pInstallLogtextedit->insertPlainText("\n"+info+"\n"+msg);
        appinstallstatus = false;
        changebuttonstatus(debPath_all1->at(0),1);
        if (debPath_all1->length()>0)
            thread_install_deb();
        else
            kreInStallUnlock();
    }
    if (debPath_all1->size() == 0 && batch_listwidget->count()>0 && appinstallstatus ==false){
        Batch_install->hide();
        Batch_viewlog->show();
        Batch_addinstall->hide();
        Batch_return->show();
    }
    return;

}
/*********线程批量安装*********/
void kreInstall::thread_install_deb(){
    debPath_all1->removeFirst();
    if (debPath_all1->size()>0){
        app_installed_fast();
        if(debPath_all1->size()>0){
            change_installapp =  debPath_all1->at(0);
            delete m_pLingmoInstalldbusCallThread;
            m_pLingmoInstalldbusCallThread = nullptr;
            m_pLingmoInstalldbusCallThread = new LingmoInstalldbusCallThread(debPath_all1->at(0),m_appName,m_application,m_applicationZh,m_appVersion,kare->kareImage);
            m_pLingmoInstalldbusCallThread->start();
            changebuttonstatus(debPath_all1->at(0),2);

            connect(m_pLingmoInstalldbusCallThread, &LingmoInstalldbusCallThread::lingmoOsinstallStatus, \
                    this, &kreInstall::Batch_osInstallOverSoftwareAptSlots);

            connect(m_pLingmoInstalldbusCallThread, &LingmoInstalldbusCallThread::lingmobatchOsinstalldingStatus, \
                    this, &kreInstall::batchInstalldingStatus);


        }
    }
}

/*********确定相同版本安装的后续操作函数*******/
void kreInstall::app_installed_fast(){
    bool app_version_status;
    int lsitcount;
    lsitcount = debPath_all1->size();
    install_flag = 0;
    QString appname_test = getDebInfoFromDeb(debPath_all1->at(0), "Package");
    QString zh_nameapp =  m_pdebWidget->getAppCnName(appname_test);
    for(int i = 0; i< lsitcount && debPath_all1->size()>0;i++){
        QString appname_test = getDebInfoFromDeb(debPath_all1->at(0), "Package");
        QString zh_nameapp =  m_pdebWidget->getAppCnName(appname_test);
        app_version_status=app_installed(debPath_all1->at(0));
        if (!app_version_status){
            QMessageBox msgBox;
            msgBox.setWindowFlags(Qt::Drawer);
            if (zh_nameapp != nullptr){
                QString lang = getenv("LANG");
                if (lang.contains("zh_CN")){
                    QString info = tr("The current version of the software is already installed on the system");
                    msgBox.setText('\"'+zh_nameapp+'\"'+" "+info);
                }
                else{
                    QString info = tr("The current version of the software is already installed on the system");
                    msgBox.setText('\"'+appname_test+'\"'+" "+info);
                }

            }
            else{
                QString info = tr("The current version of the software is already installed on the system");
                msgBox.setText('\"'+appname_test+'\"'+" "+info);
            }
            CACLINFO = false;
            QPushButton *btnno = new QPushButton();
            btnno->setText(tr("Cancel installation"));
            msgBox.addButton(btnno, QMessageBox::ActionRole);
            QPushButton *btnyes = new QPushButton();
            btnyes->setText(tr("continue installation"));
            msgBox.addButton(btnyes, QMessageBox::RejectRole);
            //            QFont font2 = btnno->font();
            //            font2.setPixelSize(getSystemFont());
            //            btnno->setFont(font2);
            //            btnyes->setFont(font2);
            //            msgBox.setFont(font2);
            connect(btnno,SIGNAL(clicked(bool)),this,SLOT(cancel_installed()));
            connect(btnyes,SIGNAL(clicked(bool)),this,SLOT(continue_install()));
            msgBox.exec();
            if(install_flag ==1)
                break;
            else
                if (CACLINFO == false)
                    cancel_installed();
        }

    }
    for(int i = 0; i< batch_listwidget->count();++i){
        QListWidgetItem *item1 = batch_listwidget->item(i);
        QWidget * widget2 = batch_listwidget->itemWidget(item1);
        QList <QLabel*> labellist = widget2->findChildren<QLabel*>();
        if(widget2 !=NULL){
            foreach(QLabel *label,labellist){
                if(label->text().toLower() == appname_test.toLower() || (label->text() == zh_nameapp && zh_nameapp!="")){
                    foreach(QLabel *label1,labellist){
                        if (label1->text() == "Wait"){
                            label1->show();

                        }
                    }
                }
                else{
                    if (label->text() == "Wait")
                        label->show();
                }

            }

        }
    }
}

/*******判断软件的当前版本是否已安装*******/
bool kreInstall::app_installed(QString debpathstring){

    QString current_version = getDebInfoFromDeb(debpathstring, "Version");
    QString installed_version;
    QString current_version_debPath;
    QString current_name = getDebInfoFromDeb(debpathstring, "Package");

    QString Architecture = getDebInfoFromDeb(debpathstring, "Architecture");
    QProcess terminal;
    QStringList cmdList;
    QProcess terminal1;
    QStringList cmdList1;
    QString installed_version_status;
    current_name = current_name.toLower();
    current_version_debPath = current_name;
    current_version_debPath.push_front(' ');
    current_version_debPath.push_front('"');
    current_version_debPath.push_back(' ');
    current_version_debPath.push_back(' ');
    current_version_debPath.push_back('"');

    cmdList << "-c" << QString("dpkg -l | grep "+ current_version_debPath + "| awk '{print $3}' | sed -n '1p'").toStdString().c_str();
    terminal.start("/bin/bash",cmdList);
    terminal.waitForFinished();
    installed_version = terminal.readAll();//获取此包已安装的版本号
    if (installed_version.isEmpty())
    {
        cmdList.clear();
        current_version_debPath = "";
        current_version_debPath = current_name;
        current_version_debPath.push_front(' ');
        current_version_debPath.push_front('"');
        current_version_debPath.push_back(':');
        current_version_debPath.push_back(Architecture);
        current_version_debPath.push_back('"');
        cmdList << "-c" << QString("dpkg -l | grep "+ current_version_debPath + "| awk '{print $3}' | sed -n '1p'").toStdString().c_str();
        terminal.start("/bin/bash",cmdList);
        terminal.waitForFinished();
        installed_version = terminal.readAll();//获取此包已安装的状态
    }

    cmdList1 << "-c" << QString("dpkg -l | grep "+ current_version_debPath + "| awk '{print $1}' | sed -n '1p'").toStdString().c_str();
    terminal1.start("/bin/bash",cmdList1);
    terminal1.waitForFinished();
    installed_version_status = terminal1.readAll();//获取此包已安装的状态
    if (installed_version_status.isEmpty())
    {
        cmdList1.clear();
        current_version_debPath = "";
        current_version_debPath = current_name;
        current_version_debPath.push_front(' ');
        current_version_debPath.push_front('"');
        current_version_debPath.push_back(':');
        current_version_debPath.push_back(Architecture);
        current_version_debPath.push_back('"');
        cmdList1 << "-c" << QString("dpkg -l | grep "+ current_version_debPath + "| awk '{print $1}' | sed -n '1p'").toStdString().c_str();
        terminal1.start("/bin/bash",cmdList1);
        terminal1.waitForFinished();
        installed_version_status = terminal1.readAll();//获取此包已安装的状态
    }

    installed_version_status = installed_version_status.left(installed_version_status.length() - 1);

    installed_version = installed_version.left(installed_version.length() - 1);


    if (installed_version == current_version && installed_version_status.contains("ii"))
        return false;
    else
        return true;
}

/********批量安装取消继续安装当前软件*****/
void kreInstall::cancel_installed(){
    CACLINFO = true;
    if(FilePath == false){
        QString appname_test = getDebInfoFromDeb(debPath_all1->at(0), "Package");
        QString zh_nameapp =  m_pdebWidget->getAppCnName(appname_test);
        for(int i = 0; i< batch_listwidget->count();i++){
            QListWidgetItem *item1 = batch_listwidget->item(i);
            QWidget * widget2 = batch_listwidget->itemWidget(item1);
            if(widget2 !=NULL){
                QList <QLabel*> labellist = widget2->findChildren<QLabel*>();
                foreach(QLabel *label,labellist){
                    if(label->text().toLower() == appname_test.toLower() || (label->text() == zh_nameapp && zh_nameapp!="")){
                        QListWidgetItem *item3 = batch_listwidget->takeItem(i);
                        batch_listwidget->removeItemWidget(item1);
                        delete item3;
                        break;
                    }

                }

            }
        }
        QString md5sum;
        md5sum = getDebInfoFromDeb_md5(debPath_all1->at(0));
        for(int i = 0; i< debPath_all2->count();i++){
            if (debPath_all2->at(i) == md5sum){
                debPath_all2->removeAt(i);
            }
        }
        debPath_all1->removeFirst();
        if (batch_listwidget->count() == 0)
            return_homepage();
    }
    else
    {
        CANSCELISNTALL = true;
    }
}

/*********选择确定的时候******/
void kreInstall::continue_install(){
    install_flag = 1;
    CANSCELISNTALL = false;
}

/*****添加包名相同，版本不同的包时判断*****/
void kreInstall::canseaddapplication(){
    canceladd = 1;
}

/******替换文件函数*******/
void kreInstall::replace_application(){
    QString appname_test = getDebInfoFromDeb(path_deb, "Package");
    QString zh_nameapp =  m_pdebWidget->getAppCnName(appname_test);
    for(int i = 0; i< batch_listwidget->count();i++){
        QListWidgetItem *item1 = batch_listwidget->item(i);
        QWidget * widget2 = batch_listwidget->itemWidget(item1);
        if(widget2 !=NULL){
            QList <QLabel*> labellist = widget2->findChildren<QLabel*>();
            foreach(QLabel *label,labellist){
                if(label->text().toLower() == appname_test.toLower() || (label->text() == zh_nameapp && zh_nameapp!="")){
                    QListWidgetItem *item3 = batch_listwidget->takeItem(i);
                    batch_listwidget->removeItemWidget(item1);
                    delete item3;
                    debPath_all1->removeAt(i);
                    break;
                }

            }

        }
    }

}



bool kreInstall::getAppStatus()
{
    QString shell = "dpkg --get-selections " + m_appName;
    QProcess *pro =  new QProcess();
    pro->start(shell);
    pro->waitForReadyRead(-1);
    QString str = pro->readAllStandardOutput();
    pro->waitForFinished(-1);
    if (str.contains(m_appName)){
        if (str.remove(m_appName).contains("install"))
            return true;
        else
            return false;
    }
    return false;
}
void kreInstall::secondaryInstallation(QString debPath)
{

    QFileInfo fileinfo(debPath);
    if (!fileinfo.isFile()) {
        qDebug() << debPath << "is not file";
    }
    QString dst_debPath = fileinfo.absoluteFilePath();


    if (dst_debPath == "/usr/bin/lingmo-installer")
        return;


    if (m_pAllStackedWidget->currentIndex() == 1 || m_pAllStackedWidget->currentIndex() == 5) {
        this->hide();
        kdk::LingmoUIStyleHelper::self()->removeHeader(this);
        this->show();
        this->activateWindow();
    }
    else if (m_pAllStackedWidget->currentIndex() == 4 && FilePath == false ||  m_pAllStackedWidget->currentIndex() == 7){
        QStringList debpathlist;
        QMimeDatabase mimeDatabase;
        QMimeType mimeType = mimeDatabase.mimeTypeForFile(debPath);
        //安卓软件包的mimetype
        if (mimeType.name() == "application/vnd.debian.binary-package"){
            m_pAllStackedWidget->setCurrentIndex(7);
            debpathlist.append(debPath);
            batch_installWidgeted(debpathlist);
        }
        else{
            QString  errroinfo = tr("Batch installation does not support apk package installation!");//批量安装不支持apk包安装

            QScopedPointer<QWidget> widget(new widbox(errroinfo));
            widget->show();
            widget ->raise();
        }
        //  qWarning() << "批量安装无法安装apk包";

    }
    else {
        isFile();                    // 判断文件是否存在
        SECENTSTART = true;//  判断是否是第二次启动
        initDebInfo(dst_debPath);   // 初始化deb信号，获取deb信息
        if (EXITQMESSG == true)
            return;
        resetDebInfo();         // 初始化界面数据
        m_pAllStackedWidget->setCurrentIndex(4);

        //  getOsVersionValue();    // 根据系统下的lsb-release文件，来判别当前系统版本
        //     initPermissionsList(debPath);
        //     getControlfield();      // 根据control获取是兼容环境的包还是系统环境的包
        this->hide();
        kdk::LingmoUIStyleHelper::self()->removeHeader(this);
        this->show();
        this->activateWindow();
    }
    return;
}


void kreInstall::resetDebInfo()
{
    // m_installed_tips->setVisible(false);
    m_pOsInstallButton->setText(QObject::tr("one-click install"));

    /*重设安装到系统的界面数据*/
    m_pOsDebMainWidget->resetAppInfo(m_appName, m_appVersion);
    m_pOsDebInstallIngWidget->resetAppInfo(m_appName, m_appVersion);
    /*重设安装到兼容环境的界面数据*/
    m_pdebWidget->resetAppInfo(m_appName, m_appVersion);
    m_pdebInstallIngWidget->resetAppInfo(m_appName, m_appVersion);
}

void kreInstall::getKreAppDesktopFileName()
{
    QDomDocument doc;
    QFile file(XML_APPLICATIONS_PATH);
    if (!file.open(QIODevice::ReadOnly | QFile::Text)) {
        printf("Error : open xml file fail\n");
        return;
    }

    if (!doc.setContent(&file)) {
        printf("Error : load xml file fail\n");
        return;
    }
    file.close();

    QDomElement root = doc.documentElement();
    QDomNode first = root.firstChild();
    QString desktopString;
    desktopString.clear();
    while (!first.isNull()) {
        if (first.nodeName() == "appPkgName") {
            QString pkg_name = first.toElement().attribute("name");
            if (pkg_name == m_appName) {
                QDomNode second = first.firstChild();
                while (!second.isNull()) {
                    desktopString = second.toElement().attribute("desktop");
                    second = second.nextSibling();
                    if (desktopString == nullptr) {
                        continue;
                    }
                    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
                    QString fullFilePath = desktopPath + "/" + desktopString;
                    if (fullFilePath == desktopPath + "/") {
                        continue;
                    }
                    QFileInfo fileInfo(fullFilePath);
                    if (fileInfo.exists()) {
                        QString exec = "rm " + fullFilePath;
                        qDebug() << "desktop文件路径，需要删除的文件" << exec;
                        QProcess p(0);
                        p.startDetached(exec);
                        p.waitForStarted();
                    }
                }
            }
        }
        first = first.nextSibling();
    }

    return;
}

// 灵墨安装器一键安装按钮槽函数
void kreInstall::OsInstallButtonSlots()
{
    bool app_version_status;
    QString appname_test = getDebInfoFromDeb(m_debPath, "Package");
    QString zh_nameapp =  m_pdebWidget->getAppCnName(appname_test);
    app_version_status=app_installed(m_debPath);
    if (!app_version_status){
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("install error!"));
        msgBox.setIcon(QMessageBox::Critical);
        if (zh_nameapp != nullptr){
            QString lang = getenv("LANG");
            if (lang.contains("zh_CN")){
                QString info = tr("The current version of the software is already installed on the system");
                msgBox.setText('\"'+zh_nameapp+'\"'+" "+info);
            }
            else{
                QString info = tr("The current version of the software is already installed on the system");
                msgBox.setText('\"'+appname_test+'\"'+" "+info);
            }

        }
        else{
            QString info = tr("The current version of the software is already installed on the system");
            msgBox.setText('\"'+appname_test+'\"'+" "+info);
        }
        CACLINFO = false;
        CANSCELISNTALL =false;
        install_flag = 0;
        QPushButton *btnno = new QPushButton();
        btnno->setText(tr("Cancel installation"));
        msgBox.addButton(btnno, QMessageBox::ActionRole);
        QPushButton *btnyes = new QPushButton();
        btnyes->setText(tr("continue installation"));
        msgBox.addButton(btnyes, QMessageBox::RejectRole);

        //        QFont font2 = btnno->font();
        //        font2.setPixelSize(getSystemFont());
        //        btnno->setFont(font2);
        //        btnyes->setFont(font2);
        //        msgBox.setFont(font2);
        connect(btnno,SIGNAL(clicked(bool)),this,SLOT(cancel_installed()));
        connect(btnyes,SIGNAL(clicked(bool)),this,SLOT(continue_install()));
        msgBox.exec();
        if (CACLINFO == false && install_flag == 0)
            cancel_installed();
    }
    if (CANSCELISNTALL == true){
        kreInStallUnlock();
        CANSCELISNTALL=false;
        return;
    }
    else{
        install_status = true;
        m_pLingmoInstalldbusCallThread = new LingmoInstalldbusCallThread(m_debPath,m_appName,m_application,m_applicationZh,m_appVersion,kare->kareImage);
        m_pLingmoInstalldbusCallThread->start();


        connect(m_pLingmoInstalldbusCallThread, &LingmoInstalldbusCallThread::lingmoOsinstallStatus, \
                this, &kreInstall::osInstallOverSoftwareAptSlots);


        connect(m_pLingmoInstalldbusCallThread, &LingmoInstalldbusCallThread::lingmoOsinstalldingStatus, \
                this, &kreInstall::InstalldingStatus);
        m_pAllStackedWidget->setCurrentIndex(5);
    }
}

/*判断dpkg是否正在运行*/
int kreInstall::installerdpkg()
{
    QProcess terminal_1;
    QStringList cmdList;
    int   deskt;
    cmdList << "-c" << QString("ps -C \"dpkg -i\" |wc -l");
    terminal_1.start("/bin/bash",cmdList);
    terminal_1.waitForFinished();
    QByteArray result = terminal_1.readAll();
    deskt= result.toInt();
    qDebug() << "dpkg 运行数量" << deskt;
    return deskt;

}
/* 兼容环境安装失败 */
void kreInstall::installFailSlot()
{
    m_pAllStackedWidget->setCurrentIndex(3);
}

/* 兼容环境安装成功 */
void kreInstall::installSuccessSlot()
{
    m_pAllStackedWidget->setCurrentIndex(2);
    getKreAppDesktopFileName();
}

void kreInstall::minBtnClicked()
{
    this->showMinimized();
}

void kreInstall::closeBtnClicked(bool status)
{
    kreInStallUnlock();
    this->close();
    exit(0);
}

void kreInstall::batchlist()
{
    if(FilePath == false)
        m_pAllStackedWidget->setCurrentIndex(7);
    else
        m_pAllStackedWidget->setCurrentIndex(3);

}

/*
 * 上锁
*/
bool kreInstall::kreInStallLock()
{
    //获取用户名与用户ID
    int uid = getuid();
    struct passwd *pwd = getpwuid(uid);
    QString nameuid = QString("%1(%2)\n").arg(pwd->pw_name).arg(uid);
    char*  charnameuid;
    QByteArray ba = nameuid.toLatin1(); // must
    charnameuid=ba.data();

    //初始化运行程序名称
    char name_string[25] = {};
    strcpy(name_string , PROGRAM_NAME);
    QDir dir("/tmp/lock/");
    if (!dir.exists()) {
        dir.mkdir("/tmp/lock/");//只创建一级子目录，即必须保证上级目录存在
        chmod("/tmp/lock/", 0777);
    }
    QString lockPath = LOCK_FILE_PATH;
    umask(0000);
    gi_fd = open(lockPath.toUtf8().data(), O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (gi_fd < 0) {
        qDebug()<<"文件锁打开异常";
    } else {
        write(gi_fd, QString::number(uid).toStdString().c_str(), strlen(QString::number(uid).toStdString().c_str()));
        write(gi_fd, "\n", strlen("\n"));
        write(gi_fd, "lingmo-installer\n", strlen("lingmo-installer\n"));
        write(gi_fd, "install\n", strlen("install\n"));
        write(gi_fd, QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz").toStdString().c_str(), strlen(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz").toStdString().c_str()));
        write(gi_fd, "\n", strlen("\n"));
    }
    return flock(gi_fd, LOCK_EX | LOCK_NB);
}
/*
 * 解锁
*/
void kreInstall::kreInStallUnlock()
{

    /*  QString lockPath = LOCK_FILE_PATH;
    umask(0000);
    int fd = open(lockPath.toUtf8().data(), O_RDONLY, 0666);
    if (fd < 0) {
        qDebug()<<"解锁时文件锁打开异常";
    } else {
        ftruncate(fd, 0);
    }*/

    int ret = flock(gi_fd, LOCK_UN);
    qDebug() << "解锁flock 返回值======" << ret;
}

void kreInstall::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    QStyleOption option;
    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    QPainterPath rectPath;
    rectPath.addRoundedRect(this->rect(), 0, 0); // 左上右下
    p.setOpacity(1);
    p.save();
    p.fillPath(rectPath, option.palette.color(QPalette::Base));
    p.restore();
}


/**************************************安卓函数***************************/

bool kreInstall::isAndroidReady(const QString &username, const QString &uid)
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

/********getusername******/
QString kreInstall::getUserName()
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
QString kreInstall::getUid()
{
    uid_t uid = getuid();
    return QString::number(uid);
}

int kreInstall::isAndroidEnvInstalled()
{
    int nRes = -1;
    QString libPath = "/usr/lib/libkmre.so";
    if (!QFile::exists(libPath)) {
        return nRes;
    }

    void *module_handle;
    char *module_error;
    module_handle = dlopen(libPath.toStdString().c_str(), RTLD_LAZY);
    if (!module_handle) {
        return nRes;
    }

    bool (*is_android_env_installed)();
    is_android_env_installed = (bool(*)())dlsym(module_handle, "is_android_env_installed");
    if ((module_error = dlerror()) != NULL) {
        dlclose(module_handle);
        return nRes;
    }
    bool ret = is_android_env_installed();
    dlclose(module_handle);

    if (ret) {
        nRes = 1;
    }
    else {
        nRes = 0;
    }

    return nRes;
}



/********初始化安卓apk包信息*****/
void kreInstall::onAnalysisApkFile(QString debPath)
{


    QString m_loginUserName = getUserName();
    QString m_loginUserId = getUid();



    int value = isAndroidEnvInstalled();
    if (value == 0) {
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("open file error"));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText(tr("Lingmo mobile operating environment is not installed!!!"));//灵墨移动运行环境未安装！！！
        QPushButton *btn = new QPushButton();
        btn->setText(tr("OK"));
        msgBox.addButton(btn, QMessageBox::AcceptRole);
        msgBox.exec();
        msgBox.setContextMenuPolicy(Qt::NoContextMenu);
        if (SECENTSTART ==  false){
            exit(0);

        }
        else
        {
            EXITQMESSG = true;
        }
    }

    bool androidReady = isAndroidReady(m_loginUserName, m_loginUserId);
    if (!androidReady) {
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("open file error"));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText(tr("Lingmo mobile running environment has not started, unable to install APK files!!!"));//灵墨移动运行环境未启动，无法安装apk文件！！！
        QPushButton *btn = new QPushButton();
        btn->setText(tr("OK"));
        msgBox.addButton(btn, QMessageBox::AcceptRole);
        msgBox.exec();
        msgBox.setContextMenuPolicy(Qt::NoContextMenu);
        if (SECENTSTART ==  false){
            exit(0);

        }
        else
        {
            EXITQMESSG = true;
        }
    }
    m_appName.clear();
    m_appVersion.clear();
    QFileInfo file_info(debPath);
    if (!file_info.exists()) {
        return;
    }

    qApp->processEvents();

    QString application_label = "";
    QString application_zh_label = "";
    QString fileSize = "";
    QString iconPath = "";
    QString version = "";

    QProcess process;
    QStringList cmdList;


    fileSize = QString("%1 KB (%2 MB)").arg(file_info.size()/1024).arg(file_info.size()/1048576);

    //s#ifndef LINGMO_V10
    if (QFileInfo("/usr/bin/aapt").exists()) {
        process.start("aapt", QStringList()<< "d" << "badging" << debPath);
        process.waitForStarted(5*1000);
        process.waitForFinished(10*1000);//process.waitForFinished(-1);
        const QString output = process.readAllStandardOutput();//process.readAll();

        for (const auto &line : output.split('\n')) {
            if (line.startsWith("package:")) {
                const QStringList &info = QString(line).split(' ');
                if (info.size() == 5) {
                    for (int i=0;i<info.size();i++) {
                        if (QString(info.at(i)).startsWith("name=")) {
                            const QStringList &nameinfo = QString(info.at(i)).split('=');
                            m_appName = QString(nameinfo.at(1)).replace("\'", "").trimmed();//com.rightware.BasemarkOSII
                        }
                        if (QString(info.at(i)).startsWith("versionName=")) {
                            const QStringList &versioninfo = QString(info.at(i)).split('=');
                            version = QString(versioninfo.at(1)).replace("\'", "").trimmed();//2.0
                            m_appVersion = version;

                        }
                    }
                }
            }
            if (QString(line).startsWith("application-label:")) {//可能不存在该行
                const QStringList &info = QString(line).split(':');
                if (info.size() == 2) {
                    application_label = QString(info.at(1)).replace("\'", "").trimmed();
                }
            }
            if (line.startsWith("application-label-zh-CN:")) {//可能不存在该行
                const QStringList &info = QString(line).split(':');
                if (info.size() == 2) {
                    application_zh_label = QString(info.at(1)).replace("\'", "").trimmed();
                }
            }
            if (line.startsWith("application-label-zh:")) {//可能不存在该行
                const QStringList &info = QString(line).split(':');
                if (info.size() == 2) {
                    if (application_zh_label.isEmpty()) {
                        application_zh_label = QString(info.at(1)).replace("\'", "").trimmed();
                    }
                }
            }
            if (line.startsWith("application: label=")) {
                int firstIndex = QString(line).indexOf(QChar('\''));
                int secondIndex = QString(line).indexOf(QChar('\''), strlen("application: label='"));
                QString labelContent = QString(line).left(secondIndex);
                QString labelStr = labelContent.right(labelContent.length() - 1 - firstIndex);
                if (application_label.isEmpty()) {
                    application_label = labelStr;
                }
                if (application_zh_label.isEmpty()) {
                    application_zh_label = labelStr;
                }

                const QStringList &info = QString(line).split(' ');
                for (int i=0;i<info.size();i++) {
                    if (QString(info.at(i)).startsWith("icon=")) {
                        //icon path
                        if (iconPath.isEmpty()) {
                            const QStringList &iconinfo = QString(info.at(i)).split('=');
                            iconPath = QString(iconinfo.at(1)).replace("\'", "").trimmed();
                        }
                    }
                }
            }
            if (QString(line).startsWith("launchable-activity:")) {//launchable-activity: name='com.rightware.BasemarkOSII.BasemarkOSII'  label='Basemark OS II' icon=''
                int firstIndex = QString(line).indexOf(QChar('\''));
                int secondIndex = QString(line).indexOf(QChar('\''), strlen("application: label='"));
                QString labelContent = QString(line).left(secondIndex);
                QString labelStr = labelContent.right(labelContent.length() - 1 - firstIndex);
                if (application_label.isEmpty()) {
                    application_label = labelStr;
                }
                if (application_zh_label.isEmpty()) {
                    application_zh_label = labelStr;
                }

                const QStringList &info = QString(line).split(' ');
                for (int i=0;i<info.size();i++) {
                    if (QString(info.at(i)).startsWith("icon=")) {
                        //icon path
                        if (iconPath.isEmpty()) {
                            const QStringList &iconinfo = QString(info.at(i)).split('=');
                            iconPath = QString(iconinfo.at(1)).replace("\'", "").trimmed();
                        }
                    }
                }
            }
            if (!application_label.isEmpty() && !(QString(m_appName).isEmpty())) {
                if (application_zh_label.isEmpty()) {
                    application_zh_label = application_label;
                }
                m_application = application_label;
                m_applicationZh = application_zh_label;
            }
        }
    }

}

/**********批量安装添加文件后界面显示**********/
void kreInstall::batch_installWidgeted(QStringList debPath){

    if (debPath_all1->count() == 0)
        Batch_install->setEnabled(true);
    m_pAllStackedWidget->setCurrentIndex(7);
    for(int i = 0; i< debPath.size();++i)
    {
        QString md5sum;
        QString tmp = debPath.at(i);
        QString appname_test = getDebInfoFromDeb(tmp, "Package");

        if(appname_test == "The software package is damaged"){
            deb_Damage_info();
            continue;
        }
        QString zh_nameapp =  m_pdebWidget->getAppCnName(appname_test);
        QString appversion_test = getDebInfoFromDeb(tmp, "Version");
        QString debArch_test = getDebInfoFromDeb(tmp,"Architecture");

        QStringList debpathnamelist = tmp.split("/");
        QString debpathname = debpathnamelist.at(debpathnamelist.size()-1);
        if (debpathname.contains("`",Qt::CaseSensitive)){
            QMessageBox msgBox;
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.setText(tr("Open file error\n"
                              "Naming contains nonstandard characters '`'"));
            QPushButton *btn = new QPushButton();
            btn->setText(tr("OK"));
            msgBox.addButton(btn, QMessageBox::AcceptRole);
            msgBox.exec();
            continue;
        }
        md5sum = getDebInfoFromDeb_md5(debPath.at(i));
        canceladd = 0;
        path_deb = tmp;
        if(ArchIsMatch(debArch_test,tmp) == false)
            continue;
        //  path_debint = i;
        for(QMap<QString, QString>::iterator it = maplist.begin();it != maplist.end();it++){
            if ((zh_nameapp == it.key()||appname_test == it.key())  && appversion_test != it.value()){
                QMessageBox msgBox;
                msgBox.setWindowFlags(Qt::Drawer);
                /*sgBox.setWindowTitle(tr("install error!"));
                msgBox.setIcon(QMessageBox::Critical);*/
                if (zh_nameapp != nullptr){
                    QString lang = getenv("LANG");
                    if (lang.contains("zh_CN")){
                        QString info1=tr("In the list ");
                        QString info2=tr(" already exists ");
                        QString info3=tr(" version, currently to be added ");
                        QString info4=tr(" of ");
                        QString info5=tr(" version, select replace?");
                        msgBox.setText(info1+zh_nameapp+info2+it.value()+info3+zh_nameapp+info4+appversion_test+info5);
                    }
                    else{
                        QString info1=tr("In the list ");
                        QString info2=tr(" already exists ");
                        QString info3=tr(" version, currently to be added ");
                        QString info4=tr(" of ");
                        QString info5=tr(" version, select replace?");
                        msgBox.setText(info1+appname_test+info2+it.value()+info3+appname_test+info4+appversion_test+info5);
                    }
                }
                else{
                    QString info1=tr("In the list ");
                    QString info2=tr(" already exists ");
                    QString info3=tr(" version, currently to be added ");
                    QString info4=tr(" of ");
                    QString info5=tr(" version, select replace?");
                    msgBox.setText(info1+appname_test+info2+it.value()+info3+appname_test+info4+appversion_test+info5);
                }
                QPushButton *btnno = new QPushButton();
                btnno->setText(tr("Cancel"));
                msgBox.addButton(btnno, QMessageBox::AcceptRole);
                QPushButton *btnyes = new QPushButton();
                btnyes->setText(tr("Replace"));
                connect(btnyes,SIGNAL(clicked(bool)),this,SLOT(replace_application()));
                connect(btnno,SIGNAL(clicked(bool)),this,SLOT(canseaddapplication()));
                msgBox.addButton(btnyes, QMessageBox::AcceptRole);
                msgBox.exec();
            }

        }
        if(canceladd == 1)
            continue;
        if (zh_nameapp != nullptr)
            maplist.insert(zh_nameapp,appversion_test);
        else
            maplist.insert(appname_test,appversion_test);
        if(debPath_all1->contains(tmp)){
            QApplication::setActiveWindow(this);
            this->setFocus();
            QMessageBox msgBox(QApplication::activeWindow());
            msgBox.setWindowFlags(Qt::Drawer);
            this->show();
            if (zh_nameapp != nullptr){
                QString lang = getenv("LANG");
                if (lang.contains("zh_CN")){
                    QString info = tr("The file already exists in the installation list");
                    msgBox.setText('\"'+zh_nameapp+'\"'+info);
                }
                else{
                    QString info = tr("The file already exists in the installation list");
                    msgBox.setText('\"'+appname_test+'\"'+info);
                }
            }
            else{
                QString info = tr("The file already exists in the installation list");
                msgBox.setText('\"'+appname_test+'\"'+info);
            }
            if (QIcon::fromTheme(appname_test.toLower()).isNull()) {
                QPixmap pixmap = QPixmap(batchwidget1->getIconPath(appname_test.toLower()));
                pixmap.scaled(QSize(48,48), Qt::KeepAspectRatio);
                if(batchwidget1->getIconPath(appname_test.toLower()) != nullptr){
                    msgBox.setIconPixmap(QIcon(QPixmap(batchwidget1->getIconPath(appname_test.toLower()))).pixmap(QSize(48,48)));
                }else{
                    msgBox.setIconPixmap(QIcon::fromTheme("application-x-desktop").pixmap(QSize(48,48)));
                }
            } else {
                msgBox.setIconPixmap(QIcon::fromTheme(appname_test.toLower()).pixmap(QSize(48,48)));
            }
            QPushButton *btn = new QPushButton();
            btn->setText(tr("OK"));
            msgBox.addButton(btn, QMessageBox::AcceptRole);
            msgBox.exec();
            continue;
        }
        else if(debPath_all2->contains(md5sum))
        {
            QApplication::setActiveWindow(this);
            this->setFocus();
            QMessageBox msgBox(QApplication::activeWindow());
            msgBox.setWindowFlags(Qt::Drawer);;
            if (zh_nameapp != nullptr){
                QString lang = getenv("LANG");
                if (lang.contains("zh_CN")){
                    QString info = tr("The file already exists in the installation list");
                    msgBox.setText('\"'+zh_nameapp+'\"'+info);
                }
                else{
                    QString info = tr("The file already exists in the installation list");
                    msgBox.setText('\"'+appname_test+'\"'+info);
                }
            }
            else{
                QString info = tr("The file already exists in the installation list");
                msgBox.setText('\"'+appname_test+'\"'+info);
            }
            if (QIcon::fromTheme(appname_test.toLower()).isNull()) {
                QPixmap pixmap = QPixmap(batchwidget1->getIconPath(appname_test.toLower()));
                pixmap.scaled(QSize(48,48), Qt::KeepAspectRatio);
                if(batchwidget1->getIconPath(appname_test.toLower()) != nullptr){
                    msgBox.setIconPixmap(QIcon(QPixmap(batchwidget1->getIconPath(appname_test.toLower()))).pixmap(QSize(48,48)));
                }else{
                    msgBox.setIconPixmap(QIcon::fromTheme("application-x-desktop").pixmap(QSize(48,48)));
                }
            } else {
                msgBox.setIconPixmap(QIcon::fromTheme(appname_test.toLower()).pixmap(QSize(48,48)));
            }
            QPushButton *btn = new QPushButton();
            btn->setText(tr("OK"));
            msgBox.addButton(btn, QMessageBox::AcceptRole);
            msgBox.exec();
            continue;
        }
        debPath_all2->append(md5sum);
        debPath_all1->append(tmp);
        adddebpathcompail(tmp);/******构建item*******/
    }
    if(debPath_all1->size() == 0)
        return_homepage();

}
/********文件添加函数*******/
void kreInstall::adddebpathcompail(QString tmp){

    batchwidget1 = new Batchinstall(tmp);

    QListWidgetItem *item = new QListWidgetItem();
    batchwidget1->setProperty("useIconHighlightEffect", 0x2);
    QSize size = item->sizeHint();
    item->setSizeHint(QSize(size.width(),70));
    item->setFlags(Qt::ItemIsEnabled);
    batch_listwidget->addItem(item);
    batchwidget1->setSizeIncrement(QSize(size.width(),85));
    batch_listwidget->setItemWidget(item,batchwidget1);
    batch_listwidget->setStyleSheet("QListWidget{background-color:rgba(0,0,0,0);border:0px solid #cfcfcf;}QListWidget::item{background-color:rgba(0,0,0,0);height:65px;margin-top:0px;border:0px solid #cfcfcf;}QListWidget::item:hover{background-color:background-color:rgba(0,0,0,0);}");

    connect(batchwidget1, &Batchinstall::deleteitemstring, \
            this, &kreInstall::deletewidgetitem);

}


/**************删除界面item***************/

void kreInstall::deletewidgetitem(QString itemwidget){
    if( batch_listwidget->count() == debPath_all1->size()){
        for(int i = 0; i< batch_listwidget->count();i++){
            if (itemwidget == debPath_all1->at(i)){
                QListWidgetItem *item1 = batch_listwidget->takeItem(i);
                batch_listwidget->removeItemWidget(item1);
                delete item1;
                debPath_all1->removeAt(i);
                break;
            }
        }

    }
    else{
        QString appname_test = getDebInfoFromDeb(itemwidget, "Package");
        QString zh_nameapp =  m_pdebWidget->getAppCnName(appname_test);
        for(int i = 0; i< batch_listwidget->count();++i){
            QListWidgetItem *item1 = batch_listwidget->item(i);
            QWidget * widget2 = batch_listwidget->itemWidget(item1);
            if(widget2 !=NULL){
                QList <QLabel*> labellist = widget2->findChildren<QLabel*>();
                foreach(QLabel *label,labellist){
                    if(label->text().toLower() == appname_test.toLower() || (label->text() == zh_nameapp && zh_nameapp!="")){
                        QListWidgetItem *item3 = batch_listwidget->takeItem(i);
                        batch_listwidget->removeItemWidget(item1);
                        delete item3;
                        ftime->stop();
                        break;
                    }

                }

            }
        }
        for(int i = 0; i< debPath_all1->size();i++){
            if (itemwidget == debPath_all1->at(i))
                debPath_all1->removeAt(i);
        }
    }
    QString md5sum;
    md5sum = getDebInfoFromDeb_md5(itemwidget);
    for(int i = 0; i< debPath_all2->count();i++){
        if (debPath_all2->at(i) == md5sum){
            debPath_all2->removeAt(i);
        }
    }
    if(debPath_all1->size() == 0){
        return_homepage();
    }



}

/*******循环播放安装中图片******/
void kreInstall::play_installingpicture(){
    QString teststr;
    for(int i = 0; i< batch_listwidget->count();++i){
        QString appname_test = getDebInfoFromDeb(change_installapp, "Package");
        QString zh_nameapp =  m_pdebWidget->getAppCnName(appname_test);
        QListWidgetItem *item1 = batch_listwidget->item(i);
        QWidget * widget2 = batch_listwidget->itemWidget(item1);
        if(widget2 !=NULL){
            QList <QLabel*> labellist = widget2->findChildren<QLabel*>();
            foreach(QLabel *label,labellist){
                if(label->text().toLower() == appname_test.toLower() || (label->text() == zh_nameapp && zh_nameapp!="")){
                    foreach(QLabel *label1,labellist){
                        if (label1->objectName() == "app_status"){
                            teststr=QString::number(settime,10);
                            label1->show();
                            label1->setProperty("useIconHighlightEffect", 0x2);
                            label1->setPixmap(QIcon::fromTheme("lingmo-loading-symbolic-"+teststr).pixmap(QSize(20,20)));
                            settime +=1;
                            if(settime == 7)
                                settime=0;
                            break;
                        }
                    }

                }
            }

        }

    }

}


/**********自动删除item*****/
void kreInstall::automatic_deletewidgetitem(){
    QString appname_test = getDebInfoFromDeb(flagstring, "Package");
    QString zh_nameapp =  m_pdebWidget->getAppCnName(appname_test);
    for(int i = 0; i< batch_listwidget->count();++i){
        QListWidgetItem *item1 = batch_listwidget->item(i);
        QWidget * widget2 = batch_listwidget->itemWidget(item1);
        if(widget2 !=NULL){
            QList <QLabel*> labellist = widget2->findChildren<QLabel*>();
            foreach(QLabel *label,labellist){
                if(label->text().toLower() == appname_test.toLower() || (label->text() == zh_nameapp && zh_nameapp!="")){
                    QListWidgetItem *item3 = batch_listwidget->takeItem(i);
                    batch_listwidget->removeItemWidget(item1);
                    delete item3;
                    break;
                }
            }
        }
    }
    // ftime->stop();
    QString md5sum;
    md5sum = getDebInfoFromDeb_md5(flagstring);
    for(int i = 0; i< debPath_all2->count();i++){
        if (debPath_all2->at(i) == md5sum){
            debPath_all2->removeAt(i);
        }
    }
    if (debPath_all1->size() == 0 && appinstallstatus == true){
        kdk::KSoundEffects::playSound(SoundType::COMPLETE);
        m_pAllStackedWidget->setCurrentIndex(8);
    }



}
//dex=0 安装成功，dex=1，安装失败，dex=2，安装中。
/**********************根据安装信号来修改itemwidget中的button状态********************/
void kreInstall::changebuttonstatus(QString itemwidget, int dex){

    QPalette pe = palette();
    pe.setColor(QPalette::Text, Qt::red);
    //   pe.setBrush(QPalette::Window,brush);
    for(int i = 0; i< batch_listwidget->count();++i){
        QString appname_test = getDebInfoFromDeb(itemwidget, "Package");
        QString zh_nameapp =  m_pdebWidget->getAppCnName(appname_test);
        QListWidgetItem *item1 = batch_listwidget->item(i);
        QWidget * widget2 = batch_listwidget->itemWidget(item1);
        if(widget2 !=NULL){
            if(dex ==0){
                QList <QLabel*> labellist = widget2->findChildren<QLabel*>();
                foreach(QLabel *label,labellist){
                    if(label->text().toLower() == appname_test.toLower() || (label->text() == zh_nameapp && zh_nameapp!="")){
                        QList <QPushButton*> buttonlist = widget2->findChildren<QPushButton*>();
                        foreach(QPushButton *button,buttonlist)
                            if(button->objectName() == "delete_app")
                                button->hide();
                        foreach(QLabel *label1,labellist){
                            if (label1->objectName() == "app_status"){
                                label1->show();
                                label1->setPixmap(QIcon::fromTheme("lingmo-dialog-success").pixmap(QSize(20,20)));
                            }
                            if (label1->objectName() == "progress")
                                label1->hide();
                            if (label1->objectName() == "appstatus")
                                label1->hide();
                            QList <QProgressBar*> qprolist = widget2->findChildren<QProgressBar*>();
                            foreach(QProgressBar *progress,qprolist){
                                progress->hide();
                                progress->setValue(0);
                            }
                        }

                    }
                }
                QList <QProgressBar*> qprolist = widget2->findChildren<QProgressBar*>();
                foreach(QProgressBar *progress,qprolist)
                    progress->hide();
            }
            else if (dex ==1){
                QList <QLabel*> labellist = widget2->findChildren<QLabel*>();
                foreach(QLabel *label,labellist){
                    if(label->text().toLower() == appname_test.toLower()|| (label->text() == zh_nameapp && zh_nameapp!="")){
                        QList <QToolButton*> buttonlist = widget2->findChildren<QToolButton*>();
                        foreach(QToolButton *button,buttonlist)
                            if(button->objectName() == "delete_app")
                                button->hide();
                        foreach(QLabel *label1,labellist){
                            if (label1->objectName() == "app_status"){
                                label1->show();
                                label1->setPixmap(QIcon::fromTheme("dialog-error").pixmap(QSize(20,20)));
                            }
                            if (label1->objectName() == "progress")
                                label1->hide();
                            if (label1->objectName() == "appstatus"){
                                label1->setText(tr("install Fail"));
                                pe.setBrush(QPalette::Active,QPalette::Text,QColor(255,0,0));
                                label1->setPalette(pe);
                                label1->show();
                                QList <QProgressBar*> qprolist = widget2->findChildren<QProgressBar*>();
                                foreach(QProgressBar *progress,qprolist){
                                    progress->hide();
                                    progress->setValue(0);
                                }
                            }

                        }
                        // break;
                    }
                }
                QList <QProgressBar*> qprolist = widget2->findChildren<QProgressBar*>();
                foreach(QProgressBar *progress,qprolist)
                    progress->hide();
                // break;

            }
            else if (dex ==2){
                QList <QLabel*> labellist = widget2->findChildren<QLabel*>();
                foreach(QLabel *label,labellist){
                    if(label->text().toLower() == appname_test.toLower() || (label->text() == zh_nameapp && zh_nameapp!="")){
                        QList <QPushButton*> buttonlist = widget2->findChildren<QPushButton*>();
                        foreach(QPushButton *button,buttonlist)
                            if(button->objectName() == "delete_app")
                                button->hide();
                        foreach(QLabel *label1,labellist){
                            if (label1->objectName() == "app_status"){
                                label1->hide();
                            }
                            if (label1->objectName() == "progress")
                            {
                                label1->setText("1%");
                                label1->show();
                            }
                            if (label1->objectName() == "appstatus")
                                label1->hide();

                        }
                        QList <QProgressBar*> qprolist = widget2->findChildren<QProgressBar*>();
                        foreach(QProgressBar *progress,qprolist)
                        {
                            progress->setValue(1);
                            progress->show();
                        }
                        break;
                    }
                }


            }
        }
    }

}
//安装成功图标：lingmo-dialog-success
//安装失败图标：dialog-erro

/*********批量安装槽函数********/
// 灵墨安装器一键安装按钮槽函数
void kreInstall::Batch_OsInstallButtonSlots()
{
    Batch_install->setEnabled(false);
    if (debPath_all1->size() == 0)
        return;
    app_installed_fast();
    if (debPath_all1->size() >0 ){
        change_installapp = debPath_all1->at(0);
        m_pLingmoInstalldbusCallThread = new LingmoInstalldbusCallThread(debPath_all1->at(0),m_appName,m_application,m_applicationZh,m_appVersion,kare->kareImage);
        m_pLingmoInstalldbusCallThread->start();
        changebuttonstatus(debPath_all1->at(0),2);


        connect(m_pLingmoInstalldbusCallThread, &LingmoInstalldbusCallThread::lingmoOsinstallStatus, \
                this, &kreInstall::Batch_osInstallOverSoftwareAptSlots);

        connect(m_pLingmoInstalldbusCallThread, &LingmoInstalldbusCallThread::lingmobatchOsinstalldingStatus, \
                this, &kreInstall::batchInstalldingStatus);
    }
    // m_pAllStackedWidget->setCurrentIndex(5);
}


bool kreInstall::isOsTree()
{
    QFile osTreeFile("/etc/ostree-release");
    QDir ostreeDir("/ostree");
    QDir sysRootDir("/sysroot");
    if (osTreeFile.exists() && sysRootDir.exists() && ostreeDir.exists()){
        return true;
    }
    return false;

}


bool kreInstall::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == this)
    {
        if(event->type() == QEvent::KeyPress){
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            if(keyEvent->key() == Qt::Key_F1){
                //显示用户手册
                kdk::kabase::UserManualManagement userManualTest;
                if (!userManualTest.callUserManual("lingmo-installer")) {
                    qCritical() << "user manual call fail!";
                }
            }

        }
    }
}



