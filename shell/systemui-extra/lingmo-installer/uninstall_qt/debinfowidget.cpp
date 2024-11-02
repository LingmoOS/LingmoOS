#include "debinfowidget.h"
#include "lib_fun.h"

#define ORG_LINGMO_STYLE_FONT            "org.lingmo.style"
#define GSETTING_FONT_KEY              "systemFontSize"

debInfoWidget::debInfoWidget(QString icon , QString name , QString appName, QString appVersion, QWidget *parent) : QWidget(parent)
{

    qDebug() << "zzzzzzzzzzzzzzzzzzz====" << name;
    qDebug() << "zzzzzzzzzzzzzzzzzzz====" << appName;
    m_appName = appName;
    m_appVersion = appVersion;
    m_name =  name;
    int DEB_lenth = 0;

    debInfoLayout = new QVBoxLayout();
    debInfoLayout->setContentsMargins(0, 0, 0, 0);

    IconAndAppNameWidget = new QWidget();
    //IconAndAppNameWidget->setFixedSize(550, 120);
    IconAndAppNameWidget->setContentsMargins(0, 0, 0, 0);

    IconAndAppNameLayout = new QHBoxLayout();
    IconAndAppNameLayout->setContentsMargins(0,0,0,0);
    IconAndAppNameLayout->setSpacing(0);
    debIconLabel = new QLabel();
    debIconLabel->setFixedSize(48, 48);

    /*先判断主题中是否有icon*/
    if (QIcon::fromTheme(m_appName).isNull()) {
        QPixmap pixmap = QPixmap(getIconPath(m_appName));
        pixmap.scaled(QSize(48,48), Qt::KeepAspectRatio);
        debIconLabel->setPixmap(QIcon(QPixmap(getIconPath(m_appName))).pixmap(QSize(48,48)));
    } else {
        debIconLabel->setPixmap(QIcon::fromTheme(m_appName).pixmap(QSize(48,48)));
    }
    debIconLabel->adjustSize();
    fontGsettingInit();
    debAppNameLabel = new QLabel();
    QFont font2 =  debAppNameLabel->font();
    font2.setPixelSize(m_pGsettingFontSize->get(GSETTING_FONT_KEY).toInt() * 2.5);
    debAppNameLabel->setFont(font2);
    //
    QLocale locale;

    debAppNameLabel->setFixedWidth(260);

    if (locale.language() == QLocale::Chinese) {
        setdebnamelable();
        //debAppNameLabel->setText(setLabelStringBody(getAppCnName(m_appName), debAppNameLabel));

      /* if (font3.width(getAppCnName(m_appName))< 110){
            debAppNameLabel->setFixedWidth(font3.width(getAppCnName(m_appName))+1);
            DEB_lenth = 50;
         }
        else if (font3.width(getAppCnName(m_appName))>110 && font3.width(getAppCnName(m_appName))<200 ){
            debAppNameLabel->setFixedWidth(font3.width(getAppCnName(m_appName))+1);
            DEB_lenth = 25;
         }
        else
            DEB_lenth = 0;*/
    } else {
        debAppNameLabel->setWordWrap(true);
        debAppNameLabel->setText(setLabelStringBody(m_appName, debAppNameLabel));
    }

  //  qDebug() << "字体真实长度MMMMMMMMM====" <<  debAppNameLabel->width();


    IconAndAppNameLayout->addItem(new QSpacerItem((550-debAppNameLabel->width()-48-12)/2-DEB_lenth, 10, QSizePolicy::Fixed));
    IconAndAppNameLayout->addWidget(debIconLabel);
    IconAndAppNameLayout->addItem(new QSpacerItem(12, 10, QSizePolicy::Fixed));
    IconAndAppNameLayout->addWidget(debAppNameLabel);
    IconAndAppNameLayout->addItem(new QSpacerItem((550-debAppNameLabel->width()-48)/2, 10, QSizePolicy::Expanding));
    IconAndAppNameWidget->setLayout(IconAndAppNameLayout);

    debNameAndVersionWidget = new QWidget();
    debNameAndVersionLayout = new QHBoxLayout();
    debNameAndVersionLayout->setContentsMargins(0,0,0,0);
    debNameAndVersionLayout->setSpacing(0);

    debNameLabel = new QLabel();
    QString debName = tr("deb name:");
    debName.append(m_appName);
    debNameLabel->adjustSize();
    debNameLabel->setText(debName);
    //debNameLabel->adjustSize();
    debNameLabel->setFixedWidth(280);
    debNameLabel->setWordWrap(true);

    debNameLabel_pkg = new QLabel();
    debNameLabel_pkg->setText(m_appName);

    debVersionLabel = new QLabel();
    QString debVersion = tr("deb version:");
    debVersion.append(appVersion);
    debVersionLabel->setText(debVersion);
    debVersionLabel->adjustSize();
    if (debVersionLabel->width() > 329) {
        debVersionLabel->setText(debVersion.left(20) + "...");
        debVersionLabel->setToolTip(debVersion);
    }
    nameVersionWidget = new QWidget();
    nameVersionVLayout = new QVBoxLayout();
    nameVersionVLayout->setContentsMargins(0, 0, 0, 0);
    nameVersionVLayout->addWidget(debNameLabel);
    nameVersionVLayout->setSpacing(15);
    nameVersionVLayout->addWidget(debVersionLabel);

    nameVersionWidget->setLayout(nameVersionVLayout);
    qDebug() << "deb包名====" <<debAppNameLabel->width();
    debNameAndVersionLayout->addItem(new QSpacerItem((550-debAppNameLabel->width()-48)/2+48+6-DEB_lenth, 20, QSizePolicy::Fixed));
    debNameAndVersionLayout->addWidget(nameVersionWidget);
    debNameAndVersionLayout->addItem(new QSpacerItem(0, 20, QSizePolicy::Expanding));
    debNameAndVersionWidget->setLayout(debNameAndVersionLayout);

    debInfoLayout->addWidget(IconAndAppNameWidget);
    debInfoLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding));
    debInfoLayout->addWidget(debNameAndVersionWidget);
    this->setLayout(debInfoLayout);
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
    return formatBody;
}

//设置字符显示长度函数
QString debInfoWidget::setdebnamelable()
{
    QString qstrData;
    QFontMetrics font3 (debAppNameLabel->font());
   // qDebug() << "返回字符串的名字111111111====" << font3.width(formatBody);
    qDebug() << "返回字符串的名字2222222222222222====" << m_appName;
    if (getAppCnName(m_appName).isNull()){
        if (font3.width(m_appName) > debAppNameLabel->width()){
           qstrData = SpliteText(font3,m_appName, debAppNameLabel->width());
           QStringList list = qstrData.split("\n");

           if (list.count() <=1){
               debAppNameLabel->setWordWrap(false);
               debAppNameLabel->setText(qstrData);
           }
           else{
               debAppNameLabel->setWordWrap(false);
               debAppNameLabel->setText(setLabelStringBody(qstrData,debAppNameLabel));
               debAppNameLabel->setToolTip(m_appName);
           }
        }
        else{
            qDebug() << "返回字符串的名字33333333333====" << m_appName;
            debAppNameLabel->setWordWrap(true);
            debAppNameLabel->setText(m_appName);
            qDebug() << "返回字符串的名字4444444444444====" << m_appName;
        }
    }else{
        qDebug() << "返回字符串的名字2222222222222wwwww====" << m_appName;
        debAppNameLabel->setWordWrap(false);
        debAppNameLabel->setText(setLabelStringBody(m_name,debAppNameLabel));
        //debAppNameLabel->setText(formatBody);
        debAppNameLabel->setToolTip(m_name);
    }
    return nullptr;
}

void debInfoWidget::setLabelText()
{
    QFont font;
    font.setPixelSize(m_pGsettingFontSize->get(GSETTING_FONT_KEY).toInt() * 2.5);
    debAppNameLabel->setFont(font);
    QLocale locale;
    if (locale.language() == QLocale::Chinese) {
        setdebnamelable();//设置debnamelable的文字显示问题
    }
    else {
        debAppNameLabel->setText(setLabelStringBody(m_appName, debAppNameLabel));
    }

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


QString debInfoWidget::getAppCnName(QString appName)
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

QString debInfoWidget::getIconPath(QString appName)
{
    QString iconFilePath;
    /*判断~/.cache/icons下是否有icon*/
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


void debInfoWidget::resetAppInfo(QString appName,QString appVersion)
{
    m_appName = appName;
    m_appVersion = appVersion;
    /*先判断主题中是否有icon*/
    if (QIcon::fromTheme(m_appName).isNull()) {
        QPixmap pixmap = QPixmap(getIconPath(m_appName));
        pixmap.scaled(QSize(48,48), Qt::KeepAspectRatio);
        debIconLabel->setPixmap(QIcon(QPixmap(getIconPath(m_appName))).pixmap(QSize(48,48)));
    } else {
        debIconLabel->setPixmap(QIcon::fromTheme(m_appName).pixmap(QSize(48,48)));
    }
    setdebnamelable();//设置字符在debnamelable的显示问题

    QString debName = tr("deb name:");
    debName.append(appName);
    debNameLabel->setText(debName);

    QString debVersion = tr("deb version:");
    debVersion.append(m_appVersion);
    debVersionLabel->setText(debVersion);
    if (debVersionLabel->width() > 329) {
        debVersionLabel->setText(debVersion.left(20) + "...");
        debVersionLabel->setToolTip(debVersion);
    }
}


