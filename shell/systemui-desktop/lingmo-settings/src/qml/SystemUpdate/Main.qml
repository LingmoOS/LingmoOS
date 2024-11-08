import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import LingmoUI 1.0 as LingmoUI
import Lingmo.Settings 1.0
import "../"

ItemPage {
    id: control
    headerTitle: qsTr("Update")

    property bool updating: false
    property bool updateSuccess: false

    // DragHandler {
    //     target: null
    //     acceptedDevices: PointerDevice.GenericPointer
    //     grabPermissions: PointerHandler.CanTakeOverFromItems
    //     onActiveChanged: if (active) { rootWindow.helper.startSystemMove(rootWindow) }
    // }

    StackView {
        id: stackView
        anchors.fill: parent
        initialItem: firstPage
    }

    Updator {
        id: updator

        // onCheckUpdateFinished: {
        //     stackView.push(updatePage)
        // }

        onStartingUpdate: {
            rootWindow.updating = true
            stackView.push(updatingPage)
        }

        onUpdateFinished: {
            rootWindow.updateSuccess = true
            rootWindow.updating = false
            stackView.push(finishedPage)
        }

        onUpdateError: {
            rootWindow.updateSuccess = false
            rootWindow.updating = false
            stackView.push(finishedPage)
        }
    }

    Component {
        id: firstPage

        FirstPage { }
    }

    // Component {
    //     id: homePage

    //     HomePage { }
    // }

    // Component {
    //     id: updatePage

    //     UpdatePage {}
    // }

    Component {
        id: updatingPage

        UpdatingPage {}
    }

    Component {
        id: finishedPage

        FinishedPage {}
    }
}
