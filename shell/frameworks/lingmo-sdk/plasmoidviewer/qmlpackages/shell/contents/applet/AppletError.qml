/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.lingmo.core as LingmoCore
import org.kde.lingmo.components 3.0 as PC3
import org.kde.lingmoui 2.20 as LingmoUI
import org.kde.lingmo.plasmoid 2.0

PlasmoidItem {
    id: root

    enum LayoutType {
        HorizontalPanel,
        VerticalPanel,
        Desktop,
        DesktopCompact
    }

    property var errorInformation

    readonly property real minimumPreferredWidth: LingmoUI.Units.gridUnit * 12
    readonly property real minimumPreferredHeight: LingmoUI.Units.gridUnit * 12

    // To properly show the error message in panel
    readonly property int layoutForm: {
        if (fullRepresentationItem.width >= root.minimumPreferredWidth) {
            if (fullRepresentationItem.height >= root.minimumPreferredHeight) {
                return AppletError.Desktop;
            } else if (fullRepresentationItem.height >= LingmoUI.Units.iconSizes.huge + root.fullRepresentationItem.buttonLayout.implicitHeight) {
                return AppletError.DesktopCompact;
            }
        }

        return Plasmoid.formFactor === LingmoCore.Types.Vertical ? AppletError.VerticalPanel : AppletError.HorizontalPanel;
    }
    preloadFullRepresentation: true
    fullRepresentation: GridLayout {
        id: fullRep
        property alias buttonLayout: buttonLayout
        Layout.minimumWidth: {
            switch (root.layoutForm) {
            case AppletError.Desktop:
            case AppletError.DesktopCompact:
                // [Icon] [Text]
                //    [Button]
                // [Information]
                return Math.max(root.minimumPreferredWidth, buttonLayout.implicitWidth);
            case AppletError.VerticalPanel:
                // [Icon]
                // [Copy]
                // [Open]
                return Math.max(headerIcon.implicitWidth, buttonLayout.implicitWidth);
            case AppletError.HorizontalPanel:
                // [Icon] [Copy] [Open]
                return headingLayout.implicitWidth + rowSpacing + buttonLayout.implicitWidth;
            }
        }
        Layout.minimumHeight: {
            switch (root.layoutForm) {
            case AppletError.Desktop:
                return headingLayout.implicitHeight + fullRep.columnSpacing + buttonLayout.implicitHeight + fullRep.columnSpacing + fullContentView.implicitHeight;
            case AppletError.DesktopCompact:
                return Math.max(headingLayout.implicitHeight, buttonLayout.implicitHeight);
            case AppletError.VerticalPanel:
                return headingLayout.implicitHeight + fullRep.columnSpacing + buttonLayout.implicitHeight;
            case AppletError.HorizontalPanel:
                return Math.max(headingLayout.implicitHeight, buttonLayout.implicitHeight);
            }
        }
        // Same as systray popups
        Layout.preferredWidth: LingmoUI.Units.gridUnit * 24
        Layout.preferredHeight: LingmoUI.Units.gridUnit * 24
        Layout.maximumWidth: LingmoUI.Units.gridUnit * 34
        Layout.maximumHeight: LingmoUI.Units.gridUnit * 34

        rowSpacing: textArea.topPadding
        columnSpacing: rowSpacing
        flow: {
            switch (root.layoutForm) {
            case AppletError.HorizontalPanel:
                return GridLayout.LeftToRight;
            default:
                return GridLayout.TopToBottom;
            }
        }

        RowLayout {
            id: headingLayout

            Layout.margins: root.layoutForm !== AppletError.Desktop ? 0 : LingmoUI.Units.gridUnit
            Layout.maximumWidth: fullRep.width
            spacing: 0
            Layout.fillWidth: true

            LingmoUI.Icon {
                id: headerIcon
                implicitWidth: Math.min(LingmoUI.Units.iconSizes.huge, fullRep.width, fullRep.height)
                implicitHeight: implicitWidth

                activeFocusOnTab: true
                source: "dialog-error"

                Accessible.description: heading.text

                LingmoCore.ToolTipArea {
                    anchors.fill: parent
                    enabled: !heading.visible || heading.truncated
                    mainText: heading.text
                    textFormat: Text.PlainText
                }
            }

            LingmoUI.Heading {
                id: heading
                visible: root.layoutForm !== AppletError.VerticalPanel
                // Descent is equal to the amount of space above and below capital letters.
                // Add descent to the sides to make the spacing around Latin text look more even.
                leftPadding: headingFontMetrics.descent
                rightPadding: headingFontMetrics.descent
                text: root.errorInformation ? root.errorInformation.compactError : "No error information."
                textFormat: Text.PlainText
                level: 2
                wrapMode: Text.Wrap
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
                Layout.fillWidth: true
                Layout.maximumHeight: headerIcon.implicitHeight

                FontMetrics {
                    id: headingFontMetrics
                    font: heading.font
                }
            }
        }

        GridLayout {
            id: buttonLayout

            Layout.alignment: Qt.AlignCenter

            rowSpacing: fullRep.rowSpacing
            columnSpacing: parent.columnSpacing
            flow: {
                switch (root.layoutForm) {
                case AppletError.HorizontalPanel:
                case AppletError.VerticalPanel:
                    return fullRep.flow;
                default:
                    return GridLayout.LeftToRight;
                }
            }

            PC3.Button {
                id: copyButton
                display: root.layoutForm === AppletError.HorizontalPanel || root.layoutForm === AppletError.VerticalPanel ? PC3.AbstractButton.IconOnly : PC3.AbstractButton.TextBesideIcon
                text: i18nd("lingmo_shell_org.kde.lingmo.desktop", "Copy to Clipboard")
                icon.name: "edit-copy"
                onClicked: {
                    textArea.selectAll()
                    textArea.copy()
                    textArea.deselect()
                }

                LingmoCore.ToolTipArea {
                    anchors.fill: parent
                    enabled: parent.display === PC3.AbstractButton.IconOnly
                    mainText: parent.text
                    textFormat: Text.PlainText
                }
            }

            Loader {
                id: compactContentLoader
                active: root.layoutForm !== AppletError.Desktop
                visible: active
                sourceComponent: PC3.Button {
                    display: copyButton.display
                    icon.name: "window-new"
                    text: i18nd("lingmo_shell_org.kde.lingmo.desktop", "View Error Details…")
                    checked: dialog.visible
                    onClicked: dialog.visible = !dialog.visible

                    LingmoCore.ToolTipArea {
                        anchors.fill: parent
                        enabled: parent.display === PC3.AbstractButton.IconOnly
                        mainText: parent.text
                        textFormat: Text.PlainText
                    }

                    QQC2.ApplicationWindow {
                        id: dialog
                        flags: Qt.Dialog | Qt.WindowStaysOnTopHint | Qt.WindowMinMaxButtonsHint | Qt.WindowCloseButtonHint
                        minimumWidth: dialogFontMetrics.height * 12
                            + dialogTextArea.leftPadding + dialogTextArea.rightPadding
                        minimumHeight: dialogFontMetrics.height * 12
                            + dialogTextArea.topPadding + dialogTextArea.bottomPadding
                        width: LingmoUI.Units.gridUnit * 24
                        height: LingmoUI.Units.gridUnit * 24
                        color: palette.base
                        QQC2.ScrollView {
                            id: dialogScrollView
                            anchors.fill: parent
                            QQC2.TextArea {
                                id: dialogTextArea
                                // HACK: silence binding loop warnings.
                                // contentWidth seems to be causing the binding loop,
                                // but contentWidth is read-only and we have no control
                                // over how it is calculated.
                                implicitWidth: 0
                                wrapMode: TextEdit.Wrap
                                text: textArea.text
                                font.family: "monospace"
                                readOnly: true
                                selectByMouse: true
                                background: null
                                FontMetrics {
                                    id: dialogFontMetrics
                                    font: dialogTextArea.font
                                }
                            }
                            background: null
                        }
                    }
                }
            }
        }

        PC3.ScrollView {
            id: fullContentView
            // Not handled by a Loader because we need
            // TextEdit::copy() to copy to clipboard.
            visible: !compactContentLoader.active
            Layout.fillHeight: true
            Layout.fillWidth: true
            PC3.TextArea {
                id: textArea
                // HACK: silence binding loop warnings.
                // contentWidth seems to be causing the binding loop,
                // but contentWidth is read-only and we have no control
                // over how it is calculated.
                implicitWidth: 0
                wrapMode: TextEdit.Wrap
                text: root.errorInformation && root.errorInformation.errors ?
                    root.errorInformation.errors.join("\n\n")
                    // This is just to suppress warnings. Users should never see this.
                    : "No error information."
                font.family: "monospace"
                readOnly: true
                selectByMouse: true
            }
        }
    }
}
