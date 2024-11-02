#include <QDebug>
#include <QDomDocument>
#include <QFile>
#include <QStyleOption>
#include <polkit-qt5-1/polkitqt1-authority.h>
#include <QCoreApplication>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QDebug>
#include <QDBusReply>
#include <QMessageBox>
#include <QString>
#include <QTranslator>
#include <QLocale>
#include <glib.h>
#include <string>
#include <fstream>
#include <iostream>
#include <qprocess.h>
#include <glib/gprintf.h>
#include <QPainterPath>
#include "main_window.h"
#include "lib_fun.h"
#include <QDir>
#include "xatom-helper.h"
#include <QSizePolicy>

main_window::main_window(QString app_name , QString runtime , QString desktop_name , QString zh_name, QWidget *parent) : QWidget(parent)
{

    this->setAttribute(Qt::WA_AlwaysShowToolTips);
    gsettingInit();

    init(app_name , runtime , desktop_name,zh_name);

    KWindowSystem::setState(this->winId(),NET::SkipTaskbar |NET::SkipPager);

    creare_interface_mainbox(desktop_name);
    create_interruputbox();

}

main_window::~main_window()
{

}

void main_window::gsettingInit()
{
    const QByteArray style_id(ORG_LINGMO_STYLE);
    m_pGsettingFontSize = new QGSettings(style_id);
    connect(m_pGsettingFontSize, &QGSettings::changed, this, [=] (const QString &key){
        if (key==GSETTING_KEY) {
            QFont font;
            font.setPixelSize(m_pGsettingFontSize->get(GSETTING_KEY).toInt() * 2.5);
            cancelbutton->adjustSize();

            QFontMetrics font1(debtextLabel->font());
            QString texq1 = setAutoWrap(font1,labeltext,debtextLabel->width());
            debtextLabel->setText(texq1);
            debtextLabel->adjustSize();
            debtextLabel->resize(debtextLabel->size());


            font.setPixelSize(m_pGsettingFontSize->get(GSETTING_KEY).toInt() * 1.8);
            uninstallbutton->adjustSize();

        }
    });
}

void main_window::init(QString app_name , QString runtime , QString desktop_name,QString zh_name)
{
    int i_ret = -1;

    this->setWindowTitle(tr("uninstaller"));

    p_title = NULL;
    p_core = NULL;
    p_uninstall_label = NULL;
    p_vlayout_1 = NULL;
    p_uninstall = NULL;
    p_hlayout_1 = NULL;
    p_result = NULL;

    this->m_app_name = app_name;
    this->m_zhname = zh_name;

    /*get runtime and version*/
    this->m_runtime = runtime;
    if (runtime == "local_pkg") {
        this->m_version = get_local_pkg_version(m_app_name);
        qDebug() << "this->m_version ====" << this->m_version;
        if (this->m_version.isEmpty()) {
            this->m_version = "null";
        }
    }
    else if (runtime == "local_Android_pkg") {
        this->m_version = get_local_Android_pkg_version(m_app_name);

        if (this->m_version.isEmpty()) {
            this->m_version = "null";
        }
    }
    else {
        this->m_version = get_version(m_app_name);
        if (this->m_version.isEmpty()) {
            this->m_version = "null";
        }
    }

    if (this->m_app_name.isEmpty() || this->m_runtime.isEmpty()) {
        printf("Error : pkg_name or runtime is empty\n");
        exit(-1);
    }

#if 0
    QStringList tmp = analysis_app_xml(APP_XML_FILE_PATH);
    if (!tmp.isEmpty()) {
        this->m_runtime = tmp.at(0);
        this->m_version = get_version(app_name);
        if (this->m_version.isEmpty()) {
            this->m_version = "NULL";
        }
    } else {
        this->m_runtime = "local_pkg";
        this->m_version = get_local_pkg_version(m_app_name);
        if (this->m_version.isEmpty()) {
            this->m_version = "NULL";
        }
    }
#endif

    /*get icon and name*/
    char p_icon[1024];
    char p_name[1024];
    char p_exec[1024];

    memset(p_icon , 0x00 , sizeof(p_icon));
    memset(p_name , 0x00 , sizeof(p_name));
    memset(p_exec , 0x00 , sizeof(p_exec));


    printf("Info : analysis icon is [%s]\nInfo : analysis name is [%s]\nInfo : analysis exec is [%s]\n" , p_icon , p_name , p_exec);

    this->m_icon_path = p_icon;
    this->m_name = p_name;
    this->m_exec = p_exec;
    desktopname = desktop_name;
    p_thread = new tool_thread(this->m_runtime , this->m_app_name , this->m_version,zh_name,desktop_name);
}

void main_window::creare_interface_mainbox(QString desktop)
{
    this->setFixedWidth(420);
    this->setMaximumHeight(180);
    debIconLabel = new QLabel();
    debIconLabel->setMaximumSize(48, 48);
    closeButton = new QPushButton(this);

    closeButton->setFlat(true);
    closeButton->setProperty("isWindowButton", 0x2);
    closeButton->setProperty("useIconHighlightEffect", 0x8);
    closeButton->setFixedSize(30,30);
    closeButton->setToolTip(tr("Close"));
    closeButton->setIcon(QIcon::fromTheme("window-close-symbolic"));


    QHBoxLayout * closelayout = new QHBoxLayout();
    closelayout->setContentsMargins(0,5,0,0);
    closelayout->setSpacing(0);
    closelayout->addSpacing(this->width()-closeButton->width()-5);
    closelayout->addWidget(closeButton,Qt::AlignTop);
    closelayout->addSpacing(5);


    QHBoxLayout * iconlayout = new QHBoxLayout();
    iconlayout->setContentsMargins(0,0,0,55);
    iconlayout->setSpacing(0);
    iconlayout->addSpacing(20);
    iconlayout->addWidget(debIconLabel,Qt::AlignTop);



    debtextLabel = new QLabel();
    debtextLabel->setAlignment(Qt::AlignLeft);


    iconTextLayout = new QHBoxLayout();
    iconTextLayout->setContentsMargins(0,0,0,0);
    iconTextLayout->setSpacing(0);
    iconTextLayout->addWidget(debtextLabel);


    cancelbutton = new QToolButton();
    cancelbutton->setMinimumSize(96,36);
    cancelbutton->setText(tr("Cancel"));

    uninstallbutton = new QToolButton();
    uninstallbutton->setMinimumSize(96,36);
    uninstallbutton->setText(tr("Uninstall"));

    buttonLayout = new QHBoxLayout();
    buttonLayout->setContentsMargins(0,0,0,0);
    buttonLayout->setSpacing(0);

    buttonLayout->addStretch(188);
    buttonLayout->addWidget(cancelbutton,0,Qt::AlignLeft);
    buttonLayout->setSpacing(16);
    buttonLayout->addWidget(uninstallbutton,0,Qt::AlignLeft);
    buttonLayout->addSpacing(24);

    allLayout = new QVBoxLayout();
    allLayout->setContentsMargins(0,0,0,0);
    allLayout->setSpacing(0);
    allLayout->addLayout(closelayout);
    allLayout->addStretch();
    allLayout->addLayout(iconTextLayout);
    allLayout->addSpacing(46);
    allLayout->addLayout(buttonLayout);
    allLayout->addSpacing(22);

    QHBoxLayout *allLayout1 = new QHBoxLayout();
    allLayout1->setContentsMargins(0,0,0,0);
    allLayout1->setSpacing(0);
    allLayout1->addLayout(iconlayout);
    allLayout1->addStretch();
    allLayout1->addLayout(allLayout);



    this->setLayout(allLayout1);
    /*先判断主题中是否有icon*/
    if (QIcon::fromTheme(this->m_app_name).isNull()) {
        QString path = getIconPath(this->m_app_name,desktop);
        QPixmap pixmap = QPixmap(getIconPath(this->m_app_name,desktop));
        pixmap.scaled(QSize(48,48), Qt::KeepAspectRatio);
        debIconLabel->setPixmap(QIcon(QPixmap(getIconPath(this->m_app_name,desktop))).pixmap(QSize(48,48)));
    } else {

        debIconLabel->setPixmap(QIcon::fromTheme(this->m_app_name).pixmap(QSize(48,48)));
    }



    QString LANG = getenv("LANG");
    QString info = tr("Are you sure you want to uninstall ");

    if(this->m_zhname.isEmpty()){
        debtextLabel->setText(info + '\"' + this->m_app_name + '\"');
        labeltext = info + '\"' + this->m_app_name + '\"';
    }
    else
    {
        if (LANG.contains("zh_CN")){
            labeltext = info + '\"' + this->m_app_name + '\"';
            debtextLabel->setText(info + '\"' + this->m_zhname + '\"');
        }
        else{
            labeltext = info + '\"' + this->m_app_name + '\"';
            debtextLabel->setText(info + '\"' + this->m_app_name + '\"');
        }
    }

    debtextLabel->setFixedWidth(300);
    QFontMetrics font1(debtextLabel->font());
    QString texq1 = setAutoWrap(font1,debtextLabel->text(),debtextLabel->width());
    debtextLabel->setText(texq1);

    connect(cancelbutton,SIGNAL(clicked(bool)),this,SLOT(slot_close_interface()));
    connect(closeButton,SIGNAL(clicked(bool)),this,SLOT(slot_close_interface()));


    //    this->show();


}

QString main_window::setAutoWrap(const QFontMetrics& font, const QString& text, int nLabelSize)
{

    int nTextSize = font.width(text);
    if (nTextSize > nLabelSize)
    {
        int nPos = 0;
        long nOffset = 0;
        for (int i = 0; i < text.size(); i++)
        {
            nOffset += font.width(text.at(i));
            if (nOffset > nLabelSize)
            {
                nPos = i -1;
                break;
            }
        }

        QString qstrLeftData = text.left(nPos);
        QString qstrMidData = text.mid(nPos);
        return qstrLeftData + "\n" + setAutoWrap(font, qstrMidData, nLabelSize);
    }
    return text;

}

QString main_window::getIconPath(QString appName,QString desktop)
{
    /*解析desktop 文件来获取icon图标路径*/
    QFile file(desktop);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)){
        while (!file.atEnd()){
            QByteArray line = file.readLine();
            QString str(line);
            if(str.contains("Icon",Qt::CaseSensitive)){
                if (str.contains("#Icon",Qt::CaseSensitive))
                    continue;
                QStringList list = str.split("=");
                QString desk_one = list[1];
                QFile file(desk_one);
                if (file.exists()){
                    return desk_one.simplified();
                }
            }
        }
    }
    QString iconFilePath;
    /*判断~/.cache/icons下是否有icon*/
    iconFilePath.clear();
    iconFilePath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.cache/uksc/icons/"+appName + ".png";
    qDebug()<<iconFilePath;
    if (QFile::exists(iconFilePath)) {
        return iconFilePath;
    }

    /*判断~/.local/share/icons/下是否有icon*/
    iconFilePath.clear();
    iconFilePath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.local/share/icons/"+appName + ".svg";
    if (QFile::exists(iconFilePath)) {
        return iconFilePath;
    }

    /*最后判断/usr/share/ubuntu-lingmo-software-center/data/icons下面是否有*/
    iconFilePath.clear();
    iconFilePath = "/usr/share/lingmo-software-center/data/icons/" + appName + ".png";
    qDebug()<<iconFilePath;
    if (QFile::exists(iconFilePath)) {
        return iconFilePath;
    }

    return "";
}


void main_window::create_interruputbox()
{
    /*卸载按钮*/
    connect(uninstallbutton , &QPushButton::clicked , this , &main_window::slot_uninstall);
    /*卸载结果*/
    connect(p_thread , &tool_thread::sig_uninstall_result , this ,&main_window::dispaly_resultbox);

    // connect(p_thread , &tool_thread::sig_uninstall_status , this ,&main_window::set_uninstall_lable_status);

    return;
}
void main_window::dispaly_resultbox(int i_result)
{

    if (i_result == 0) {
        /*卸载成功*/
        delete_android_soft_desktop();
        this->close();
    }
    else
        this->close();
    return;
}


QString main_window::getAppCnName(QString appName)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");

    db.setDatabaseName(QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.cache/uksc/uksc.db");
    if (!db.open()) {
        qDebug()<<"db open failed!";
    }
    /**************************使用QSqlQuery操作数据库**************************/
    QSqlQuery query;	//执行操作类对象

    bool ret = query.exec(QString("SELECT * FROM application WHERE app_name IS '%1'").arg(appName));	//执行
    qDebug()<<"ret:"<<ret;
    while (query.next()) {
        qDebug()<<query.size();
        qDebug()<<query.value(4).toString();
        return query.value(4).toString();
    }
    db.close();
    return nullptr;
}



QString main_window::get_local_pkg_version(QString pkg_name)
{
    QString local_pkg_version;
    local_pkg_version.clear();

    char p_cmd[1024];
    char p_buff[1024];
    FILE *fd = NULL;

    memset(p_cmd , 0x00 , sizeof(p_cmd));
    memset(p_buff , 0x00 , sizeof(p_buff));

    QByteArray std_pkg_name = pkg_name.toLatin1();
    char *p_pkg_name = std_pkg_name.data();

    QProcess terminal;
    QStringList cmdList;
    QString m_app_path;
    m_app_path = pkg_name;
    m_app_path.push_front('"');
    m_app_path.push_front(' ');
    m_app_path.push_back(' ');
    m_app_path.push_back('"');
    cmdList << "-c" << "dpkg -l | grep "+ m_app_path + "| awk '{print $3}' | sed -n '1p'";
    terminal.start("/bin/bash",cmdList);
    terminal.waitForFinished();
    local_pkg_version = terminal.readAll();//获取此包已安装的版本号
    local_pkg_version = local_pkg_version.left(local_pkg_version.length() - 1);

    qDebug() <<"卸载软件的版本   ========  "<<local_pkg_version;
    if (local_pkg_version ==nullptr){
        local_pkg_version = "";
    }
    return local_pkg_version;
}
/*卸载安卓软件时去其对应的/home/xxx/.local/share/application/pkg_name.desktop文件中获取安卓软件的对应版本*/
QString main_window::get_local_Android_pkg_version(QString pkg_name)
{
    QString local_Android_pkg_version;
    QString home_path;

    local_Android_pkg_version.clear();

    home_path = "";
    home_path.append("/home/").append(g_get_user_name()).append("/.local/share/applications/").append(pkg_name).append(".desktop");

    QByteArray ba = home_path.toLatin1();
    char *filedesktop =ba.data();

    QFile file(filedesktop);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        while (!file.atEnd())
        {
            QByteArray line = file.readLine();
            QString str(line);

            if(str.contains("Exec=/usr/bin/startapp",Qt::CaseSensitive))
            {
                QStringList list = str.split(" ");
                if (list.count() == 2){
                    local_Android_pkg_version = "";
                    return local_Android_pkg_version;
                }
                else if((list.count() == 3)){
                    QStringList list1=list[list.count()-1].split("\n");
                    local_Android_pkg_version =list1[0];
                    return local_Android_pkg_version;
                }
                else{
                    qDebug() << "抛出异常";
                    local_Android_pkg_version = "";
                    return local_Android_pkg_version;
                }
            }
        }
        file.close();

    }
    else{
        qDebug() << "安卓desktop文件不存在";
    }

    return local_Android_pkg_version;
}


/*卸载安卓软件后删除对应desktop文件*/
void main_window::delete_android_soft_desktop()
{
    QString filename;
    QStringList cmdList;
    if ((m_app_name.contains("topsap",Qt::CaseSensitive))){
        m_app_name = "TopSAP";
    }
    filename.append("/home/").append(g_get_user_name()).append("/.local/share/applications/").append(m_app_name).append(".desktop");
    qDebug() << "    dektop path    =====    " <<filename;
    QFile file(filename);
    if (file.exists()){
        file.remove();
    }
    QString desktoppath;
    desktoppath.clear();
    QFileInfo desktop (desktopname);
    QString filenameer = desktop.fileName();
    desktoppath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)+"/"+filenameer;
    QFile shortcutFile(desktoppath);
    if (shortcutFile.exists()){
        shortcutFile.remove();
    }
}

bool main_window::polkit()
{
    PolkitQt1::Authority::Result result;

    result = PolkitQt1::Authority::instance()->checkAuthorizationSync("org.lingmo.uninstaller.action" ,
                                                                      PolkitQt1::UnixProcessSubject(QCoreApplication::applicationPid()) ,
                                                                      PolkitQt1::Authority::AllowUserInteraction);
    if (result == PolkitQt1::Authority::Yes) {
        qDebug() << QString("operation authorized");
        return true;
    } else {
        qDebug() << QString("not authorized");
        return false;
    }
}

QString main_window::get_version(QString pkg_name)
{
    QString version;
    version.clear();

    /*创建接口映射*/
    QDBusInterface *server_fun = new QDBusInterface("com.lingmo.runtime" , "/com/lingmo/runtime" , "com.lingmo.runtime.interface" , QDBusConnection::systemBus());
    if (!server_fun->isValid()) {
        qDebug() << "创建dbus接口映射失败：" << QDBusConnection::systemBus().lastError().message();
        return version;
    }
    QDBusReply<QString> reply = server_fun->call("get_version" , pkg_name);
    version = reply;


    return version;
}


/*protected fun*/
void main_window::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isDragging) {
        move(event->globalPos() - m_dragStartPosition);
        event->accept();
    } else {
        event->ignore();
    }
    return;
}

void main_window::mousePressEvent(QMouseEvent *event)
{

    // 如果鼠标点击的是MyWidget的顶部10像素的区域，则开始拖拽
    if (event->y() < 5) {
        m_isDragging = true;
        m_dragStartPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    } else {
        m_isDragging = false;
        event->ignore();
    }
    return;
}

void main_window::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_isDragging) {
        m_isDragging = false;
        event->accept();
    } else {
        event->ignore();
    }
}

void main_window::paintEvent(QPaintEvent *event)
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

void main_window::keyReleaseEvent(QKeyEvent *event){
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
    if(keyEvent->key() == Qt::Key_Escape){
        slot_close_interface();
    }
}


/*槽函数*/
void main_window::slot_close_interface()
{
    qDebug() << this->width();
    this->close();
    return;
}

void main_window::slot_minisize_interface()
{
    this->showMinimized();
    return;
}

void main_window::slot_uninstall()
{
    this->hide();


    p_thread->start();

    return;
}
