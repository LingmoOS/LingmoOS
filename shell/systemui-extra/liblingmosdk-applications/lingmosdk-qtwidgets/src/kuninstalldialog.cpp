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

#include <QLocale>
#include <QFont>
#include <QVariant>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>
#include <QStandardPaths>
#include <QByteArray>
#include <QFileDialog>
#include "kuninstalldialog.h"

namespace kdk
{
class KUninstallDialogPrivate:public QObject
{
    Q_DECLARE_PUBLIC(KUninstallDialog)
    Q_OBJECT

public:
    KUninstallDialogPrivate(KUninstallDialog*parent);
    QString      getIconPath(QString appName);
    QString      setLabelStringBody(QString text, QLabel *lable);
    QString      getAppCnName(QString appName);
    void         resetAppInfo(QString appName, QString appVersion);
    void         fontGsettingInit();
    void         setLabelText();

private:
    KUninstallDialog* q_ptr;
    QString      m_appName;
    QString      m_appVersion;
    QVBoxLayout  *debInfoLayout;
    QWidget      *IconAndAppNameWidget;
    QHBoxLayout  *IconAndAppNameLayout;
    QWidget      *debNameAndVersionWidget;
    QHBoxLayout  *debNameAndVersionLayout;
    QWidget      *nameVersionWidget;
    QVBoxLayout  *nameVersionVLayout;
    QGSettings   *m_pGsettingFontSize;
    QLabel       *debIconLabel;
    QLabel       *debAppNameLabel;
    QLabel       *debNameLabel;
    QLabel       *debVersionLabel;
    QPushButton * m_pUninstallBtn;
};
KUninstallDialog::~KUninstallDialog()
{

}

QLabel *KUninstallDialog::debAppNameLabel()
{
    Q_D(KUninstallDialog);
    return d->debAppNameLabel;
}

QLabel *KUninstallDialog::debNameLabel()
{
    Q_D(KUninstallDialog);
    return d->debNameLabel;
}

QLabel *KUninstallDialog::debIconLabel()
{
    Q_D(KUninstallDialog);
    return d->debIconLabel;
}

QLabel *KUninstallDialog::debVersionLabel()
{
    Q_D(KUninstallDialog);
    return d->debVersionLabel;
}

QPushButton *KUninstallDialog::uninstallButtton()
{
    Q_D(KUninstallDialog);
    return d->m_pUninstallBtn;
}

void KUninstallDialog::changeTheme()
{
    KDialog::changeTheme();
    if(ThemeController::themeMode() == LightTheme)
    {
        //to do 还没有具体的设计图 只有截图
    }
    else
    {
        //to do 还没有具体的设计图 只有截图
    }
}

KUninstallDialog::KUninstallDialog(QString appName, QString appVersion, QWidget *parent)
    :KDialog(parent),
      d_ptr(new KUninstallDialogPrivate(this))
{
    Q_D(KUninstallDialog);
    d->m_appName = appName;
    d->m_appVersion = appVersion;

    setWindowTitle(tr("uninstall"));
    this->setFixedSize(550,450);

    d->debInfoLayout = new QVBoxLayout();
    d->debInfoLayout->setContentsMargins(0, 0, 0, 0);

    d->IconAndAppNameWidget = new QWidget();
    d->IconAndAppNameWidget->setContentsMargins(0, 0, 0, 0);

    d->IconAndAppNameLayout = new QHBoxLayout();
    d->IconAndAppNameLayout->setContentsMargins(0,0,0,0);
    d->IconAndAppNameLayout->setSpacing(0);
    d->debIconLabel = new QLabel();
    d->debIconLabel->setFixedSize(48, 48);

    /*先判断主题中是否有icon*/
    if (QIcon::fromTheme(d->m_appName).isNull())
    {
        QPixmap pixmap = QPixmap(d->getIconPath(d->m_appName));
        pixmap.scaled(QSize(48,48), Qt::KeepAspectRatio);
        d->debIconLabel->setPixmap(QIcon(QPixmap(d->getIconPath(d->m_appName))).pixmap(QSize(48,48)));
        setWindowIcon(QIcon(QPixmap(d->getIconPath(d->m_appName))).pixmap(QSize(24,24)));
    }
    else
    {
        d->debIconLabel->setPixmap(QIcon::fromTheme(d->m_appName).pixmap(QSize(48,48)));
        setWindowIcon(QIcon::fromTheme(d->m_appName).pixmap(QSize(24,24)));
    }
    d->debIconLabel->adjustSize();
    d->fontGsettingInit();
    d->debAppNameLabel = new QLabel();
    QFont font2 =  d->debAppNameLabel->font();
    font2.setPixelSize(d->m_pGsettingFontSize->get(GSETTING_FONT_KEY).toInt() * 2.5);
    d->debAppNameLabel->setFont(font2);
    d->debAppNameLabel->setFixedWidth(331);
    QLocale locale;
    if (locale.language() == QLocale::Chinese) {
        if (d->getAppCnName(d->m_appName).isNull()) {
            d->debAppNameLabel->setText(d->setLabelStringBody(d->m_appName, d->debAppNameLabel));
        }
        else {
            d->debAppNameLabel->setText(d->setLabelStringBody(d->getAppCnName(d->m_appName), d->debAppNameLabel));
        }
    } else {
        d->debAppNameLabel->setText(d->setLabelStringBody(d->m_appName, d->debAppNameLabel));
    }
    d->debAppNameLabel->adjustSize();

    d->IconAndAppNameLayout->addItem(new QSpacerItem(124, 10, QSizePolicy::Fixed));
    d->IconAndAppNameLayout->addWidget(d->debIconLabel);
    d->IconAndAppNameLayout->addItem(new QSpacerItem(12, 10, QSizePolicy::Fixed));
    d->IconAndAppNameLayout->addWidget(d->debAppNameLabel);
    d->IconAndAppNameLayout->addItem(new QSpacerItem(400, 10, QSizePolicy::Expanding));
    d->IconAndAppNameWidget->setLayout(d->IconAndAppNameLayout);

    d->debNameAndVersionWidget = new QWidget();
    d->debNameAndVersionLayout = new QHBoxLayout();
    d->debNameAndVersionLayout->setContentsMargins(0,0,0,0);
    d->debNameAndVersionLayout->setSpacing(0);

    d->debNameLabel = new QLabel();
    QString debName = tr("deb name:");
    debName.append(appName);
    d->debNameLabel->setText(debName);
    d->debNameLabel->adjustSize();
    d->debVersionLabel = new QLabel();
    QString debVersion = tr("deb version:");
    debVersion.append(appVersion);
    d->debVersionLabel->setText(debVersion);
    d->debVersionLabel->adjustSize();
    if (d->debVersionLabel->width() > 329) {
        d->debVersionLabel->setText(debVersion.left(20) + "...");
        d->debVersionLabel->setToolTip(debVersion);
    }
    d->nameVersionWidget = new QWidget();
    d->nameVersionVLayout = new QVBoxLayout();
    d->nameVersionVLayout->setContentsMargins(0, 0, 0, 0);
    d->nameVersionVLayout->addWidget(d->debNameLabel);
    d->nameVersionVLayout->addWidget(d->debVersionLabel);
    d->nameVersionVLayout->setSpacing(0);
    d->nameVersionWidget->setLayout(d->nameVersionVLayout);

    d->debNameAndVersionLayout->addItem(new QSpacerItem(184, 10, QSizePolicy::Fixed));
    d->debNameAndVersionLayout->addWidget(d->nameVersionWidget);
    d->debNameAndVersionLayout->addItem(new QSpacerItem(330, 10, QSizePolicy::Expanding));
    d->debNameAndVersionWidget->setLayout(d->debNameAndVersionLayout);

    d->debInfoLayout->addStretch();
    d->debInfoLayout->addWidget(d->IconAndAppNameWidget);
    d->debInfoLayout->addWidget(d->debNameAndVersionWidget);
    d->debInfoLayout->addSpacing(25);

    d->m_pUninstallBtn = new QPushButton(tr("uninstall"),this);
    d->m_pUninstallBtn->setFixedSize(122,40);
    QWidget *pBtnWidget = new QWidget();
    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->addItem(new QSpacerItem(184, 10, QSizePolicy::Fixed));
    hLayout->addWidget(d->m_pUninstallBtn);
    hLayout->addStretch();
    pBtnWidget->setLayout(hLayout);
    d->debInfoLayout->addWidget(pBtnWidget);
    d->debInfoLayout->addStretch();
    mainWidget()->setLayout(d->debInfoLayout);
    changeTheme();
}

KUninstallDialogPrivate::KUninstallDialogPrivate(KUninstallDialog *parent)
    :q_ptr(parent)
{
    setParent(parent);
}

QString KUninstallDialogPrivate::getIconPath(QString appName)
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

QString KUninstallDialogPrivate::setLabelStringBody(QString text, QLabel *lable)
{
    QFontMetrics fontMetrics(lable->font());
    int LableWidth = lable->width();
    int fontSize = fontMetrics.width(text);
    QString formatBody = text;
    if(fontSize > (LableWidth - 10)) {
        formatBody = fontMetrics.elidedText(formatBody, Qt::ElideRight, LableWidth - 10);
        return formatBody;
    }
    return formatBody;
}

QString KUninstallDialogPrivate::getAppCnName(QString appName)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.cache/uksc/uksc.db");
    if (!db.open()) {
        return nullptr;
    }
    QSqlQuery query;
    query.exec(QString("SELECT * FROM application WHERE app_name IS '%1'").arg(appName));	//执行
    while(query.next()) {
        return query.value(4).toString();
    }
    db.close();
    return nullptr;
}

void KUninstallDialogPrivate::resetAppInfo(QString appName, QString appVersion)
{
    Q_Q(KUninstallDialog);

    m_appName = appName;
    m_appVersion = appVersion;
    /*先判断主题中是否有icon*/
    if (QIcon::fromTheme(m_appName).isNull())
    {
        QPixmap pixmap = QPixmap(getIconPath(m_appName));
        pixmap.scaled(QSize(48,48), Qt::KeepAspectRatio);
        debIconLabel->setPixmap(QIcon(QPixmap(getIconPath(m_appName))).pixmap(QSize(48,48)));
    } else
    {
        debIconLabel->setPixmap(QIcon::fromTheme(m_appName).pixmap(QSize(48,48)));
    }

    if (getAppCnName(m_appName).isNull())
        debAppNameLabel->setText(m_appName);
     else
        debAppNameLabel->setText(getAppCnName(m_appName));

    QString debName = tr("deb name:");
    debName.append(appName);
    debNameLabel->setText(debName);

    QString debVersion = tr("deb version:");
    debVersion.append(appVersion);
    debVersionLabel->setText(debVersion);
    debVersionLabel->adjustSize();
    if (debVersionLabel->width() > 329)
    {
        debVersionLabel->setText(debVersion.left(20) + "...");
        debVersionLabel->setToolTip(debVersion);
    }
}

void KUninstallDialogPrivate::fontGsettingInit()
{
    Q_Q(KUninstallDialog);
    const QByteArray style_id(ORG_LINGMO_STYLE_FONT);
    m_pGsettingFontSize = new QGSettings(style_id);
    connect(m_pGsettingFontSize, &QGSettings::changed, this, [=] (const QString &key)
    {
        if (key == GSETTING_FONT_KEY)
        {
            setLabelText();
        }
    });
}

void KUninstallDialogPrivate::setLabelText()
{
    Q_Q(KUninstallDialog);
    QFont font;
    font.setPixelSize(m_pGsettingFontSize->get(GSETTING_FONT_KEY).toDouble() * 2.5);
    debAppNameLabel->setFont(font);
    QLocale locale;
    if (locale.language() == QLocale::Chinese)
    {
        if (getAppCnName(m_appName).isNull())
        {
            q->setWindowTitle(setLabelStringBody(m_appName, debAppNameLabel));
        } else
        {
            q->setWindowTitle(setLabelStringBody(getAppCnName(m_appName), debAppNameLabel));
        }
    } else
    {
        q->setWindowTitle(setLabelStringBody(m_appName, debAppNameLabel));
    }
}

}

#include "kuninstalldialog.moc";
#include "moc_kuninstalldialog.cpp";
