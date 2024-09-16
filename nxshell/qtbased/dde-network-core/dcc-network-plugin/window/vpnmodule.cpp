// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "controllitemsmodel.h"
#include "vpnmodule.h"
#include "editpage/connectionvpneditpage.h"
#include "widgets/floatingbutton.h"

#include <widgets/switchwidget.h>
#include <widgets/widgetmodule.h>

#include <DFloatingButton>
#include <DFontSizeManager>
#include <DListView>
#include <DSwitchButton>

#include <QHBoxLayout>
#include <QApplication>
#include <QFileDialog>
#include <DDialog>
#include <QStandardPaths>
#include <QProcess>
#include <QTimer>

#include <networkcontroller.h>
#include <vpncontroller.h>

#include <networkmanagerqt/settings.h>

using namespace dde::network;
using namespace DCC_NAMESPACE;
DWIDGET_USE_NAMESPACE

QString vpnConfigType(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        return QString();

    const QString content = f.readAll();
    f.close();

    if (content.contains("openconnect"))
        return "openconnect";
    if (content.contains("l2tp"))
        return "l2tp";
    if (content.startsWith("[main]"))
        return "vpnc";

    return "openvpn";
}

VPNModule::VPNModule(QObject *parent)
    : PageModule("networkVpn", tr("VPN"), tr("VPN"), QIcon::fromTheme("dcc_vpn"), parent)
{
    m_modules.append(new WidgetModule<SwitchWidget>("wired_adapter", tr("Wired Network Adapter"), [](SwitchWidget *vpnSwitch) {
        QLabel *lblTitle = new QLabel(tr("VPN Status"));
        DFontSizeManager::instance()->bind(lblTitle, DFontSizeManager::T5, QFont::DemiBold);
        vpnSwitch->setLeftWidget(lblTitle);
        vpnSwitch->switchButton()->setAccessibleName(lblTitle->text());
        vpnSwitch->setChecked(NetworkController::instance()->vpnController()->enabled());
        connect(vpnSwitch, &SwitchWidget::checkedChanged, NetworkController::instance()->vpnController(), &VPNController::setEnabled);
        connect(NetworkController::instance()->vpnController(), &VPNController::enableChanged, vpnSwitch, [vpnSwitch](const bool enabled) {
            vpnSwitch->blockSignals(true);
            vpnSwitch->setChecked(enabled);
            vpnSwitch->blockSignals(false);
        });
        auto updateVisible = [vpnSwitch]() {
            bool visible = !NetworkController::instance()->vpnController()->items().isEmpty();
            vpnSwitch->setVisible(visible);
        };
        updateVisible();
        connect(NetworkController::instance()->vpnController(), &VPNController::itemAdded, vpnSwitch, updateVisible);
        connect(NetworkController::instance()->vpnController(), &VPNController::itemRemoved, vpnSwitch, updateVisible);
    }));
    m_modules.append(new WidgetModule<DListView>("vpnlist", QString(), this, &VPNModule::initVPNList));
    ModuleObject *extraCreate = new WidgetModule<FloatingButton>("addWired", tr("Add Network Connection"), [this](FloatingButton *createVpnBtn) {
        createVpnBtn->setIcon(DStyle::StandardPixmap::SP_IncreaseElement);
        createVpnBtn->setToolTip(tr("Create VPN"));
        createVpnBtn->setAccessibleName(tr("Create VPN"));
        connect(createVpnBtn, &DFloatingButton::clicked, this, [this]() {
            editConnection(nullptr, qobject_cast<QWidget *>(sender()));
        });
    });
    extraCreate->setExtra();
    m_modules.append(extraCreate);
    ModuleObject *extraImport = new WidgetModule<FloatingButton>("addWired", tr("Add Network Connection"), [this](FloatingButton *importVpnBtn) {
        importVpnBtn->setIcon(QIcon::fromTheme("dcc_vpn_import"));
        importVpnBtn->setToolTip(tr("Import VPN"));
        importVpnBtn->setAccessibleName(tr("Import VPN"));
        connect(importVpnBtn, &DFloatingButton::clicked, this, &VPNModule::importVPN);
    });
    extraImport->setExtra();
    m_modules.append(extraImport);
    for (auto &it : m_modules) {
        appendChild(it);
    }
}

void VPNModule::initVPNList(DListView *vpnView)
{
    vpnView->setAccessibleName("List_vpnList");
    ControllItemsModel *model = new ControllItemsModel(vpnView);
    auto updateItems = [model, this]() {
        const QList<VPNItem *> conns = NetworkController::instance()->vpnController()->items();
        QList<ControllItems *> items;
        for (VPNItem *it : conns) {
            items.append(it);
            if (!m_newConnectionPath.isEmpty() && it->connection()->path() == m_newConnectionPath) {
                NetworkController::instance()->vpnController()->connectItem(it);
                m_newConnectionPath.clear();
            }
        }
        model->updateDate(items);
    };
    VPNController *vpnController = NetworkController::instance()->vpnController();
    updateItems();
    connect(vpnController, &VPNController::itemAdded, model, updateItems);
    connect(vpnController, &VPNController::itemRemoved, model, updateItems);
    connect(vpnController, &VPNController::itemChanged, model, &ControllItemsModel::updateStatus);
    connect(vpnController, &VPNController::activeConnectionChanged, model, &ControllItemsModel::updateStatus);
    connect(vpnController, &VPNController::enableChanged, model, &ControllItemsModel::updateStatus);

    vpnView->setModel(model);
    vpnView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    vpnView->setBackgroundType(DStyledItemDelegate::BackgroundType::ClipCornerBackground);
    vpnView->setSelectionMode(QAbstractItemView::NoSelection);
    vpnView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    auto adjustHeight = [vpnView]() {
        vpnView->setMinimumHeight(vpnView->model()->rowCount() * 41);
    };
    adjustHeight();
    connect(model, &ControllItemsModel::modelReset, vpnView, adjustHeight);
    connect(model, &ControllItemsModel::detailClick, this, &VPNModule::editConnection);
    connect(vpnView, &DListView::clicked, this, [](const QModelIndex &idx) {
        VPNItem *item = static_cast<VPNItem *>(idx.internalPointer());
        if (!item->connected())
            NetworkController::instance()->vpnController()->connectItem(item);
    });
}

void VPNModule::editConnection(ControllItems *item, QWidget *parent)
{
    VPNItem *vpn = static_cast<VPNItem *>(item);
    QString connUuid;
    if (vpn) {
        connUuid = vpn->connection()->uuid();
    }
    ConnectionVpnEditPage *editPage = new ConnectionVpnEditPage(connUuid, parent);
    editPage->setAttribute(Qt::WA_DeleteOnClose);
    if (vpn) {
        editPage->initSettingsWidget();
        editPage->setLeftButtonEnable(true);
    } else {
        editPage->initSettingsWidgetByType(ConnectionVpnEditPage::VpnType::UNSET);
        editPage->setButtonTupleEnable(true);
    }
    connect(editPage, &ConnectionVpnEditPage::disconnect, this, [=] {
        VPNController *vpnController = NetworkController::instance()->vpnController();
        vpnController->disconnectItem();
    });
    connect(editPage, &ConnectionVpnEditPage::activateVpnConnection, this, [vpn, this](const QString &path, const QString &devicePath) {
        Q_UNUSED(devicePath);
        VPNController *vpnController = NetworkController::instance()->vpnController();
        if (vpn) {
            vpnController->connectItem(vpn);
        } else {
            m_newConnectionPath.clear();
            bool findConnection = false;
            QList<VPNItem *> items = vpnController->items();
            for (VPNItem *item : items) {
                if (item->connection()->path() == path) {
                    vpnController->connectItem(item);
                    findConnection = true;
                    break;
                }
            }
            if (!findConnection)
                m_newConnectionPath = path;
        }
    });
    editPage->exec();
}

void VPNModule::importVPN()
{
    QWidget *parent = qobject_cast<QWidget *>(sender());
    QFileDialog *importFile = new QFileDialog(parent);
    importFile->setAccessibleName("VpnPage_importFile");
    importFile->setModal(true);
    importFile->setNameFilter("*.conf");
    importFile->setAcceptMode(QFileDialog::AcceptOpen);
    QStringList directory = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    if (!directory.isEmpty()) {
        importFile->setDirectory(directory.first());
    }
    importFile->deleteLater();
    if (QFileDialog::Accepted == importFile->exec()) {
        QString file = importFile->selectedFiles().first();
        if (file.isEmpty())
            return;

        const auto args = QStringList{ "connection", "import", "type", vpnConfigType(file), "file", file };

        QProcess p;
        p.start("nmcli", args);
        p.waitForFinished();
        const auto stat = p.exitCode();
        const QString output = p.readAllStandardOutput();
        QString error = p.readAllStandardError();

        qDebug() << stat << ",output:" << output << ",err:" << error;

        if (stat) {
            DDialog *dialog = new DDialog(parent);
            dialog->setAttribute(Qt::WA_DeleteOnClose);
            dialog->setTitle(tr("Import Error"));
            dialog->setMessage(tr("File error"));
            dialog->addButton(tr("OK"));

            const auto ratio = dialog->devicePixelRatioF();
            QPixmap icon = QIcon::fromTheme("dialog-error").pixmap(QSize(48, 48) * ratio);
            icon.setDevicePixelRatio(ratio);

            dialog->setIcon(icon);
            dialog->exec();
            return;
        }

        const QRegularExpression regexp("\\(\\w{8}(-\\w{4}){3}-\\w{12}\\)");
        const auto match = regexp.match(output);

        if (match.hasMatch()) {
            m_editingConnUuid = match.captured();
            m_editingConnUuid.replace("(", "");
            m_editingConnUuid.replace(")", "");
            qDebug() << "editing connection Uuid";
            QTimer::singleShot(10, this, &VPNModule::changeVpnId);
        }
    }
}

void VPNModule::changeVpnId()
{
    NetworkManager::Connection::List connList = NetworkManager::listConnections();
    QString importName = "";
    for (const auto &conn : connList) {
        if (conn->settings()->connectionType() == NetworkManager::ConnectionSettings::Vpn) {
            if (m_editingConnUuid == conn->uuid()) {
                importName = conn->name();
                break;
            }
        }
    }
    if (importName.isEmpty()) {
        QTimer::singleShot(10, this, &VPNModule::changeVpnId);
        return;
    }

    QString changeName = "";
    bool hasSameName = false;
    for (const auto &conn : connList) {
        const QString vpnName = conn->name();
        const QString vpnUuid = conn->uuid();
        if ((vpnName == importName) && (vpnUuid != m_editingConnUuid)) {
            changeName = importName + "(1)";
            hasSameName = true;
            break;
        }
    }
    if (!hasSameName) {
        return;
    }

    for (int index = 1;; index++) {
        hasSameName = false;
        for (const auto &conn : connList) {
            QString vpnName = conn->name();
            if (vpnName == changeName) {
                changeName = importName + "(%1)";
                changeName = changeName.arg(index);
                hasSameName = true;
                break;
            }
        }
        if (!hasSameName) {
            break;
        }
    }

    NetworkManager::Connection::Ptr uuidConn = NetworkManager::findConnectionByUuid(m_editingConnUuid);
    if (uuidConn) {
        NetworkManager::ConnectionSettings::Ptr connSettings = uuidConn->settings();
        connSettings->setId(changeName);
        // update function saves the settings on the hard disk
        QDBusPendingReply<> reply = uuidConn->update(connSettings->toMap());
        reply.waitForFinished();
        if (reply.isError()) {
            qDebug() << "error occurred while updating the connection" << reply.error();
            return;
        }
        qDebug() << "find Connection By Uuid is success";
        return;
    }
}
