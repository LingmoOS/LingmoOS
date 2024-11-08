/*
 * liblingmosdk-qtwidgets's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#include <QBoxLayout>
#include <QPixmap>
#include <QIcon>
#include <QLabel>
#include <QTextEdit>
#include <QStyleOption>
#include <QPainter>
#include <QTextDocument>
#include <QDesktopServices>
#include <QGuiApplication>
#include <QMouseEvent>
#include <QProcess>
#include <QDir>
#include <QSettings>
#include <QMessageBox>
#include <QLibraryInfo>
#include <QTranslator>
#include <QApplication>
#include "kaboutdialog.h"

namespace kdk
{
#define MAILTYPE "x-scheme-handler/mailto"
#define LOCAL_CONFIG_DIR "/.config/"
#define SYSTEM_CONFIG_DIR "/usr/share/applications/"

class LinkLabel :public QLabel
{
    Q_OBJECT

public:
    LinkLabel(QWidget *parent=nullptr);

protected:
    void mousePressEvent(QMouseEvent *event);

};
class KAboutDialogPrivate:public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KAboutDialog)

public:
    KAboutDialogPrivate(KAboutDialog*parent);
    void adjustMiniMode();
    QString getDefaultAppId(const char *contentType);
    bool isOpenLingmo();
    void updateAppVersionText();
private:
    KAboutDialog *q_ptr;
    QLabel *m_pAppIconLabel;
    QLabel *m_pAppNameLabel;
    QLabel *m_pAppVersionLabel;
    QLabel *m_pAppSupportLabel;
    LinkLabel *m_pAppPrivacyLabel;
    QString m_iconName;
    QString m_bodyText;
    bool m_appVersionFlag;
};

KAboutDialog::KAboutDialog(QWidget*parent,const QIcon& appIcon,const QString& appName,const QString& appVersion,const QString& appInfo)
    :KDialog(parent),
      d_ptr(new KAboutDialogPrivate(this))
{
    Q_D(KAboutDialog);
    setProperty("isAboutDialog",true);
    mainLayout()->setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);
    d->m_iconName = appIcon.name();
    d->m_pAppIconLabel = new QLabel(this);
    QPixmap pix(appIcon.pixmap(QSize(96,96)));
    d->m_pAppIconLabel->setPixmap(pix);
    d->m_pAppIconLabel->setAlignment(Qt::AlignCenter);
    d->m_pAppNameLabel = new QLabel(this);
    d->m_pAppNameLabel->setText(appName);
    d->m_pAppNameLabel->setAlignment(Qt::AlignCenter);

    if(!(appVersion=="") && (!(appVersion.isNull())) )
        d->m_appVersionFlag =false;
    d->m_pAppVersionLabel = new QLabel(this);
    d->m_pAppVersionLabel->setText(appVersion);
    d->m_pAppVersionLabel->setAlignment(Qt::AlignCenter);
    d->m_pAppVersionLabel->setFixedWidth(400);

    d->m_pAppSupportLabel = new QLabel(this);
    d->m_pAppSupportLabel->setFixedWidth(400);

    d->m_pAppPrivacyLabel = new LinkLabel(this);
    d->m_pAppPrivacyLabel->setAlignment(Qt::AlignCenter);
    d->m_pAppPrivacyLabel->setVisible(false);

    d->adjustMiniMode();

    connect(d->m_pAppSupportLabel,&QLabel::linkActivated,this,[=](const QString url){
        QString appid = d->getDefaultAppId(MAILTYPE);
        if(appid.isEmpty())
        {
            QMessageBox msg(this);
            msg.setIcon(QMessageBox::Icon::Information);
            msg.setIconPixmap(QIcon::fromTheme("dialog-info").pixmap(24,24));
            msg.setInformativeText(tr("Your system does not have any email application installed or the default mail application is not set up"));
            msg.setText(tr("Unable to open mail application"));
            msg.exec();
        }
        else
            QDesktopServices::openUrl(QUrl(url));
    });
    changeTheme();
    connect(m_gsetting,&QGSettings::changed,this,[=](){changeTheme();});

    QString qtTransPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);

    QTranslator *trans_qt = new QTranslator(this);
    if (trans_qt->load(QLocale(), "qt", "_", qtTransPath))
       qApp->installTranslator(trans_qt);

    QTranslator *trans = new QTranslator(this);
    if(trans->load(QString(":/translations/gui_%1.qm").arg(QLocale::system().name())))
        qApp->installTranslator(trans);
    if(d->isOpenLingmo())
        d->m_pAppSupportLabel->hide();
}

KAboutDialog::~KAboutDialog()
{

}

void KAboutDialog::setAppIcon(const QIcon& icon)
{
    Q_D(KAboutDialog);
    d->m_iconName = icon.name();
    QPixmap pix(icon.pixmap(QSize(96,96)));
    d->m_pAppIconLabel->setPixmap(pix);
    d->m_pAppIconLabel->setAlignment(Qt::AlignCenter);
    update();
}


void KAboutDialog::setAppName(const QString& appName)
{
    Q_D(KAboutDialog);
    d->m_pAppNameLabel->setText(appName);
    update();
}

QString KAboutDialog::appName()
{
    Q_D(KAboutDialog);
    return d->m_pAppNameLabel->text();
}

void KAboutDialog::setAppVersion(const QString& appVersion)
{
    Q_D(KAboutDialog);
    if(!appVersion.isNull())
    {
        d->m_appVersionFlag = false;
    d->m_pAppVersionLabel->setText(appVersion);
    update();
}
}

QString KAboutDialog::appVersion()
{
    Q_D(KAboutDialog);
    return d->m_pAppVersionLabel->text();
}

void KAboutDialog::setBodyText(const QString& bodyText)
{
    Q_D(KAboutDialog);
    d->m_bodyText = bodyText;
}

QString KAboutDialog::bodyText()
{
    Q_D(KAboutDialog);
    return d->m_bodyText;
}

void KAboutDialog::setAppSupport(const QString& appSupport)
{
    Q_D(KAboutDialog);
    d->m_pAppSupportLabel->setText(appSupport);
    update();
}

QString KAboutDialog::appSupport()
{
    Q_D(KAboutDialog);
    return d->m_pAppSupportLabel->text();
}

void KAboutDialog::setBodyTextVisiable(bool flag)
{
    Q_D(KAboutDialog);
    //do nothing
}

void KAboutDialog::setAppPrivacyLabelVisible(bool flag)
{
    Q_D(KAboutDialog);
    d->m_pAppPrivacyLabel->setVisible(flag);
}

bool KAboutDialog::AppPrivacyLabelIsVisible()
{
    Q_D(KAboutDialog);
    return d->m_pAppPrivacyLabel->isVisible();
}

void KAboutDialog::changeTheme()
{
    Q_D(KAboutDialog);
    KDialog::changeTheme();

    auto icon = QIcon::fromTheme(d->m_iconName);
    if(!icon.isNull())
    {
        QPixmap pix(icon.pixmap(QSize(96,96)));
        d->m_pAppIconLabel->setPixmap(pix);
    }
    QFont font;
    font.setPixelSize(ThemeController::systemFontSize() * 1.7);
    font.setWeight(QFont::Medium);
    d->m_pAppNameLabel->setFont(font);
    QPalette palette = qApp->palette();
    if(ThemeController::themeMode() == LightTheme)
    {
       palette.setColor(QPalette::Text,QColor("#595959"));
       palette.setColor(QPalette::WindowText,QColor("#595959"));
       palette.setColor(QPalette::Base,QColor(0,0,0,0));
       d->m_pAppSupportLabel->setPalette(palette);
       d->m_pAppVersionLabel->setPalette(palette);
       d->m_pAppSupportLabel->setText(tr("Service & Support: ") +
                                              "<a href=\"mailto://support@kylinos.cn\""
                                              "style=\"color:#595959\">"
                                              "support@kylinos.cn</a>");
       palette.setColor(QPalette::WindowText,QColor("#307FF5"));
       d->m_pAppPrivacyLabel->setPalette(palette);
       d->m_pAppPrivacyLabel->setText(tr("Privacy statement"));
    }
    else
    {
        palette.setColor(QPalette::ButtonText,QColor("#D9D9D9"));
        palette.setColor(QPalette::WindowText,QColor("#D9D9D9"));
        d->m_pAppSupportLabel->setPalette(palette);
        d->m_pAppVersionLabel->setPalette(palette);
        d->m_pAppSupportLabel->setText(tr("Service & Support: ") +
                                             "<a href=\"mailto://support@kylinos.cn\""
                                             "style=\"color:#D9D9D9\">"
                                             "support@kylinos.cn</a>");
        palette.setColor(QPalette::WindowText,QColor("#307FF5"));
        d->m_pAppPrivacyLabel->setPalette(palette);
        d->m_pAppPrivacyLabel->setText(tr("Privacy statement"));
    }
}

void KAboutDialog::paintEvent(QPaintEvent *paintEvent)
{
    Q_D(KAboutDialog);

    if(d->m_appVersionFlag)
        d->updateAppVersionText();
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

KAboutDialogPrivate::KAboutDialogPrivate(KAboutDialog *parent)
    :q_ptr(parent),m_appVersionFlag(true)
{
    setParent(parent);
}

void KAboutDialogPrivate::adjustMiniMode()
{
    Q_Q(KAboutDialog);
    if(q->mainWidget()->layout())
        delete q->mainWidget()->layout();
    m_pAppSupportLabel->setAlignment(Qt::AlignCenter);
    QVBoxLayout *vBoxLayout = new QVBoxLayout();
    vBoxLayout->setSpacing(0);
    vBoxLayout->addSpacing(20);
    vBoxLayout->addWidget(m_pAppIconLabel);
    vBoxLayout->addSpacing(24);
    vBoxLayout->addWidget(m_pAppNameLabel);
    vBoxLayout->addSpacing(16);
    vBoxLayout->addWidget(m_pAppVersionLabel);
    vBoxLayout->addSpacing(16);
    vBoxLayout->addWidget(m_pAppSupportLabel);
    vBoxLayout->addSpacing(16);
    vBoxLayout->addWidget(m_pAppPrivacyLabel);
    vBoxLayout->setContentsMargins(25,0,25,0);
    vBoxLayout->addSpacing(35);
    q->mainWidget()->setLayout(vBoxLayout);
}

QString KAboutDialogPrivate::getDefaultAppId(const char *contentType)
{
    QString localfile = QDir::homePath() + LOCAL_CONFIG_DIR + "mimeapps.list";
    QString systemfile = SYSTEM_CONFIG_DIR + QString("lingmo-mimeapps.list");
    if (QFile(localfile).exists()) {
        QSettings* mimeappFile = new QSettings(localfile, QSettings::IniFormat);
        mimeappFile->setIniCodec("utf-8");
        QString str = mimeappFile->value(QString("Default Applications/%1").arg(contentType)).toString();
        if (!str.isEmpty()) {
            if (QFile(SYSTEM_CONFIG_DIR +str).exists()) {
                return str;
            } else {
                return QString("");
            }
        }
        delete mimeappFile;
        mimeappFile = nullptr;
    }
    if (QFile(systemfile).exists()) {
        QSettings* mimeappFile = new QSettings(systemfile, QSettings::IniFormat);
        mimeappFile->setIniCodec("utf-8");
        QString str = mimeappFile->value(QString("Default Applications/%1").arg(contentType)).toString();
        if (!str.isEmpty()) {
            if (QFile(SYSTEM_CONFIG_DIR +str).exists()) {
                return str;
            } else {
                return QString("");
            }
        }
        delete mimeappFile;
        mimeappFile = nullptr;
    }
    return QString("");
}

bool KAboutDialogPrivate::isOpenLingmo()
{
    QString filename = "/etc/os-release";
    QSettings osSettings(filename, QSettings::IniFormat);

    QString versionID = osSettings.value("ID").toString();

    if (versionID.compare("lingmo", Qt::CaseInsensitive)) {
        return false;
    }
    return true;
}

void KAboutDialogPrivate::updateAppVersionText()
{
    Q_Q(KAboutDialog);
    QProcess process;
    if( QCoreApplication::applicationFilePath().contains(QApplication::applicationName()))
    {
        QStringList list;
        QString str("dpkg -l | grep "+QApplication::applicationName() +" | awk '{print $3}'");
        list<<"-c"<<str;
        process.start("/bin/bash",list);
        process.waitForFinished();
        QString result(tr("version :"));
        QString string;
        string=process.readAllStandardOutput().data();
        if(string =="" || string.isNull())
        {
            result +=QString(tr("Version number not found"));
        }
        else
{
            result += string;
            result.remove("\n"); //去掉 \n 字符
    }
        m_pAppVersionLabel->setText(result);
    }
    process.close();
}

LinkLabel::LinkLabel(QWidget *parent)
    :QLabel(parent)
{

}

void LinkLabel::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        QProcess *process = new QProcess(this);
        QStringList l;
        l <<"-A"<<"lingmo-os";
        process->start("lingmo-user-guide",l);
    }
    QLabel::mousePressEvent(event);
}

}

#include "kaboutdialog.moc"
#include "moc_kaboutdialog.cpp"
