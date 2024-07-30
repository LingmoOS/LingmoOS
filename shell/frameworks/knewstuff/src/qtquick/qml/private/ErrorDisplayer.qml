/*
    SPDX-FileCopyrightText: 2020 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.newstuff as NewStuff

Kirigami.PromptDialog {
    id: component

    title: i18ndc("knewstuff6", "Title for a dialog box which shows error messages", "An Error Occurred");
    standardButtons: Kirigami.Dialog.NoButton

    property bool active: true
    property NewStuff.Engine engine

    readonly property Connections connection: Connections {
        target: component.engine
        function onErrorCode(errorCode, message, metadata) {
            component.showError(errorCode, message, metadata);
        }
    }

    property var errorsToShow: []
    function showError(errorCode, errorMessage, errorMetadata) {
        if (active === true) {
            errorsToShow.push({
                code: errorCode,
                message: errorMessage,
                metadata: errorMetadata
            });
            showNextError();
        }
    }
    onVisibleChanged: displayThrottle.start()
    property QtObject displayThrottle: Timer {
        interval: Kirigami.Units.shortDuration
        onTriggered: showNextError()
    }
    function showNextError() {
        if (visible === false && errorsToShow.length > 0) {
            currentError = errorsToShow.shift();
            open();
        }
    }

    property var currentError: null

    RowLayout {
        implicitWidth: Kirigami.Units.gridUnit * 10
        spacing: Kirigami.Units.largeSpacing

        Kirigami.Icon {
            Layout.alignment: Qt.AlignVCenter
            visible: source !== ""
            source: {
                if (currentError === null) {
                    return "";
                } else if (currentError.code === NewStuff.ErrorCode.TryAgainLaterError) {
                    return "accept_time_event";
                } else {
                    return "dialog-warning";
                }
            }
        }

        Kirigami.SelectableLabel {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            wrapMode: Text.Wrap
            textFormat: TextEdit.AutoText
            onLinkActivated: link => Qt.openUrlExternally(link)

            text: {
                if (currentError === null) {
                    return "";
                } else if (currentError.code === NewStuff.ErrorCode.TryAgainLaterError) {
                    return currentError.message + "\n\n" + i18n("Please try again later.")
                } else {
                    return currentError.message;
                }
            }
        }
    }
}
