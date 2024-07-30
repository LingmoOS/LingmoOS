import QtQuick 2.1
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1
import org.kde.people 1.0
import org.kde.plasma.components 2.0
import org.kde.plasma.core 2.0 as Core
import org.kde.plasma.extras 2.0
import org.kde.kquickcontrolsaddons 2.0

ApplicationWindow {
    width: 640
    height: 480
    visible: true

    Core.SortFilterModel {
        id: filteredPeople
        sortRole: "display"
        sortCaseSensitivity: Qt.CaseInsensitive
        sourceModel: PersonsModel {
            id: people
        }
        filterRegExp: searchField.text
    }

    CheckBox {
        id: areWeMerging
        anchors {
            top: parent.top
            left: parent.left
        }
        text: "Merging"
        onCheckedChanged: toMergeItems.clear()
    }

    TextField {
        id: searchField
        focus: true
        anchors {
            left: areWeMerging.right
            right: mergeLabel.left
        }
    }

    Label {
        id: mergeLabel
        anchors {
            top: parent.top
            right: view.right
        }

        ListModel {
            id: toMergeItems

            function uriIndex(uri) {
                var ret = -1
                for (var i = 0; i < count && ret < 0; ++i) {
                    if (get(i).uri == uri)
                        ret = i;
                }
                return ret
            }

            function addUri(uri, name) {
                if (uriIndex(uri) < 0)
                    toMergeItems.append({ "uri": uri, "name": name })
            }

            function indexes() {
                var ret = new Array
                for(var i=0; i<count; ++i) {
                    ret.push(get(i).uri)
                }
                return people.indexesForUris(ret)
            }

            function removeUri(uri) {
                remove(uriIndex(uri))
            }
        }
    }

    GridView {
        id: view
        clip: true
        anchors {
            top: searchField.bottom
            bottom: parent.bottom
            left: parent.left
            right: contactItem.left
        }

        cellWidth: 100
        cellHeight: 100
        model: filteredPeople
        delegate: ListItem {
            height: view.cellHeight
            width: view.cellWidth - 5

            clip: true
            enabled: true

            ColumnLayout {
                anchors.fill: parent

                Core.IconItem {
                    id: avatar
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    source: decoration
                }
                Label {
                    width: parent.width
                    text: display
                    wrapMode: Text.WrapAnywhere
                    elide: Text.ElideMiddle
                    visible: avatar.status!=Image.Ready
                }
            }

            onClicked: {
                contactItem.contactData = model
                personActions.personUri = model.personUri

                if (areWeMerging.checked) {
                    toMergeItems.addUri(model.personUri, model.display)
                }
            }
        }
    }

    Flickable {
        id: contactItem
        anchors {
            top: parent.top
            right: parent.right
            bottom: parent.bottom
        }
        width: parent.width/2
        property variant contactData

        ColumnLayout {
            width: parent.width
            spacing: 5

            Column {
                Layout.fillWidth: true
                visible: toMergeItems.count > 0

                Label {
                    text: "Contacts To Merge:"
                }

                Repeater {
                    model: toMergeItems
                    delegate: Label {
                        width: parent.width
                        text: name + " - " + uri
                    }
                }

                Button {
                    text: "Merge!"
                    onClicked: {
                        people.createPersonFromIndexes(toMergeItems.indexes())
                        toMergeItems.clear()
                    }
                }

                Rectangle {
                    width: parent.width
                    height: 1
                    color: "#888"
                }
            }


            RowLayout {
                Core.IconItem {
                    id: avatar
                    height: 64
                    width: height
                    source: contactItem.contactData ? contactItem.contactData.decoration : null
                }

                Label {
                    text: contactItem.contactData ? contactItem.contactData.display : ""
                }
            }

            Label {
                id: contactText
                width: parent.width
                text: dataToString(contactItem.contactData)

                function dataToString(data) {
                    var text = ""
                    if (data != null) {
                        for (var prop in data) {
                            text += prop + ": ";
                            var currentData = data[prop]
                            text += currentData == null ? "null" : currentData
                            text += '\n'
                        }
                    }

                    return text
                }
            }

            Flow {
                Layout.fillWidth: true
                Layout.fillHeight: true

                Repeater {
                    width: parent.width
                    model: PersonActions {
                        id: personActions
                    }
                    delegate: Button {
                        text: model.display
                        iconSource: model.iconName
                        onClicked: personActions.triggerAction(model.index)
                    }
                }
            }

            Button {
                text: "Unmerge"
                visible: contactItem.contactData!=null && contactItem.contactData.contactsCount>1
                onClicked: {
                    dialogLoader.sourceComponent = unmergeDialogComponent
                    dialogLoader.item.open()
                }
                Loader {
                    id: dialogLoader
                }
            }
        }
    }

    Component {
        id: unmergeDialogComponent
        CommonDialog {
            id: unmergeDialog
            property string name: contactItem.contactData.name
            property int index: filteredPeople.mapRowToSource(contactItem.contactData.index)
            property url uri: contactItem.contactData.uri

            buttonTexts: ["Unmerge", "Cancel"]
            titleText: i18n("Unmerging %1", unmergeDialog.name)
            content: Column {
                width: 500
                height: 200
                Repeater {
                    id: unmergesView
                    model: ColumnProxyModel {
                        rootIndex: indexFromModel(people, unmergeDialog.index)
                    }
                    delegate: ListItem {
                        property alias checked: willUnmerge.checked
                        property string contactUri: uri
                        enabled: true
                        onClicked: willUnmerge.checked=!willUnmerge.checked
                        Row {
                            spacing: 5
                            CheckBox { id: willUnmerge }
                            Label { text: display+" - "+contactUri }
                        }
                    }
                }
            }
            onButtonClicked: if(index==0) {
                for(var i=0; i<unmergesView.count; ++i) {
                    var item = unmergesView.itemAt(i)
                    if(item.checked)
                        people.unmerge(item.contactUri, uri)
                }
            }
        }
    }
}
