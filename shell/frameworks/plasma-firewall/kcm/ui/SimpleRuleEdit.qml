// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2018 Alexis Lopes Zubeta <contact@azubieta.net>
// SPDX-FileCopyrightText: 2020 Tomaz Canabrava <tcanabrava@kde.org>

import QtQuick 2.12
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1

import QtQuick.Controls 2.12 as QQC2
import org.kde.kirigami 2.4 as Kirigami

import org.kcm.firewall 1.0 as Firewall

Kirigami.FormLayout {
    property alias service : application.model
    property alias index: application.currentIndex
    property alias policy: policy
    property alias incoming: incoming

    QQC2.ComboBox {
        id: application
        Kirigami.FormData.label: kcm.client.name == "firewalld" ?
i18n("Allow connections for:") : i18n("Application:")
        model: kcm.client.knownApplications()
    }

    QQC2.ComboBox {
        id: policy
        Kirigami.FormData.label: i18n("Policy:")
        model: policyChoices
        textRole: "text"
        currentIndex: rule.policy == "" ? 0 : policyChoices.findIndex((policy) => policy.data == rule.policy)
        visible: kcm.client.name != "firewalld"
        onActivated: index => {
            rule.policy = policyChoices[index].data;
        }
    }

    RowLayout {
        Kirigami.FormData.label: i18n("Direction:")
        visible: kcm.client.name != "firewalld"
        QQC2.RadioButton {
            id: incoming
            text: i18n("Incoming")
            icon.name: "arrow-down"
            checked: rule.incoming
            onClicked: rule.incoming = true
        }
        QQC2.RadioButton {
            text: i18n("Outgoing")
            icon.name: "arrow-up"
            checked: !rule.incoming
            onClicked: rule.incoming = false
        }
    }

    onVisibleChanged: {
        console.log("services available: ", kcm.client.knownApplications());
        application.model = kcm.client.knownApplications();
        application.currentIndex = -1;
    }
}
