// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2022 Lucas Biaggi <lbjanuario@gmail.com>

import QtQuick 2.12
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1

import QtQuick.Controls 2.12 as QQC2
import org.kde.kirigami 2.4 as Kirigami

import org.kcm.firewall 1.0 as Firewall

Kirigami.FormLayout {
    id: root
    property var rule: null
    property alias sourceAddress: sourceAddress
    property alias destinationAddress: destinationAddress
    property alias destinationPort: destinationPort
    property alias sourcePort: sourcePort
    property alias policy: policy
    property alias incoming: incoming

    readonly property int maxComboboxWidth: Math.max(
                                                policy.implicitWidth,
                                                protocolCb.implicitWidth,
                                                interfaceCb.implicitWidth,
                                                loggingCb.implicitWidth)

    QQC2.ComboBox {
        id: policy
        Kirigami.FormData.label: i18n("Policy:")
        Layout.preferredWidth: root.maxComboboxWidth
        model: policyChoices
        textRole: "text"
        currentIndex: rule == null ? 0 : rule.policy == "" ? 0 : policyChoices.findIndex((policy) => policy.data == rule.policy)
        onActivated: index => {
            rule.policy = policyChoices[index].data;
        }
    }

    RowLayout {
        Kirigami.FormData.label: i18n("Direction:")
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

    RowLayout {
        Kirigami.FormData.label: i18n("IP Version:")

        QQC2.RadioButton {
            text: i18n("IPv4")
            checked: !rule.ipv6
            onClicked: rule.ipv6 = false;
        }
        QQC2.RadioButton {
            text: i18n("IPv6")
            checked: rule.ipv6
            onClicked: rule.ipv6 = true
        }
    }

    RowLayout {
        Kirigami.FormData.label: i18n("Source:")

        IpTextField {
            id: sourceAddress
            ipv6: rule.ipv6
            focus: true // default focus object
            text: rule.sourceAddress
            Layout.preferredWidth: root.maxComboboxWidth
            // NOTE onEditingFinished doesn't fire with non-acceptable / empty input
            onTextChanged: rule.sourceAddress = text
        }
        PortTextField{
            id: sourcePort
            Layout.preferredWidth: Math.round(root.maxComboboxWidth * 0.75)
            text: rule.sourcePort
            onTextChanged: rule.sourcePort = text
        }
    }

    RowLayout {
        Kirigami.FormData.label: i18n("Destination:")

        IpTextField {
            id: destinationAddress
            ipv6: rule.ipv6
            text: rule.destinationAddress
            Layout.preferredWidth: root.maxComboboxWidth
            onTextChanged: rule.destinationAddress = text
        }
        PortTextField {
            id: destinationPort
            Layout.preferredWidth: Math.round(root.maxComboboxWidth * 0.75)
            text: rule.destinationPort
            onTextChanged: rule.destinationPort = text
        }
    }

    QQC2.ComboBox {
        id: protocolCb

        Kirigami.FormData.label: i18n("Protocol:")
        Layout.preferredWidth: root.maxComboboxWidth
        model: ruleEdit.client.knownProtocols()
        currentIndex: rule.protocol
        onActivated: index => {
            rule.protocol = index;
        }
    }
    QQC2.ComboBox {
        id: interfaceCb

        Kirigami.FormData.label: i18n("Interface:")
        Layout.preferredWidth: root.maxComboboxWidth
        model: ruleEdit.client.knownInterfaces()
        currentIndex: rule.interface
        onActivated: index => {
            rule.interface = index;
        }
    }

    QQC2.ComboBox {
        id: loggingCb

        Kirigami.FormData.label: i18n("Logging:")
        Layout.preferredWidth: root.maxComboboxWidth
        model: ruleChoices
        textRole: "text"
        currentIndex: rule.logging == "" ? 0 : ruleChoices.findIndex((rules) => rules.data == rule.logging)
        onActivated: index => {
            rule.logging = ruleChoices[index].data;
        }
    }
}
