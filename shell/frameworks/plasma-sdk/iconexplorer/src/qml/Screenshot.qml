/*
    SPDX-FileCopyrightText: 2019 Carson Black <uhhadd@gmail.com>
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>
    SPDX-FileCopyrightText: 2020 Ismael Asensio <isma.af@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.2
import QtQuick.Dialogs

import org.kde.kirigami 2.8 as Kirigami


Item {
    id: screenshot

    enum MontageType {
        Active,
        Normal,
        Dark,
        Dual
    }

    visible: false

    signal finished

    property int type: Screenshot.MontageType.Dual;
    property Item selectedMontage: {
        switch (type) {
        case Screenshot.MontageType.Active:
            return activeMontage
        case Screenshot.MontageType.Normal:
            return lightMontage
        case Screenshot.MontageType.Dark:
            return darkMontage
        case Screenshot.MontageType.Dual:
            return dualMontage
        }
    }

    Kirigami.Theme.inherit: false

    function shot(montageType) {
        type = montageType
        ssPicker.open()
    }

    FileDialog {
        id: ssPicker
        fileMode: FileDialog.SaveFile
        onAccepted: {
            selectedMontage.grabToImage(function(result) {
                result.saveToFile(ssPicker.selectedFile.toString().slice(7))
                finished()
            });
        }
        onRejected: {
            finished()
        }
        nameFilters: [ "PNG screenshot files (*.png)" ]
    }

    IconMontage {
        id: activeMontage
        showWatermark: true

        width: 512
        height: 256

        Kirigami.Theme.inherit: true
    }
    Column {
        id:dualMontage

        IconMontage {
            id: lightMontage
            showWatermark: true

            width: 512
            height: 256

            Kirigami.Theme.inherit: false
            Kirigami.Theme.textColor: "#232629"
            Kirigami.Theme.backgroundColor: "#eff0f1"
            Kirigami.Theme.highlightColor: "#3daee9"
            Kirigami.Theme.highlightedTextColor: "#eff0f1"
            Kirigami.Theme.positiveTextColor: "#27ae60"
            Kirigami.Theme.neutralTextColor: "#f67400"
            Kirigami.Theme.negativeTextColor: "#da4453"
        }
        IconMontage {
            id: darkMontage
            showWatermark: screenshot.type == Screenshot.MontageType.Dark

            width: 512
            height: 256

            Kirigami.Theme.inherit: false
            Kirigami.Theme.textColor: "#eff0f1"
            Kirigami.Theme.backgroundColor: "#31363b"
            Kirigami.Theme.highlightColor: "#3daee9"
            Kirigami.Theme.highlightedTextColor: "#eff0f1"
            Kirigami.Theme.positiveTextColor: "#27ae60"
            Kirigami.Theme.neutralTextColor: "#f67400"
            Kirigami.Theme.negativeTextColor: "#da4453"
        }
    }
}
