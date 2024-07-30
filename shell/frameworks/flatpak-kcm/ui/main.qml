/**
 * SPDX-FileCopyrightText: 2022 Suhaas Joshi <joshiesuhaas0@gmail.com>
 * SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.lingmoui as LingmoUI
import org.kde.kcmutils as KCM
import org.kde.kitemmodels as KItemModels
import org.kde.lingmo.kcm.flatpakpermissions

KCM.ScrollViewKCM {
    id: root
    title: i18n("Flatpak Applications")
    LingmoUI.ColumnView.fillWidth: false
    implicitWidth: LingmoUI.Units.gridUnit * 40
    implicitHeight: LingmoUI.Units.gridUnit * 20
    framedView: false

    function shouldChange(toAppAtFilteredRowIndex: int) {
        const fromAppAtSourceRowIndex = KCM.ConfigModule.currentIndex();

        const toAppAtSourceRowIndex = (toAppAtFilteredRowIndex === -1)
            ? toAppAtFilteredRowIndex
            : filteredRefsModel.mapToSource(filteredRefsModel.index(toAppAtFilteredRowIndex, 0)).row;

        if (toAppAtSourceRowIndex === KCM.ConfigModule.currentIndex()) {
            // Don't reload if it's current anyway.
            return;
        }

        if (fromAppAtSourceRowIndex !== -1 && KCM.ConfigModule.isSaveNeeded()) {
            const m = KCM.ConfigModule.refsModel;
            const fromAppAtSourceIndex = m.index(fromAppAtSourceRowIndex, 0);
            const applicationName = m.data(fromAppAtSourceIndex, FlatpakReferencesModel.Name);
            const applicationIcon = m.data(fromAppAtSourceIndex, FlatpakReferencesModel.Icon);
            const dialog = applyOrDiscardDialogComponent.createObject(this, {
                applicationName,
                applicationIcon,
                fromAppAtSourceRowIndex,
                toAppAtSourceRowIndex,
            });
            dialog.open();
        } else {
            changeApp(toAppAtSourceRowIndex)
        }
    }

    function changeApp(toAppAtSourceRowIndex) {
        let ref = null;
        if (toAppAtSourceRowIndex !== -1) {
            const sourceIndex = KCM.ConfigModule.refsModel.index(toAppAtSourceRowIndex, 0);
            ref = KCM.ConfigModule.refsModel.data(sourceIndex, FlatpakReferencesModel.Ref) as FlatpakReference;
            appsListView.setCurrentIndexLater(toAppAtSourceRowIndex);
        }
        KCM.ConfigModule.pop();
        KCM.ConfigModule.setIndex(toAppAtSourceRowIndex);
        KCM.ConfigModule.push("permissions.qml", { ref });
    }

    Component.onCompleted: {
        KCM.ConfigModule.columnWidth = LingmoUI.Units.gridUnit * 15
        changeApp(KCM.ConfigModule.currentIndex());
    }

    KCM.ConfigModule.buttons: KCM.ConfigModule.Apply | KCM.ConfigModule.Default

    Component {
        id: applyOrDiscardDialogComponent

        LingmoUI.PromptDialog {
            id: dialog

            required property string applicationName
            required property url applicationIcon

            // source model indices
            required property int fromAppAtSourceRowIndex
            required property int toAppAtSourceRowIndex

            readonly property bool narrow: (parent.width - leftMargin - rightMargin) < LingmoUI.Units.gridUnit * 20

            parent: root.LingmoUI.ColumnView.view
            title: i18n("Apply Permissions")
            subtitle: i18n("The permissions of application %1 have been changed. Do you want to apply these changes or discard them?", applicationName)
            standardButtons: QQC2.Dialog.Apply | QQC2.Dialog.Discard

            GridLayout {
                columns: dialog.narrow ? 1 : 2
                columnSpacing: LingmoUI.Units.largeSpacing
                rowSpacing: LingmoUI.Units.largeSpacing

                LingmoUI.Icon {
                    source: dialog.applicationIcon.toString() !== "" ? dialog.applicationIcon : "application-vnd.flatpak.ref"

                    Layout.alignment: Qt.AlignCenter
                    Layout.preferredWidth: LingmoUI.Units.iconSizes.large
                    Layout.preferredHeight: LingmoUI.Units.iconSizes.large
                }
                LingmoUI.SelectableLabel {
                    text: dialog.subtitle
                    wrapMode: Text.Wrap

                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }
            }

            QQC2.Overlay.modal: KcmPopupModal {}

            onOpened: {
                const button = standardButton(QQC2.Dialog.Apply);
                button.forceActiveFocus(Qt.TabFocusReason);
            }

            onApplied: {
                root.KCM.ConfigModule.save()
                root.changeApp(toAppAtSourceRowIndex)
                dialog.close()
            }

            onDiscarded: {
                root.KCM.ConfigModule.load()
                root.changeApp(fromAppAtSourceRowIndex)
                dialog.close()
            }

            onRejected: {
                appsListView.currentIndex = root.KCM.ConfigModule.currentIndex()
            }

            onClosed: destroy()
        }
    }

    header: LingmoUI.SearchField {
        id: filterField
        KeyNavigation.tab: appsListView
        KeyNavigation.down: appsListView
        autoAccept: false
        onAccepted: {
            if (text === "") {
                // The signal also fires when user clicks the "Clear" action/icon/button.
                // In this case don't treat it as a command to open first app.
                return;
            }
            if (filteredRefsModel.count >= 0) {
                appsListView.setCurrentIndexLater(0);
                root.shouldChange(0);
            }
        }
    }

    view: ListView {
        id: appsListView

        function setCurrentIndexLater(sourceRowIndex) {
            // View has not updated yet, and alas -- we don't have suitable
            // hooks here. Note that ListView::onCountChanged WON'T WORK, as
            // unlike KSortFilterProxyModel it won't trigger when changing
            // from e.g. 2 rows [a, b] to another 2 rows [d, c].

            const sourceIndex = filteredRefsModel.sourceModel.index(sourceRowIndex, 0);
            const filteredIndex = filteredRefsModel.mapFromSource(sourceIndex);
            const filteredRowIndex = filteredIndex.valid ? filteredIndex.row : -1;

            delayedCurrentIndexSetter.index = filteredRowIndex;
            delayedCurrentIndexSetter.start();
        }

        // Using Timer object instead of Qt.callLater to get deduplication for free.
        Timer {
            id: delayedCurrentIndexSetter

            property int index: -1

            interval: 0
            running: false

            onTriggered: {
                appsListView.currentIndex = index;
            }
        }

        model: KItemModels.KSortFilterProxyModel {
            id: filteredRefsModel
            sourceModel: root.KCM.ConfigModule.refsModel
            sortOrder: Qt.AscendingOrder
            sortCaseSensitivity: Qt.CaseInsensitive
            sortRole: FlatpakReferencesModel.Name
            filterRole: FlatpakReferencesModel.Name
            filterString: filterField.text
            filterCaseSensitivity: Qt.CaseInsensitive
            onCountChanged: {
                if (count >= 0) {
                    const sourceRowIndex = root.KCM.ConfigModule.currentIndex();
                    appsListView.setCurrentIndexLater(sourceRowIndex);
                }
            }
        }
        currentIndex: -1
        delegate: QQC2.ItemDelegate {
            required property int index
            required property var model

            width: ListView.view.width - ListView.view.leftMargin - ListView.view.rightMargin

            highlighted: ListView.isCurrentItem

            text: model.name
            // Prefer source, fallback to name. This is unusual for QtQuick.Controls.
            icon.name: model.icon.toString() !== "" ? "" : "application-vnd.flatpak.ref"
            icon.source: model.icon.toString() === "" ? "" : model.icon

            onClicked: root.shouldChange(index)
        }
    }
}
