// SPDX-FileCopyrightText: 2022 Felipe Kinoshita <kinofhek@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.lingmoui as LingmoUI

/**
 * @brief A placeholder for loading pages.
 *
 * Example usage:
 * @code{.qml}
 *     LingmoUI.Page {
 *         LingmoUI.LoadingPlaceholder {
 *             anchors.centerIn: parent
 *         }
 *     }
 * @endcode
 * @code{.qml}
 *     LingmoUI.Page {
 *         LingmoUI.LoadingPlaceholder {
 *             anchors.centerIn: parent
 *             determinate: true
 *             progressBar.value: loadingValue
 *         }
 *     }
 * @endcode
 * @inherit org::kde::lingmoui::PlaceholderMessage
 */
LingmoUI.PlaceholderMessage {
    id: loadingPlaceholder

    /**
     * @brief This property holds whether the loading message shows a
     * determinate progress bar or not.
     *
     * This should be true if you want to display the actual
     * percentage when it's loading.
     *
     * default: ``false``
     */
    property bool determinate: false

    /**
     * @brief This property holds a progress bar.
     *
     * This should be used to access the progress bar to change its value.
     *
     * @property QtQuick.Controls.ProgressBar _progressBar
     */
    property alias progressBar: _progressBar

    text: qsTr("Loading…")

    QQC2.ProgressBar {
        id: _progressBar
        Layout.alignment: Qt.AlignHCenter
        Layout.fillWidth: true
        Layout.maximumWidth: LingmoUI.Units.gridUnit * 20
        indeterminate: !determinate
        from: 0
        to: 100
    }
}
