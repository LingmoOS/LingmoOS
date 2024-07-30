// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2021-2022 Harald Sitter <sitter@kde.org>

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.lingmoui 2.19 as LingmoUI

import org.kde.drkonqi 1.0

RowLayout {
    required property bool loading
    required property bool failed
    property int usefulness: BacktraceParser.InvalidUsefulness
    property int stars: {
        switch (usefulness) {
        case BacktraceParser.ReallyUseful:
            return 3
        case BacktraceParser.MayBeUseful:
            return 2
        case BacktraceParser.ProbablyUseless:
            return 1
        case BacktraceParser.Useless:
        case BacktraceParser.InvalidUsefulness:
            return 0
        }
    }

    QQC2.Label {
        Layout.fillWidth: true
        wrapMode: Text.Wrap
        text: {
            const loadingMessage = i18nc("@info", "Waiting for data…")

            if (loading) {
                return loadingMessage
            }

            switch (usefulness) {
            case BacktraceParser.InvalidUsefulness:
                return loadingMessage
            case BacktraceParser.ReallyUseful:
                return i18nc("@info", "The generated crash information is useful");
            case BacktraceParser.MayBeUseful:
                return i18nc("@info", "The generated crash information may be useful");
            case BacktraceParser.ProbablyUseless:
                return i18nc("@info", "The generated crash information is probably not useful");
            case BacktraceParser.Useless:
                return i18nc("@info", "The generated crash information is not useful");
            }
        }
    }

    LingmoUI.Icon {
        implicitWidth: LingmoUI.Units.iconSizes.small
        implicitHeight: implicitWidth
        source: "data-error"
        visible: failed || stars <= 0
    }

    LingmoUI.Icon {
        implicitWidth: LingmoUI.Units.iconSizes.small
        implicitHeight: implicitWidth
        source: "rating"
        enabled: stars >= 1
    }

    LingmoUI.Icon {
        implicitWidth: LingmoUI.Units.iconSizes.small
        implicitHeight: implicitWidth
        source: "rating"
        enabled: stars >= 2
    }

    LingmoUI.Icon {
        implicitWidth: LingmoUI.Units.iconSizes.small
        implicitHeight: implicitWidth
        source: "rating"
        enabled: stars >= 3
    }

    onUsefulnessChanged: {
        console.log("usefulness")
        console.log(usefulness)
        console.log(BacktraceParser.ReallyUseful)
    }
}
