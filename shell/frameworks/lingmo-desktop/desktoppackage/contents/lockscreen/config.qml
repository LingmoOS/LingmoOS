import QtQuick 2.5
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.1
import org.kde.lingmoui 2.12 as LingmoUI
import org.kde.kcmutils as KCM

LingmoUI.FormLayout {
    property alias cfg_alwaysShowClock: alwaysClock.checked
    property alias cfg_showMediaControls: showMediaControls.checked
    property bool cfg_alwaysShowClockDefault: false
    property bool cfg_showMediaControlsDefault: false

    twinFormLayouts: parentLayout

    QQC2.CheckBox {
        id: alwaysClock
        LingmoUI.FormData.label: i18ndc("lingmo_shell_org.kde.lingmo.desktop",
                                        "@title: group",
                                        "Clock:")
        text: i18ndc("lingmo_shell_org.kde.lingmo.desktop",
                     "@option:check",
                     "Keep visible when unlocking prompt disappears")

        KCM.SettingHighlighter {
            highlight: cfg_alwaysShowClockDefault != cfg_alwaysShowClock
        }
    }

    QQC2.CheckBox {
        id: showMediaControls
        LingmoUI.FormData.label: i18ndc("lingmo_shell_org.kde.lingmo.desktop",
                                        "@title: group",
                                        "Media controls:")
        text: i18ndc("lingmo_shell_org.kde.lingmo.desktop",
                     "@option:check",
                     "Show under unlocking prompt")

        KCM.SettingHighlighter {
            highlight: cfg_showMediaControlsDefault != cfg_showMediaControls
        }
    }
}
