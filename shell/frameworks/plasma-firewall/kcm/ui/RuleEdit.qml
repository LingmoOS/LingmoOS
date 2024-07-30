// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2018 Alexis Lopes Zubeta <contact@azubieta.net>
// SPDX-FileCopyrightText: 2020 Tomaz Canabrava <tcanabrava@kde.org>

import QtQuick 2.12
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1

import QtQuick.Controls 2.12 as QQC2
import org.kde.kirigami 2.4 as Kirigami

import org.kcm.firewall 1.0 as Firewall

FocusScope {
    id: ruleEdit

    property bool busy: false
    property bool ready: (!advancedRuleEdit.sourceAddress.length || advancedRuleEdit.sourceAddress.acceptableInput)
                          && (!advancedRuleEdit.destinationAddress.length || advancedRuleEdit.destinationAddress.acceptableInput)
                          && !(advancedRuleEdit.sourceAddress.text == advancedRuleEdit.destinationAddress.text
                               && advancedRuleEdit.sourcePort.text == advancedRuleEdit.destinationPort.text)
                          || simple.index != -1
    property alias simple: simpleRuleEdit

    property Firewall.FirewallClient client: null

    property var defaultOutgoingPolicyRule: null
    property var defaultIncomingPolicyRule: null

    property var rule: Firewall.Rule {
        policy: "deny"
        incoming: true
        logging: "none"
        protocol: 0
    }

    property var ruleChoices : [
        {text: i18n("None"), data: "none"},
        {text: i18n("New Connections"), data: "log"},
        {text: i18n("All Packets"), data: "log-all"}
    ]
    property bool newRule: false

    enabled: !busy

    implicitWidth: formLayout.implicitWidth
    implicitHeight: formLayout.implicitHeight

    Kirigami.FormLayout {
        id: formLayout
        width: parent.width
        Kirigami.InlineMessage {
            Layout.fillWidth: true
            type: Kirigami.MessageType.Information
            text: rule.incoming ? i18n("The default incoming policy is already '%1'.", advancedRuleEdit.policy.currentText)
                                : i18n("The default outgoing policy is already '%1'.", advancedRuleEdit.policy.currentText)
            visible: rule.policy === (advancedRuleEdit.incoming.checked ? defaultIncomingPolicyRule : defaultOutgoingPolicyRule) && advancedRules.checked
        }
        Kirigami.InlineMessage {
            Layout.fillWidth: true
            type: Kirigami.MessageType.Information
            text: rule.incoming ? i18n("The default incoming policy is already '%1'.", advancedRuleEdit.policy.currentText)
                                : i18n("The default outgoing policy is already '%1'.", advancedRuleEdit.policy.currentText)
            visible: rule.policy === (simple.incoming.checked ? defaultIncomingPolicyRule : defaultOutgoingPolicyRule) && !advancedRules.checked
        }
        SimpleRuleEdit {
            id: simpleRuleEdit
            visible: !advancedRules.checked
        }

        CheckBox {
            id: advancedRules
            text:"Advanced"
            onClicked: rule.simplified = !rule.simplified
            checked: rule.simplified ? false : true // show advanced mode directly if isn't simple !
        }

        AdvancedRuleEdit {
            id: advancedRuleEdit
            rule: ruleEdit.rule
            visible: advancedRules.checked
        }
    }
}
