// SPDX-FileCopyrightText: 2018 Daniel Vrátil <dvratil@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.15 as QQC2 // For StackView
import org.kde.lingmo.plasmoid 2.0
import org.kde.lingmo.components 3.0 as LingmoComponents3
import org.kde.lingmo.extras 2.0 as LingmoExtras
import org.kde.lingmo.core 2.0 as LingmoCore

import org.kde.lingmo.private.lingmopass 1.0

Item {
    id: root

    Plasmoid.fullRepresentation: LingmoExtras.Representation {
        collapseMarginsHint: true
        Layout.minimumWidth: LingmoCore.Units.gridUnit * 5
        Layout.minimumHeight: LingmoCore.Units.gridUnit * 5

        property bool expanded: false

        Component.onCompleted: {
            // FIXME: I'm probably doing something wrong, but I'm unable to access
            // "plasmoid" from elsewhere
            expanded = Qt.binding(function() { return plasmoid.expanded; });
        }

        Keys.onPressed: {
            if (!viewStack.filterMode && event.key === Qt.Key_Backspace) {
                viewStack.popPage();
                event.accepted = true;
            }
        }

        onExpandedChanged: {
            if (expanded) {
                filterField.focus = true;
                filterField.forceActiveFocus();
            } else {
                filterField.text = "";
                viewStack.pop(null);
                currentPath.clearName();
            }
        }
        header: LingmoExtras.PlasmoidHeading {
            ColumnLayout {
                anchors.fill: parent

                PasswordSortProxyModel {
                    id: passwordsModel

                    dynamicSortFilter: true
                    isSortLocaleAware: true
                    sortCaseSensitivity: Qt.CaseInsensitive

                    sourceModel: PasswordsModel {}
                }

                PasswordFilterModel {
                    id: filterModel

                    passwordFilter: filterField.text

                    sourceModel: passwordsModel
                }

                Component {
                    id: passwordsPage

                    PasswordsPage {
                        stack: viewStack
                        model: passwordsModel
                        onFolderSelected: {
                            stack.pushPage(index, name);
                        }
                    }
                }

                Component {
                    id: filterPage

                    PasswordsPage {
                        stack: viewStack
                        model: filterModel
                    }
                }

                RowLayout {
                    visible: viewStack.depth > 1 && !viewStack.filterMode
                    LingmoComponents3.ToolButton {
                        icon.name: LayoutMirroring.enabled ? "go-previous-symbolic-rtl" : "go-previous-symbolic"
                        onClicked: viewStack.popPage()
                        enabled: viewStack.depth > 1
                    }

                    LingmoComponents3.Label {
                        id: currentPath

                        Layout.fillWidth: true
                        HoverHandler {
                            id: hoverHandler
                        }
                        LingmoComponents3.ToolTip {
                            text: currentPath.text
                            visible: hoverHandler.hovered
                        }

                        property var _path: []

                        function pushName(name) {
                            _path.push(name);
                            text = _path.join("/");
                        }
                        function popName() {
                            _path.pop();
                            text = _path.join("/");
                        }
                        function clearName() {
                            _path = []
                        }
                    }
                }

                LingmoComponents3.TextField {
                    id: filterField
                    focus: true
                    activeFocusOnTab: true

                    placeholderText: i18n("Filter...")
                    clearButtonShown: true

                    Layout.fillWidth: true

                    Keys.priority: Keys.BeforeItem
                    Keys.onPressed: {
                        if (event.key == Qt.Key_Down) {
                            viewStack.currentItem.focus = true;
                            event.accepted = true;
                        } else if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                            viewStack.currentItem.activateCurrentItem();
                            event.accepted = true;
                        }
                    }
                }
            }
        }

        QQC2.StackView {
            id: viewStack
            anchors.fill: parent

            readonly property bool filterMode: filterField.text !== ""

            onCurrentItemChanged: {
                if (currentItem) {
                    currentItem.focus = true;
                    currentItem.forceActiveFocus();
                }
            }

            onFilterModeChanged: {
                pop(null);
                if (filterMode) {
                    push(filterPage.createObject(viewStack, { rootIndex: null, stack: viewStack }));
                }
                // Keep focus on the filter field
                filterField.focus = true;
                filterField.forceActiveFocus();
            }

            function pushPage(index, name) {
                const newPage = passwordsPage.createObject(viewStack, { rootIndex: index, stack: viewStack });
                push(newPage);
                currentPath.pushName(name);
            }

            function popPage() {
                pop();
                currentPath.popName();
            }
            initialItem: passwordsPage
        }
    }
}
