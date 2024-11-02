#include "batch-install.h"

Batchinstall::Batchinstall( QString debpath, QWidget *parent) : QWidget(parent)
{
    const QByteArray style_id(ORG_LINGMO_STYLE_FONT);
    m_pGsettingFontSize = new QGSettings(style_id);

     m_appname = getDebInfoFromDeb(debpath, "Package");
     m_appname = m_appname.toLower();
     m_appversion = getDebInfoFromDeb(debpath, "Version");
     m_debArch = getDebInfoFromDeb(debpath,"Architecture");
     zh_apname = getAppCnName(m_appname);
     m_debpath = debpath;

     appiconlabel = new QLabel();
     appiconlabel->adjustSize();
     appnamelabel = new QLabel();
     appversionlabel =new QLabel();
     appversionlabel->setMaximumWidth(240);

     deleteitem = new QToolButton();
     deleteitem->setIcon(QIcon::fromTheme("edit-delete-symbolic"));
     deleteitem->setIconSize(QSize(16, 16));
     deleteitem->setFixedSize(36, 36);
     deleteitem->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
//     deleteitem->setToolButtonStyle(Qt::ToolButtonIconOnly);
     deleteitem->setObjectName("delete_app");
     deleteitem->adjustSize();
//     deleteitem->setFlat(false);
     deleteitem->setProperty("isWindowButton", 0x1);
     deleteitem->setProperty("useIconHighlightEffect", 0x2);
     deleteitem->setAutoRaise(true);
//     deleteitem->setStyleSheet("QToolButton {"
//                               "    background-color: transparent;"
//                               "    border: none;"
//                               "}");


     appstatus = new QLabel();
     appstatus->setText(tr("Wait"));
     appstatus->setObjectName("appstatus");

//     appstatus->setFont(getSystemFont(14));
     appstatus->adjustSize();

     pProgressBar = new QProgressBar();
     pProgressBar->setOrientation(Qt::Horizontal);  // 水平方向
     pProgressBar->setMinimum(0);  // 最小值
     pProgressBar->setMaximum(100);  // 最大值
     pProgressBar->setValue(1);  // 当前进度
     pProgressBar->setObjectName("pProgressBar");
     pProgressBar->setFixedWidth(80);
     pProgressBar->setFixedHeight(8);
     pProgressBar->setTextVisible(false);
     pProgressBar->hide();

     progress = new QLabel();
     progress->setText("1%");
     progress->adjustSize();
     progress->setObjectName("progress");
     progress->hide();


     m_appiconlabel = new QLabel();
     m_appiconlabel->setFixedSize(25,25);
     m_appiconlabel->setObjectName("app_status");
     m_appiconlabel->hide();


    connect(deleteitem, &QPushButton::clicked, this, &Batchinstall::removelistwidgetitem);

    if (QIcon::fromTheme(m_appname).isNull()) {
        QPixmap pixmap = QPixmap(getIconPath(m_appname));
        pixmap.scaled(QSize(48,48), Qt::KeepAspectRatio);
        if(getIconPath(m_appname) != nullptr){
            appiconlabel->setPixmap(QIcon(QPixmap(getIconPath(m_appname))).pixmap(QSize(48,48)));
        }else{
            appiconlabel->setPixmap(QIcon::fromTheme("application-x-desktop").pixmap(QSize(48,48)));
        }
    } else {
        appiconlabel->setPixmap(QIcon::fromTheme(m_appname).pixmap(QSize(48,48)));
    }

    widgetall = new QWidget();
    debiconLayout = new QHBoxLayout();
    debiconLayout->setContentsMargins(0, 0, 16, 0);
    debiconLayout->setSpacing(0);
    debiconLayout->addWidget(appiconlabel);
    debNameAndVersioNLayout = new QVBoxLayout();
    debNameAndVersioNLayout->setContentsMargins(0, 0, 0, 0);
    debNameAndVersioNLayout->setSpacing(0);
    if(zh_apname.isEmpty()){
        appnamelabel->setText(m_appname);

    }else{
         appnamelabel->setText(zh_apname);
    }
//    appnamelabel->setFont(getSystemFont(14));
    QString debVersion = tr("deb version :");
    debVersion.append(m_appversion);

   // appversionlabel->setText(debVersion);

    appversionlabel->setText(setLabelStringBody(debVersion, appversionlabel));
//    appversionlabel->setFont(getSystemFont(14));
    appversionlabel->setObjectName("appversion");
    appversionlabel->adjustSize();
    appversionlabel->setAlignment(Qt::AlignTop);
//    appversionlabel->setStyleSheet("color: #818181;");
//    // 创建一个调色板对象
//    QPalette palette;

    // 设置文本颜色
//    QColor textColor("#818181");
//    palette.setColor(QPalette::WindowText, textColor);

//    // 将调色板应用于标签
//    appversionlabel->setPalette(palette);

//    // 设置标签的自动填充背景为真，以便调色板生效
//    appversionlabel->setAutoFillBackground(false);

//    // 设置标签的不透明度
//    appversionlabel->setWindowOpacity(1.0);



    appnamelabel->adjustSize();
    appnamelabel->setAlignment(Qt::AlignBottom);


    qInfo() << "appname======================sssssssssss====" << appnamelabel->font();


    debNameAndVersioNLayout->addWidget(appnamelabel);
    debNameAndVersioNLayout->addSpacing(4);
    debNameAndVersioNLayout->addWidget(appversionlabel);
    int lenth =0;
    if (appnamelabel->width()>appversionlabel->width()){
        lenth = appnamelabel->width();

    }
    else{
        lenth = appversionlabel->width();

    }

    statuswidget = new QWidget();


    deleteitemlayout = new QHBoxLayout();

    deleteitemlayout->addStretch();

    deleteitemlayout->addWidget(appstatus);
    deleteitemlayout->addWidget(pProgressBar);
    deleteitemlayout->addSpacing(10);
    deleteitemlayout->addWidget(progress);
    deleteitemlayout->setAlignment(Qt::AlignRight);
    deleteitemlayout->setContentsMargins(0, 0, 0, 0);
    deleteitemlayout->setSpacing(0);


    stauslayout = new QHBoxLayout();
    stauslayout->setContentsMargins(0, 0, 0, 0);
    stauslayout->setSpacing(0);

    stauslayout->addWidget(deleteitem);
    stauslayout->addWidget(m_appiconlabel);

    stauslayoutall = new QHBoxLayout();
    stauslayoutall->setContentsMargins(0, 0, 0, 0);
    stauslayoutall->setSpacing(0);
    stauslayoutall->addLayout(deleteitemlayout);
    stauslayoutall->setSpacing(10);
    stauslayoutall->addLayout(stauslayout);
    statuswidget->setLayout(stauslayoutall);





    Layoutall = new QHBoxLayout();
    Layoutall->addLayout(debiconLayout);
    Layoutall->addLayout(debNameAndVersioNLayout);
    Layoutall->addStretch();
    Layoutall->addWidget(statuswidget);
    Layoutall->addSpacing(16);
    Layoutall->setContentsMargins(0, 0, 0, 0);
    Layoutall->setSpacing(0);


   // Layoutall->setAlignment(Qt::AlignLeft);

    this->setLayout(Layoutall);
    this->setProperty("useIconHighlightEffect", 0x2);


}

QString Batchinstall::getDebInfoFromDeb(QString m_debPath, QString type)
{
    QStorageInfo SD;
    SD.setPath("/");
    SD.refresh();
    int disk_space = SD.bytesAvailable()/1024/1024;
    if (disk_space <=10)
    {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText(tr("Insufficient disk space!!!!"));
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
        cmdList.clear();
        terminal.close();
        cmdList << "-c" << QString("dpkg -I " + m_debPath + "| grep " + type);
        terminal.start("/bin/bash",cmdList);
        terminal.waitForFinished();

        debInfo = terminal.readAll();

        if ( debInfo!="" && debInfo.contains(type)){
            QStringList list = debInfo.split(":");
            return list[list.count()-1].simplified();
        }
    }
    else if (debInfo!="" && debinfoerro.contains("压缩数据已损坏"))
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

QString Batchinstall::getAppCnName(QString appName)
{
    QSqlDatabase db;
    if (QSqlDatabase::contains("qt_sql_default_connection"))
        db = QSqlDatabase::database("qt_sql_default_connection");
    else
        db = QSqlDatabase::addDatabase("QSQLITE");

    db.setDatabaseName(QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.cache/uksc/uksc.db");
    if (!db.open()) {
        qDebug()<<"db open failed!";
    }
    /**************************使用QSqlQuery操作数据库**************************/
    QSqlQuery query;	//执行操作类对象

    bool ret = query.exec(QString("SELECT * FROM application WHERE app_name IS '%1'").arg(appName));	//执行
    qDebug()<<"ret:"<<ret;
    while(query.next()) {
        qDebug()<<query.size();
        qDebug()<<query.value(4).toString();
        return query.value(4).toString();
    }
    db.close();
    return nullptr;
}

QString Batchinstall::getIconPath(QString appName)
{
    QString iconFilePath;
    /*cr判断~/.cache/icons下是否有icon*/
    iconFilePath.clear();
    iconFilePath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.cache/uksc/icons/"+appName + ".png";
    qDebug()<<iconFilePath;
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
    return nullptr;
}

/********发送删除item信号********/
void Batchinstall::removelistwidgetitem()
{
     emit deleteitemstring(m_debpath);
}

QString Batchinstall::setLabelStringBody(QString text, QLabel *lable)
{
    QFontMetrics fontMetrics(lable->font());
    int LableWidth = lable->width();
    int fontSize = fontMetrics.width(text);
    QString formatBody = text;
    if(fontSize > (LableWidth - 10)) {
        //说明只存在一行，在最后面加...就行
        formatBody = fontMetrics.elidedText(formatBody, Qt::ElideRight, LableWidth - 10);
        lable->setToolTip(text);
        return formatBody;
    }
    lable->setToolTip(text);
    return formatBody;
}

QFont Batchinstall::getSystemFont(int size)
{
    double fontsize = (m_pGsettingFontSize->get(GSETTING_FONT_KEY).toFloat());

//    double lableBaseFontSize = size * 9/12;//魔鬼数字，自行处理
//    double nowFontSize = lableBaseFontSize * double(fontsize) / 11;//11为系统默认大小，魔鬼数字，自行处理
    QFont font;
    if (size == 18){
        font.setFamily("Noto Sans CJK SC");
        font.setWeight(500);

    }
    font.setBold(false);
    font.setPointSizeF(fontsize);

    return font;

}
