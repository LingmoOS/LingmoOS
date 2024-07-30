/*
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "vpncadvancedwidget.h"
#include "nm-vpnc-service.h"
#include "ui_vpncadvanced.h"

#include <KAcceleratorManager>
#include <KLocalizedString>

VpncAdvancedWidget::VpncAdvancedWidget(const NetworkManager::VpnSetting::Ptr &setting, QWidget *parent)
    : QDialog(parent)
    , m_ui(new Ui::VpncAdvancedWidget)
{
    m_ui->setupUi(this);

    setWindowTitle(i18n("Advanced VPNC properties"));

    // vendor
    m_ui->vendor->addItem(i18nc("VPNC vendor name", "Cisco"), NM_VPNC_VENDOR_CISCO);
    m_ui->vendor->addItem(i18nc("VPNC vendor name", "Netscreen"), NM_VPNC_VENDOR_NETSCREEN);

    // encryption
    m_ui->encryption->addItem(i18nc("VPNC encryption method", "Secure (default)"));
    m_ui->encryption->addItem(i18nc("VPNC encryption method", "Weak (DES encryption, use with caution)"), NM_VPNC_KEY_SINGLE_DES);
    m_ui->encryption->addItem(i18nc("VPNC encryption method", "None (completely insecure)"), NM_VPNC_KEY_NO_ENCRYPTION);

    // NAT traversal
    m_ui->nat->addItem(i18nc("NAT traversal method", "NAT-T when available (default)"), NM_VPNC_NATT_MODE_NATT);
    m_ui->nat->addItem(i18nc("NAT traversal method", "NAT-T always"), NM_VPNC_NATT_MODE_NATT_ALWAYS);
    m_ui->nat->addItem(i18nc("NAT traversal method", "Cisco UDP"), NM_VPNC_NATT_MODE_CISCO);
    m_ui->nat->addItem(i18nc("NAT traversal method", "Disabled"), NM_VPNC_NATT_MODE_NONE);

    // IKE DH group
    m_ui->dhGroup->addItem(i18nc("IKE DH group", "DH Group 1"), NM_VPNC_DHGROUP_DH1);
    m_ui->dhGroup->addItem(i18nc("IKE DH group", "DH Group 2 (default)"), NM_VPNC_DHGROUP_DH2);
    m_ui->dhGroup->addItem(i18nc("IKE DH group", "DH Group 5"), NM_VPNC_DHGROUP_DH5);

    // PFS
    m_ui->pfs->addItem(i18nc("Perfect Forward Secrecy", "Server (default)"), NM_VPNC_PFS_SERVER);
    m_ui->pfs->addItem(i18nc("Perfect Forward Secrecy", "None"), NM_VPNC_PFS_NOPFS);
    m_ui->pfs->addItem(i18nc("Perfect Forward Secrecy", "DH Group 1"), NM_VPNC_PFS_DH1);
    m_ui->pfs->addItem(i18nc("Perfect Forward Secrecy", "DH Group 2"), NM_VPNC_PFS_DH2);
    m_ui->pfs->addItem(i18nc("Perfect Forward Secrecy", "DH Group 5"), NM_VPNC_PFS_DH5);

    loadConfig(setting);

    KAcceleratorManager::manage(this);
}

VpncAdvancedWidget::~VpncAdvancedWidget()
{
    delete m_ui;
}

void VpncAdvancedWidget::loadConfig(const NetworkManager::VpnSetting::Ptr &setting)
{
    m_ui->domain->setText(setting->data().value(NM_VPNC_KEY_DOMAIN));

    const QString vendor = setting->data().value(NM_VPNC_KEY_VENDOR);
    if (!vendor.isEmpty()) {
        m_ui->vendor->setCurrentIndex(m_ui->vendor->findData(vendor));
    }

    if (setting->data().value(NM_VPNC_KEY_SINGLE_DES) == "yes") {
        m_ui->encryption->setCurrentIndex(m_ui->encryption->findData(NM_VPNC_KEY_SINGLE_DES));
    } else if (setting->data().value(NM_VPNC_KEY_NO_ENCRYPTION) == "yes") {
        m_ui->encryption->setCurrentIndex(m_ui->encryption->findData(NM_VPNC_KEY_NO_ENCRYPTION));
    }

    const QString nat = setting->data().value(NM_VPNC_KEY_NAT_TRAVERSAL_MODE);
    if (!nat.isEmpty()) {
        m_ui->nat->setCurrentIndex(m_ui->nat->findData(nat));
    }

    const QString dhGroup = setting->data().value(NM_VPNC_KEY_DHGROUP);
    if (!dhGroup.isEmpty()) {
        m_ui->dhGroup->setCurrentIndex(m_ui->dhGroup->findData(dhGroup));
    } else {
        m_ui->dhGroup->setCurrentIndex(m_ui->dhGroup->findData(NM_VPNC_DHGROUP_DH2)); // default
    }

    const QString pfs = setting->data().value(NM_VPNC_KEY_PERFECT_FORWARD);
    if (!pfs.isEmpty()) {
        m_ui->pfs->setCurrentIndex(m_ui->pfs->findData(pfs));
    }

    bool ok = false;

    const QString localport = setting->data().value(NM_VPNC_KEY_LOCAL_PORT);
    if (!localport.isEmpty()) {
        const uint localportNum = localport.toUInt(&ok);
        if (ok && localportNum <= 65535) {
            m_ui->localport->setValue(localportNum);
        }
    }

    const uint dpd = setting->data().value(NM_VPNC_KEY_DPD_IDLE_TIMEOUT).toUInt(&ok);
    m_ui->deadPeer->setChecked(ok && dpd == 0);
}

NMStringMap VpncAdvancedWidget::setting() const
{
    NMStringMap result;
    if (!m_ui->domain->text().isEmpty()) {
        result.insert(NM_VPNC_KEY_DOMAIN, m_ui->domain->text());
    }

    result.insert(NM_VPNC_KEY_VENDOR, m_ui->vendor->currentData().toString());

    const QString encData = m_ui->encryption->currentData().toString();
    if (!encData.isEmpty()) {
        if (encData == NM_VPNC_KEY_SINGLE_DES) {
            result.insert(NM_VPNC_KEY_SINGLE_DES, "yes");
        } else if (encData == NM_VPNC_KEY_NO_ENCRYPTION) {
            result.insert(NM_VPNC_KEY_NO_ENCRYPTION, "yes");
        }
    }

    result.insert(NM_VPNC_KEY_NAT_TRAVERSAL_MODE, m_ui->nat->currentData().toString());
    result.insert(NM_VPNC_KEY_DHGROUP, m_ui->dhGroup->currentData().toString());
    result.insert(NM_VPNC_KEY_PERFECT_FORWARD, m_ui->pfs->currentData().toString());
    result.insert(NM_VPNC_KEY_LOCAL_PORT, QString::number(m_ui->localport->value()));

    if (m_ui->deadPeer->isChecked()) {
        result.insert(NM_VPNC_KEY_DPD_IDLE_TIMEOUT, "0");
    }

    return result;
}

#include "moc_vpncadvancedwidget.cpp"
