#include "debinfowidget.h"

debInfoWidget::debInfoWidget( QString appName, QString appVersion, QWidget *parent) : QWidget(parent)
{

    m_appName = appName;
    m_appVersion = appVersion;
    int DEB_lenth = 0;

    debInfoLayout = new QVBoxLayout();
    debInfoLayout->setContentsMargins(0, 0, 0, 0);

    IconAndAppNameWidget = new QWidget();
    //IconAndAppNameWidget->setFixedSize(550, 120);
    IconAndAppNameWidget->setContentsMargins(0, 0, 0, 0);

    IconAndAppNameLayout = new QVBoxLayout();
    IconAndAppNameLayout->setContentsMargins(0,0,0,0);
    IconAndAppNameLayout->setSpacing(0);
    debIconLabel = new QLabel();
    debIconLabel->setFixedSize(128, 128);
    /*先判断主题中是否有icon*/
    if (QIcon::fromTheme(m_appName).isNull()) {
        QPixmap pixmap = QPixmap(getIconPath(m_appName));
        pixmap.scaled(QSize(128,128), Qt::KeepAspectRatio);
        if(getIconPath(m_appName) != nullptr){
            debIconLabel->setPixmap(QIcon(QPixmap(getIconPath(m_appName))).pixmap(QSize(128,128)));
        }else{
            debIconLabel->setPixmap(QIcon::fromTheme("application-x-desktop").pixmap(QSize(128,128)));
        }
    }else {
        debIconLabel->setPixmap(QIcon::fromTheme(m_appName).pixmap(QSize(128,128)));
    }
    debIconLabel->adjustSize();
    fontGsettingInit();
    debAppNameLabel = new QLabel();
//    debAppNameLabel->setFont(getSystemFont(18));
    //
    QLocale locale;


    IconAndAppNameLayout->addSpacing(82);
    IconAndAppNameLayout->addWidget(debIconLabel);
    IconAndAppNameLayout->setAlignment(Qt::AlignHCenter);
    IconAndAppNameWidget->setLayout(IconAndAppNameLayout);

    debNameAndVersionWidget = new QWidget();
    debNameAndVersionLayout = new QHBoxLayout();
    debNameAndVersionLayout->setContentsMargins(0,0,0,0);
    debNameAndVersionLayout->setSpacing(0);

    debVersionLabel = new QLabel();
    QString debVersion = tr("deb version:");
    debVersion.append(appVersion);
    debVersionLabel->setText(debVersion);
    debVersionLabel->adjustSize();
    if (debVersionLabel->width() > 329) {
        debVersionLabel->setText(debVersion.left(20) + "...");
        debVersionLabel->setToolTip(debVersion);
    }
//    debVersionLabel->setFont(getSystemFont(14));

    // 创建一个调色板对象
    QPalette palette;

    // 设置文本颜色
    QColor textColor("#818181");
    palette.setColor(QPalette::WindowText, textColor);


//     将调色板应用于标签
    debVersionLabel->setPalette(palette);

    // 设置标签的自动填充背景为真，以便调色板生效
    debVersionLabel->setAutoFillBackground(false);

    // 设置标签的不透明度
    debVersionLabel->setWindowOpacity(1.0);

    if (locale.language() == QLocale::Chinese) {

        setdebnamelable();

    } else {
        debAppNameLabel->setWordWrap(true);
        debAppNameLabel->setText(setLabelStringBody(m_appName, debAppNameLabel));
    }
    if (debAppNameLabel->text().isEmpty())
        debAppNameLabel->setText(m_appName);
    nameVersionWidget = new QWidget();
    nameVersionVLayout = new QVBoxLayout();
    nameVersionVLayout->setContentsMargins(0, 0, 0, 0);

    Namelyout =new QHBoxLayout();
    Namelyout->setContentsMargins(0, 0, 0, 0);
    Namelyout->addWidget(debAppNameLabel);
    Namelyout->setAlignment(Qt::AlignHCenter);

    Versionlyout =new QHBoxLayout();
    Versionlyout->setContentsMargins(0, 0, 0, 0);
    Versionlyout->addWidget(debVersionLabel);
    Versionlyout->setAlignment(Qt::AlignHCenter);

    nameVersionVLayout->addLayout(Namelyout);
    nameVersionVLayout->setSpacing(15);
    nameVersionVLayout->addLayout(Versionlyout);
    nameVersionVLayout->setAlignment(Qt::AlignHCenter);
    nameVersionWidget->setLayout(nameVersionVLayout);
//    debNameAndVersionLayout->addWidget(nameVersionWidget);
//    debNameAndVersionLayout->setAlignment(Qt::AlignCenter);
//    debNameAndVersionWidget->setLayout(debNameAndVersionLayout);

    debInfoLayout->addWidget(IconAndAppNameWidget);
    debInfoLayout->setSpacing(20);
    debInfoLayout->addWidget(nameVersionWidget);
    this->setLayout(debInfoLayout);
}

/* 设置label字体长度，超出label时，设置... */
QString debInfoWidget::setLabelStringBody(QString text, QLabel *lable)
{
    QFontMetrics fontMetrics(lable->font());
    int LableWidth = lable->width();
    int fontSize = fontMetrics.width(text);
    QString formatBody = text;
    if(fontSize > (LableWidth - 10)) {
        //说明只存在一行，在最后面加...就行
        formatBody = fontMetrics.elidedText(formatBody, Qt::ElideRight, LableWidth - 10);
        return formatBody;
    }
    debAppNameLabel->setToolTip(getAppCnName(m_appName));
    return formatBody;
}
/*处理QT中纯英文在QLable中的换行问题*/
QString debInfoWidget::SpliteText(QFontMetrics font, QString text,int nlablesize)
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

QString debInfoWidget::getAppCnName(QString appName)
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
   // qDebug()<<"ret:"<<ret;
    while(query.next()) {
     //   qDebug()<<query.size();
     //   qDebug()<<query.value(4).toString();
        return query.value(4).toString();
    }
    db.close();
    return "";
}

QString debInfoWidget::getIconPath(QString appName)
{
    QString iconFilePath;
    /*cr判断~/.cache/icons下是否有icon*/
    iconFilePath.clear();
    iconFilePath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.cache/uksc/icons/"+appName + ".png";
    if (QFile::exists(iconFilePath)) {
        return iconFilePath;
    }
    /*最后判断/usr/share/ubuntu-lingmo-software-center/data/icons下面是否有*/
    iconFilePath.clear();
    iconFilePath = "/usr/share/lingmo-software-center/data/icons/" + appName + ".png";
    if (QFile::exists(iconFilePath)) {
        return iconFilePath;
    }
    return nullptr;
}

void debInfoWidget::resetAppInfo(QString appName, QString appVersion)
{
    m_appName = appName;
    m_appVersion = appVersion;
    QLocale locale;
    /*先判断主题中是否有icon*/
    if (QIcon::fromTheme(m_appName).isNull()) {
        QPixmap pixmap = QPixmap(getIconPath(m_appName));
        pixmap.scaled(QSize(128,128), Qt::KeepAspectRatio);
        if(getIconPath(m_appName) != nullptr){
            debIconLabel->setPixmap(QIcon(QPixmap(getIconPath(m_appName))).pixmap(QSize(128,128)));
        }else{
            debIconLabel->setPixmap(QIcon::fromTheme("application-x-desktop").pixmap(QSize(128,128)));
        }
    }else {
        debIconLabel->setPixmap(QIcon::fromTheme(m_appName).pixmap(QSize(128,128)));
    }
    debIconLabel->adjustSize();
    //setdebnamelable();//设置字符在debnamelable的显示问题
    if (locale.language() == QLocale::Chinese){
        QString zh_appname = getAppCnName(m_appName);
        if (getAppCnName(m_appName).isEmpty())
            debAppNameLabel->setText(m_appName);
        else
            debAppNameLabel->setText(zh_appname);
    }
    else
        debAppNameLabel->setText(m_appName);

    QString debVersion = tr("deb version:");
    debVersion.append(appVersion);
    //debAppNameLabel->setText(m_appName);
    debVersionLabel->setText(debVersion);

}

void debInfoWidget::fontGsettingInit()
{
    const QByteArray style_id(ORG_LINGMO_STYLE_FONT);
    m_pGsettingFontSize = new QGSettings(style_id);
    connect(m_pGsettingFontSize, &QGSettings::changed, this, [=] (const QString &key){
        if (key == GSETTING_FONT_KEY) {
            setLabelText();
        }
    });
}

void debInfoWidget::setLabelText()
{
    debAppNameLabel->setFont(getSystemFont(18));
    debVersionLabel->setFont(getSystemFont(14));
    QLocale locale;

}

//设置字符显示长度函数
void debInfoWidget::setdebnamelable()
{
    QString qstrData;
    QFontMetrics font3 (debAppNameLabel->font());

    if (getAppCnName(m_appName).isNull()){

        if (font3.width(m_appName) > debAppNameLabel->width()){

                   qstrData = SpliteText(font3,m_appName, debAppNameLabel->width());//处理纯英文字符无法自动换行问题
                   //debAppNameLabel->setText(setLabelStringBody(m_appName, debAppNameLabel));
                   if (qstrData.isEmpty()){
                       debAppNameLabel->setWordWrap(true);
                       debAppNameLabel->setText(m_appName);
                      // setdebNamelable();
                   }
                   else{
                       QStringList list = qstrData.split("\n");
                      if (list.count() <=1){
                           debAppNameLabel->setWordWrap(true);
                           debAppNameLabel->setText(qstrData);
                         //  setdebNamelable();
                       }
                       else{
                           debAppNameLabel->setWordWrap(false);
                           debAppNameLabel->setText(setLabelStringBody(qstrData,debAppNameLabel));
                           debAppNameLabel->setToolTip(m_appName);
                         //  setdebNamelable();


                       }
                   }
        }
        else{
            debAppNameLabel->setWordWrap(true);
            debAppNameLabel->setText(m_appName);
          //  setdebNamelable();
        }
    }
    else{
        debAppNameLabel->setWordWrap(true);
        debAppNameLabel->setText(setLabelStringBody(getAppCnName(m_appName),debAppNameLabel));
        debAppNameLabel->setToolTip(getAppCnName(m_appName));
      //  setdebNamelable();

    }

}

void debInfoWidget::setdebNamelable(){
     QFontMetrics font4 (debNameLabel->font());
     QString debName = tr("deb name:");
      QString qstrData;
     debName.append(m_appName);
    if (font4.width(debName) > debNameLabel->width()){
        qstrData = SpliteText(font4,m_appName, debNameLabel->width());

        debName.append(qstrData);
        debNameLabel->setWordWrap(false);
        debNameLabel->setText(setLabelStringBody(debName,debNameLabel));
        debNameLabel->setToolTip(m_appName);
    }
    else{
        debNameLabel->setWordWrap(true);
        debNameLabel->setText(debName);
    }
}
QFont debInfoWidget::getSystemFont(int size)
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

