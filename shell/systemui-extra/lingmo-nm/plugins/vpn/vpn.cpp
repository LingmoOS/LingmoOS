/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "vpn.h"
#include "ui_vpn.h"

#include <QProcess>
#include <QMouseEvent>
#include <QDebug>
#include <QtDBus>
#include <QDBusArgument>

#define ACTIVATING   1
#define ACTIVATED    2
#define DEACTIVATING 3
#define DEACTIVATED  4

#define LABEL_RECT 17, 0, 105, 23
#define CONTENTS_MARGINS 0, 0, 0, 0
#define ITEM_MARGINS 16, 0, 16, 0
#define FRAME_MIN_SIZE 550, 60
#define FRAME_MAX_SIZE 16777215, 16777215
#define CONTECT_FRAME_MAX_SIZE 16777215, 60
#define HINT_TEXT_MARGINS 8, 0, 0, 0
#define FRAME_MIN_SIZE 550, 60
#define LABLE_MIN_WIDTH 188
#define COMBOBOX_MIN_WIDTH 200
#define LINE_MAX_SIZE 16777215, 1
#define LINE_MIN_SIZE 0, 1
#define ICON_SIZE   24,24
#define PASSWORD_FRAME_MIN_HIGHT 60
#define PASSWORD_FRAME_FIX_HIGHT 80
#define PASSWORD_FRAME_MIN_SIZE 550, 60
#define PASSWORD_FRAME_MAX_SIZE 16777215, 86
#define PASSWORD_ITEM_MARGINS 16, 12, 16, 14

#define KVpnSymbolic "lingmo-vpn-symbolic"

#define LINGMO_APP_MANAGER_NAME "com.lingmo.AppManager"
#define LINGMO_APP_MANAGER_PATH "/com/lingmo/AppManager"
#define LINGMO_APP_MANAGER_INTERFACE "com.lingmo.AppManager"

const QString VISIBLE = "visible";
const QByteArray GSETTINGS_SCHEMA = "org.lingmo.lingmo-nm.vpnicon";

Vpn::Vpn() : m_firstLoad(true)
{
    QTranslator* translator = new QTranslator(this);
    translator->load("/usr/share/lingmo-nm/vpn/" + QLocale::system().name());
    QApplication::installTranslator(translator);

    m_pluginName = tr("VPN");
    m_pluginType = NETWORK;
}

Vpn::~Vpn()
{
    if (!m_firstLoad) {
        delete ui;
        ui = nullptr;
        delete m_interface;
        delete m_switchGsettings;
    }
}

QString Vpn::plugini18nName(){
    return m_pluginName;
}

int Vpn::pluginTypes(){
    return m_pluginType;
}

QWidget *Vpn::pluginUi(){
    if (m_firstLoad) {
        m_firstLoad = false;
        ui = new Ui::Vpn;
        m_pluginWidget = new QWidget;
        m_pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
        ui->setupUi(m_pluginWidget);

        qDBusRegisterMetaType<QVector<QStringList>>();
        m_interface = new QDBusInterface("com.lingmo.lingmovpn",
                                         "/com/lingmo/lingmovpn",
                                         "com.lingmo.lingmovpn",
                                         QDBusConnection::sessionBus());
        if(!m_interface->isValid()) {
            qWarning() << qPrintable(QDBusConnection::sessionBus().lastError().message());
        }

        initComponent();
        initConnect();
        initNet();
        initSearchText();
    }
    return m_pluginWidget;
}

const QString Vpn::name() const {

    return QStringLiteral("Vpn");
}

bool Vpn::isShowOnHomePage() const
{
    return true;
}

QIcon Vpn::icon() const
{
    return QIcon::fromTheme("lingmo-vpn-symbolic");
}

bool Vpn::isEnable() const
{
    return true;
}

QString Vpn::translationPath() const
{
    return "/usr/share/lingmo-nm/vpn/%1.ts";
}

void Vpn::initComponent(){
    //在任务栏上显示图标
    //显示已连接时间
    m_topFrame = new QFrame(m_pluginWidget);
    m_topFrame->setMinimumSize(FRAME_MIN_SIZE);
    m_topFrame->setMaximumSize(FRAME_MAX_SIZE);
    m_topFrame->setFrameShape(QFrame::Box);

    QVBoxLayout *hotspotLyt = new QVBoxLayout(m_pluginWidget);
    hotspotLyt->setContentsMargins(0, 0, 0, 0);
    m_topFrame->setLayout(hotspotLyt);

    m_showFrame = new QFrame(m_topFrame);
    m_showFrame->setFrameShape(QFrame::Shape::NoFrame);
    m_showFrame->setMinimumSize(FRAME_MIN_SIZE);
    m_showFrame->setMaximumSize(CONTECT_FRAME_MAX_SIZE);
    QHBoxLayout *showLayout = new QHBoxLayout(m_showFrame);
    m_showLabel = new QLabel(tr("Show on Taskbar"), m_showFrame);
    m_showLabel->setMinimumWidth(LABLE_MIN_WIDTH);
    m_showBtn = new KSwitchButton(m_showFrame);
    showLayout->setContentsMargins(ITEM_MARGINS);
    showLayout->addWidget(m_showLabel);
    showLayout->addStretch();
    showLayout->addWidget(m_showBtn);

    m_showFrame->setLayout(showLayout);

//    m_Line = myLine();

//    m_timeFrame = new QFrame(m_topFrame);
//    m_timeFrame->setFrameShape(QFrame::Shape::NoFrame);
//    m_timeFrame->setMinimumSize(FRAME_MIN_SIZE);
//    m_timeFrame->setMaximumSize(CONTECT_FRAME_MAX_SIZE);
//    QHBoxLayout *timeLayout = new QHBoxLayout(m_timeFrame);
//    m_timeLabel = new QLabel(tr("Open"), m_timeFrame);
//    m_timeLabel->setMinimumWidth(LABLE_MIN_WIDTH);
//    m_timeBtn = new KSwitchButton(m_timeFrame);
//    timeLayout->setContentsMargins(ITEM_MARGINS);
//    timeLayout->addWidget(m_timeLabel);
//    timeLayout->addStretch();
//    timeLayout->addWidget(m_timeBtn);

//    m_timeFrame->setLayout(timeLayout);

    hotspotLyt->addWidget(m_showFrame);
//    hotspotLyt->addWidget(m_Line);
//    hotspotLyt->addWidget(m_timeFrame);
    hotspotLyt->setSpacing(0);

    //列表

    m_listFrame = new ItemFrame(m_pluginWidget);

    ui->verticalLayout_4->addWidget(m_topFrame);
    ui->verticalLayout_3->addWidget(m_listFrame);

    connect(m_listFrame->m_addVpnWidget, &AddNetBtn::clicked, this, [=]() {
        UkccCommon::buriedSettings(QString("VPN"), QString("Add VPN"), QString("clicked"));
        runExternalApp();
    });

    if (QGSettings::isSchemaInstalled(GSETTINGS_SCHEMA)) {
        m_switchGsettings = new QGSettings(GSETTINGS_SCHEMA);
        setShowSwitchStatus();

        connect(m_switchGsettings, &QGSettings::changed, this, [=] (const QString &key) {
            if (key == VISIBLE) {
                setShowSwitchStatus();
            }
        });
    } else {
        m_showBtn->setChecked(false);
        m_showBtn->setCheckable(false);
        qDebug()<<"[Vpn] org.lingmo.lingmo-nm.visible is not installed!";
    }

    connect(m_showBtn, &KSwitchButton::stateChanged, this, [=](bool state){
        if (m_switchGsettings != nullptr) {
            m_switchGsettings->set(VISIBLE, state);
        }
    });

    connect(m_showBtn, &KSwitchButton::clicked, this, [=](bool checked){
        UkccCommon::buriedSettings(QString("VPN"), QString("Show on Taskbar"),  QString("settings"), checked ? "true":"false");
    });

//    connect(m_timeBtn, &KSwitchButton::stateChanged, this, [=](bool state){
//        if (m_switchGsettings != nullptr) {
//            m_switchGsettings->set(VISIBLE, state);
//        }
//    });
    ui->pushButton->hide();
}

void Vpn::initConnect()
{
    connect(m_interface, SIGNAL(vpnAdd(QStringList)), this, SLOT(onVpnAdd(QStringList)));
    connect(m_interface, SIGNAL(vpnRemove(QString)), this, SLOT(onVpnRemove(QString)));
    connect(m_interface, SIGNAL(vpnUpdate(QStringList)), this, SLOT(onVpnUpdate(QStringList)));
    connect(m_interface, SIGNAL(vpnActiveConnectionStateChanged(QString, int)),
                this, SLOT(onVpnActiveConnectionStateChanged(QString, int)));
}

//初始化列表
void Vpn::initNet()
{
    qDebug() << "[Vpn]initNet";
    if (!m_interface->isValid()) {
        return;
    }
    QDBusMessage result = m_interface->call(QStringLiteral("getVirtualList"));
    if(result.type() == QDBusMessage::ErrorMessage)
    {
        qWarning() << "getVirtualList error:" << result.errorMessage();
        return;
    }
    auto dbusArg =  result.arguments().at(0).value<QDBusArgument>();
    QVector<QStringList> variantList;
    dbusArg >> variantList;
    if (variantList.size() == 0) {
        qDebug() << "[Vpn]initNet list empty";
        return;
    }

    for (int i = 0; i < variantList.size(); ++i) {
        QStringList vpnInfo = variantList.at(i);
        addOneVirtualItem(vpnInfo);
    }
    return;
}

void Vpn::setShowSwitchStatus()
{
    if (QGSettings::isSchemaInstalled(GSETTINGS_SCHEMA)) {
        bool status = m_switchGsettings->get(VISIBLE).toBool();
        m_showBtn->setChecked(status);
    } else {
        qDebug()<<"[Vpn] org.lingmo.lingmo-nm.switch is not installed!";
    }
}

void Vpn::initSearchText()
{
    //~ contents_path /Vpn/Show on Taskbar
    tr("Show on Taskbar");
    //~ contents_path /Vpn/Add VPN
    tr("Add VPN");
}

void Vpn::runExternalApp(){
//    QString cmd = "nm-connection-editor";
//    QProcess process(this);
//    process.startDetached(cmd);

    if (m_interface->isValid()) {
        m_interface->call(QStringLiteral("showVpnAddWidget"));
    }
}

QFrame* Vpn::myLine()
{
    QFrame *line = new QFrame(m_pluginWidget);
    line->setMinimumSize(QSize(LINE_MIN_SIZE));
    line->setMaximumSize(QSize(LINE_MAX_SIZE));
    line->setLineWidth(0);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    return line;
}

//刪除
void Vpn::deleteVpn(QString uuid)
{
    m_interface->call(QStringLiteral("deleteVpn"), uuid);
}

//激活
void Vpn::activeConnect(QString uuid) {
    m_interface->call(QStringLiteral("activateVpn"), uuid);
}

//详情页
void Vpn::showDetailPage(QString uuid) {
    m_interface->call(QStringLiteral("showDetailPage"), uuid);
}

//断开
void Vpn::deActiveConnect(QString uuid) {
    m_interface->call(QStringLiteral("deactivateVpn"), uuid);
}


//增加一项
void Vpn::addOneVirtualItem(QStringList infoList)
{
    if (infoList.size() < 4) {
        qDebug() << "[Vpn]QStringList size less";
        return;
    }

    if (m_listFrame->m_itemMap.contains(infoList.at(1))) {
        qDebug() << "[Vpn]Already exist a virtual " << infoList.at(1);
        return;
    }

    qDebug() << "[Vpn]addOneVitualItem" << infoList.at(0) << infoList.at(3) ;
    QString connName = infoList.at(0);
    QString connUuid = infoList.at(1);
    QString connDbusPath = infoList.at(2);
    int status = infoList.at(3).toInt(); //1-连接中 2-已连接 3-断开中 4-已断开
    VpnItem * item = new VpnItem(m_pluginWidget);

    QIcon searchIcon = QIcon::fromTheme(KVpnSymbolic);
    item->m_iconLabel->setPixmap(searchIcon.pixmap(searchIcon.actualSize(QSize(ICON_SIZE))));
    item->m_titileLabel->setText(connName);

    item->m_uuid = connUuid;
    item->m_dbusPath = connDbusPath;

    if (status == 1 || status == 3) {
        item->startLoading();
    }

    connect(item->m_infoLabel, &GrayInfoButton::clicked, this, [=]{
        showDetailPage(item->m_uuid);
    });

    item->m_isAcitve = (status == 2);
    item->setConnectActionText(item->m_isAcitve);

    if (item->m_isAcitve) {
        item->m_statusLabel->setText(tr("connected"));
    } else {
        item->m_statusLabel->setText(tr("not connected"));
    }

    connect(item, &QPushButton::clicked, this, [=] {
        if (item->m_isAcitve || item->m_loading) {
            deActiveConnect(item->m_uuid);
        } else {
            activeConnect(item->m_uuid);
        }
    });

    connect(item, &VpnItem::connectActionTriggered, this, [=] {
        activeConnect(item->m_uuid);
    });
    connect(item, &VpnItem::disconnectActionTriggered, this, [=] {
        deActiveConnect(item->m_uuid);
    });
    connect(item, &VpnItem::deleteActionTriggered, this, [=] {
        deleteVpn(item->m_uuid);
    });

    //记录到deviceFrame的m_itemMap中
    m_listFrame->m_itemMap.insert(connUuid, item);
    int index = getInsertPos(connName);
    qDebug()<<"[Vpn]addOneVirtualItem " << connName << " at pos:" << index;
    m_listFrame->m_vpnVLayout->insertWidget(index, item);
}

void Vpn::removeOneVirtualItem(QString dbusPath)
{
   qDebug()<<"[Vpn]vpn remove dbus path:" << dbusPath;

   QMap<QString, VpnItem *>::iterator itemIter;
   for (itemIter = m_listFrame->m_itemMap.begin(); itemIter != m_listFrame->m_itemMap.end(); itemIter++) {
       if (itemIter.value()->m_dbusPath == dbusPath) {
           qDebug()<<"[Vpn]vpn remove " << dbusPath << " find in " << itemIter.value()->m_titileLabel->text();
           QString key = itemIter.key();
           m_listFrame->m_vpnVLayout->removeWidget(itemIter.value());
           delete itemIter.value();
           m_listFrame->m_itemMap.remove(key);
           break;
       }
   }
}

//增加
void Vpn::onVpnAdd(QStringList infoList)
{
    addOneVirtualItem(infoList);
}

//移出
void Vpn::onVpnRemove(QString path)
{
    removeOneVirtualItem(path);
}

//名称变化
void Vpn::onVpnUpdate(QStringList info)
{
    if (m_listFrame->m_itemMap.contains(info.at(1))) {
        qDebug() << "[Vpn]" << m_listFrame->m_itemMap[info.at(1)]->m_titileLabel->text() << "change to" << info.at(0);
        if (m_listFrame->m_itemMap[info.at(1)]->m_titileLabel->text() != info.at(0)) {
            m_listFrame->m_itemMap[info.at(1)]->m_titileLabel->setText(info.at(0));
        }
    }
}

void Vpn::onVpnActiveConnectionStateChanged(QString uuid, int status)
{
    if (uuid.isEmpty()) {
        qDebug() << "[Vpn]onActiveConnectionChanged but uuid is empty";
        return;
    }
    qDebug() << "[Vpn]onActiveConnectionChanged " << uuid << status;
    VpnItem * item= nullptr;

    if (m_listFrame->m_itemMap.contains(uuid)) {
        item = m_listFrame->m_itemMap[uuid];
        if (status == ACTIVATED) {
            //为已连接则放到第一个
            m_listFrame->m_vpnVLayout->removeWidget(item);
            m_listFrame->m_vpnVLayout->insertWidget(0,item);
        } else if (status == DEACTIVATED) {
            //为断开则重新插入
            int index = getInsertPos(item->m_titileLabel->text());
            qDebug() << "[Vpn]reinsert" << item->m_titileLabel->text() << "pos" << index  << "because status changes to deactive";
            m_listFrame->m_vpnVLayout->removeWidget(item);
            m_listFrame->m_vpnVLayout->insertWidget(index,item);
        }
        itemActiveConnectionStatusChanged(item, status);
    }
}

void Vpn::itemActiveConnectionStatusChanged(VpnItem *item, int status)
{
//    QString iconPath = NoNetSymbolic;
    if (status == ACTIVATING) {
        item->startLoading();
    } else if (status == ACTIVATED) {
        item->stopLoading();
//        iconPath = KLanSymbolic;
        item->m_statusLabel->clear();
        item->m_statusLabel->setMinimumSize(36,36);
        item->m_statusLabel->setMaximumSize(16777215,16777215);
        item->m_statusLabel->setText(tr("connected"));
        item->m_isAcitve = true;
    } else if (status == DEACTIVATING) {
        item->startLoading();
    } else {
        item->stopLoading();
        item->m_statusLabel->setMinimumSize(36,36);
        item->m_statusLabel->setMaximumSize(16777215,16777215);
        item->m_statusLabel->clear();
        item->m_isAcitve = false;
        item->m_statusLabel->setText(tr("not connected"));
    }
    item->setConnectActionText(item->m_isAcitve);
}

int Vpn::getInsertPos(QString connName)
{
    qDebug() << "[Vpn]getInsertPos" << connName;
    int index = 0;
    if(!m_interface->isValid()) {
        index = 0;
    } else {
        QDBusMessage result = m_interface->call(QStringLiteral("getVirtualList"));
        if(result.type() == QDBusMessage::ErrorMessage)
        {
            qWarning() << "getVirtualList error:" << result.errorMessage();
            return 0;
        }
        auto dbusArg =  result.arguments().at(0).value<QDBusArgument>();
        QVector<QStringList> variantList;
        dbusArg >> variantList;
        if (variantList.isEmpty()) {
            qDebug() << "[Vpn] virtualList is empty, getInsertPos return 0";
            return 0;
        }
        for (int i = 0; i < variantList.size(); ++i ) {
            if (variantList.at(i).at(0) == connName) {
                qDebug() << "pos in lingmo-nm is " << i;
                index = i;
                break;
            }
        }
        if (variantList.at(0).size() == 1) {
            index--;
        }
    }
    return index;
}
