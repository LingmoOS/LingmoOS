/*
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>
    SPDX-FileCopyrightText: 2020 Douglas Kosovic <doug@uq.edu.au>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "l2tppppwidget.h"
#include "nm-l2tp-service.h"
#include "ui_l2tpppp.h"

#include <KAcceleratorManager>
#include <KLocalizedString>

L2tpPPPWidget::L2tpPPPWidget(const NetworkManager::VpnSetting::Ptr &setting, QWidget *parent, bool need_peer_eap)
    : QDialog(parent)
    , m_ui(new Ui::L2tpPppWidget)
    , m_need_peer_eap(need_peer_eap)
{
    m_ui->setupUi(this);

    setWindowTitle(i18n("L2TP PPP Options"));

    KAcceleratorManager::manage(this);

    loadConfig(setting);
}

L2tpPPPWidget::~L2tpPPPWidget()
{
    delete m_ui;
}

void L2tpPPPWidget::loadConfig(const NetworkManager::VpnSetting::Ptr &setting)
{
    const QString yesString = QLatin1String("yes");

    // General settings
    const NMStringMap dataMap = setting->data();

    if (m_need_peer_eap) {
        m_ui->grp_authenfication->setVisible(false);
        resize(width(), sizeHint().height());
    } else {
        // Authentication options
        const bool refuse_pap = (dataMap[NM_L2TP_KEY_REFUSE_PAP] == yesString);
        const bool refuse_chap = (dataMap[NM_L2TP_KEY_REFUSE_CHAP] == yesString);
        const bool refuse_mschap = (dataMap[NM_L2TP_KEY_REFUSE_MSCHAP] == yesString);
        const bool refuse_mschapv2 = (dataMap[NM_L2TP_KEY_REFUSE_MSCHAPV2] == yesString);
        const bool refuse_eap = (dataMap[NM_L2TP_KEY_REFUSE_EAP] == yesString);

        QListWidgetItem *item = nullptr;
        item = m_ui->listWidget->item(0); // PAP
        item->setCheckState(refuse_pap ? Qt::Unchecked : Qt::Checked);
        item = m_ui->listWidget->item(1); // CHAP
        item->setCheckState(refuse_chap ? Qt::Unchecked : Qt::Checked);
        item = m_ui->listWidget->item(2); // MSCHAP
        item->setCheckState(refuse_mschap ? Qt::Unchecked : Qt::Checked);
        item = m_ui->listWidget->item(3); // MSCHAPv2
        item->setCheckState(refuse_mschapv2 ? Qt::Unchecked : Qt::Checked);
        item = m_ui->listWidget->item(4); // EAP
        item->setCheckState(refuse_eap ? Qt::Unchecked : Qt::Checked);
    }

    // Cryptography and compression
    const bool mppe = (dataMap[NM_L2TP_KEY_REQUIRE_MPPE] == yesString);
    const bool mppe40 = (dataMap[NM_L2TP_KEY_REQUIRE_MPPE_40] == yesString);
    const bool mppe128 = (dataMap[NM_L2TP_KEY_REQUIRE_MPPE_128] == yesString);
    const bool mppe_stateful = (dataMap[NM_L2TP_KEY_MPPE_STATEFUL] == yesString);

    if (mppe || mppe40 || mppe128) { // If MPPE is use
        m_ui->gbMPPE->setChecked(mppe || mppe40 || mppe128);
        if (mppe128) {
            m_ui->cbMPPECrypto->setCurrentIndex(1); // 128 bit
        } else if (mppe40) {
            m_ui->cbMPPECrypto->setCurrentIndex(2); // 40 bit
        } else {
            m_ui->cbMPPECrypto->setCurrentIndex(0); // Any
        }
        m_ui->cbstatefulEncryption->setChecked(mppe_stateful);
    }

    const bool nobsd = (dataMap[NM_L2TP_KEY_NOBSDCOMP] == yesString);
    m_ui->cbBSD->setChecked(!nobsd);

    const bool nodeflate = (dataMap[NM_L2TP_KEY_NODEFLATE] == yesString);
    m_ui->cbdeflate->setChecked(!nodeflate);

    const bool novjcomp = (dataMap[NM_L2TP_KEY_NO_VJ_COMP] == yesString);
    m_ui->cbTCPheaders->setChecked(!novjcomp);

    const bool nopcomp = (dataMap[NM_L2TP_KEY_NO_PCOMP] == yesString);
    m_ui->cbCompressionNegotiation->setChecked(!nopcomp);

    const bool noaccomp = (dataMap[NM_L2TP_KEY_NO_ACCOMP] == yesString);
    m_ui->cbAddressControlCompression->setChecked(!noaccomp);

    // Echo
    const int lcp_echo_interval = QString(dataMap[NM_L2TP_KEY_LCP_ECHO_INTERVAL]).toInt();
    m_ui->cbsendEcho->setChecked(lcp_echo_interval > 0);

    if (dataMap.contains(QLatin1String(NM_L2TP_KEY_MTU))) {
        m_ui->sbMTU->setValue(QString(dataMap[NM_L2TP_KEY_MTU]).toInt());
    }

    if (dataMap.contains(QLatin1String(NM_L2TP_KEY_MTU))) {
        m_ui->sbMRU->setValue(QString(dataMap[NM_L2TP_KEY_MRU]).toInt());
    }
}

NMStringMap L2tpPPPWidget::setting() const
{
    NMStringMap result;
    const QString yesString = QLatin1String("yes");

    if (!m_need_peer_eap) {
        QListWidgetItem *item = nullptr;
        item = m_ui->listWidget->item(0); // PAP
        if (item->checkState() == Qt::Unchecked) {
            result.insert(NM_L2TP_KEY_REFUSE_PAP, yesString);
        }
        item = m_ui->listWidget->item(1); // CHAP
        if (item->checkState() == Qt::Unchecked) {
            result.insert(NM_L2TP_KEY_REFUSE_CHAP, yesString);
        }
        item = m_ui->listWidget->item(2); // MSCHAP
        if (item->checkState() == Qt::Unchecked) {
            result.insert(NM_L2TP_KEY_REFUSE_MSCHAP, yesString);
        }
        item = m_ui->listWidget->item(3); // MSCHAPv2
        if (item->checkState() == Qt::Unchecked) {
            result.insert(NM_L2TP_KEY_REFUSE_MSCHAPV2, yesString);
        }
        item = m_ui->listWidget->item(4); // EAP
        if (item->checkState() == Qt::Unchecked) {
            result.insert(NM_L2TP_KEY_REFUSE_EAP, yesString);
        }
    }

    // Cryptography and compression
    if (m_ui->gbMPPE->isChecked()) {
        int index = m_ui->cbMPPECrypto->currentIndex();

        switch (index) {
        case 0: // "Any"
            result.insert(NM_L2TP_KEY_REQUIRE_MPPE, yesString);
            break;
        case 1: // "128 bit"
            result.insert(NM_L2TP_KEY_REQUIRE_MPPE_128, yesString);
            break;
        case 2: // "40 bit"
            result.insert(NM_L2TP_KEY_REQUIRE_MPPE_40, yesString);
            break;
        }

        if (m_ui->cbstatefulEncryption->isChecked()) {
            result.insert(NM_L2TP_KEY_MPPE_STATEFUL, yesString);
        }
    }

    if (!m_ui->cbBSD->isChecked()) {
        result.insert(NM_L2TP_KEY_NOBSDCOMP, yesString);
    }
    if (!m_ui->cbdeflate->isChecked()) {
        result.insert(NM_L2TP_KEY_NODEFLATE, yesString);
    }

    if (!m_ui->cbTCPheaders->isChecked()) {
        result.insert(NM_L2TP_KEY_NO_VJ_COMP, yesString);
    }

    if (!m_ui->cbCompressionNegotiation->isChecked()) {
        result.insert(NM_L2TP_KEY_NO_PCOMP, yesString);
    }

    if (!m_ui->cbAddressControlCompression->isChecked()) {
        result.insert(NM_L2TP_KEY_NO_ACCOMP, yesString);
    }

    // Echo
    if (m_ui->cbsendEcho->isChecked()) {
        result.insert(NM_L2TP_KEY_LCP_ECHO_FAILURE, "5");
        result.insert(NM_L2TP_KEY_LCP_ECHO_INTERVAL, "30");
    }

    if (m_ui->sbMTU->value() != 0) {
        result.insert(NM_L2TP_KEY_MTU, QString::number(m_ui->sbMTU->value()));
    }

    if (m_ui->sbMRU->value() != 0) {
        result.insert(NM_L2TP_KEY_MRU, QString::number(m_ui->sbMRU->value()));
    }

    return result;
}

#include "moc_l2tppppwidget.cpp"
