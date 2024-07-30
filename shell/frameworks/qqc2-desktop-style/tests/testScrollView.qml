/*
    SPDX-FileCopyrightText: 2018 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import org.kde.kirigami as Kirigami

Kirigami.ApplicationWindow {
    pageStack.initialPage: Kirigami.Page {
        Kirigami.Theme.colorSet: Kirigami.Theme.View
        Kirigami.Theme.inherit: false

        padding: 0

        RowLayout {
            spacing: 0
            anchors.fill: parent

            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: parent.width / 2

                Flickable {
                    TextArea.flickable: TextArea {
                        background: null
                        wrapMode: TextEdit.NoWrap
                        text: `Lorem ipsum dolor sit amet, consectetur adipiscing elit. Curabitur ultricies consequat nulla, ut vulputate nulla ultricies ac. Suspendisse lacinia commodo lacus, non tristique mauris dictum vitae. Sed adipiscing augue nec nisi aliquet viverra. Etiam sit amet nulla in tellus consectetur feugiat. Cras in sem tortor. Fusce a nulla at justo accumsan gravida. Maecenas dui felis, lacinia at ornare sed, aliquam et purus. Sed ut sagittis lacus. Etiam dictum pharetra rhoncus. Suspendisse auctor orci ipsum. Pellentesque vitae urna nec felis consequat lobortis dictum in urna. Phasellus a mi ac leo adipiscing varius eget a felis. Cras magna augue, commodo sed placerat vel, tempus vel ligula. In feugiat quam quis est lobortis sed accumsan nunc malesuada. Mauris quis massa sit amet felis tempus suscipit a quis diam.

Aenean quis nulla erat, vel sagittis sem. Praesent vitae mauris arcu. Cras porttitor, ante at scelerisque sodales, nibh felis consectetur orci, ut hendrerit urna urna non urna. Duis eu magna id mi scelerisque adipiscing. Aliquam sed quam in eros sodales accumsan. Phasellus tempus sagittis suscipit. Aliquam rutrum dictum justo ut viverra. Nulla felis sem, molestie sed scelerisque non, consequat vitae nulla. Aliquam ullamcorper malesuada mi, vel vestibulum magna vulputate eget. In hac habitasse platea dictumst. Cras sed lacus dui, vel semper sem. Aenean sodales porta leo vel fringilla.

Ut tempus massa et urna porta non mollis metus ultricies. Duis nec nulla ac metus auctor porta id et mi. Mauris aliquam nibh a ligula malesuada sed tincidunt nibh varius. Sed felis metus, porta et adipiscing non, faucibus id leo. Donec ipsum nibh, hendrerit eget aliquam nec, tempor ut mauris. Suspendisse potenti. Vestibulum scelerisque adipiscing libero tristique eleifend. Donec quis tortor eget elit mollis iaculis ac sit amet nisi. Proin non massa sed nunc rutrum pellentesque. Sed dui lectus, laoreet sed condimentum id, commodo sed urna.
Praesent tincidunt mattis massa mattis porta. Nullam posuere neque at mauris vestibulum vitae elementum leo sodales. Quisque condimentum lectus in libero luctus egestas. Fusce tempor neque ac dui tincidunt eget viverra quam suscipit. In hac habitasse platea dictumst. Etiam metus mi, adipiscing nec suscipit id, aliquet sed sem. Duis urna ligula, ornare sed vestibulum vel, molestie ac nisi. Morbi varius iaculis ligula. Nunc in augue leo, sit amet aliquam elit. Suspendisse rutrum sem diam. Proin eu orci nisl. Praesent porttitor dignissim est, id fermentum arcu venenatis vitae.
Integer in sapien eget quam vulputate lobortis. Morbi nibh elit, elementum vitae vehicula sed, consequat nec erat. Donec placerat porttitor est ut dapibus. Fusce augue orci, dictum et convallis vel, blandit eu tortor. Phasellus non eros nulla. In iaculis nulla fermentum nulla gravida eu mattis purus consectetur. Integer dui nunc, sollicitudin ac tincidunt nec, hendrerit bibendum nunc. Proin sit amet augue ac velit egestas varius. Sed eu ante quis orci vestibulum sagittis. Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos. Phasellus vitae urna odio, at molestie leo. In convallis neque vel mi dictum convallis lobortis turpis sagittis.
`                    }
                }
            }

            Kirigami.Separator {
                Layout.fillHeight: true
            }

            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: parent.width / 2

                ListView {
                    id: listView
                    model: 100
                    currentIndex: 1
                    delegate: ItemDelegate {
                        text: modelData
                        onClicked: listView.currentIndex = index;
                        width: ListView.view.width
                    }
                }
            }
        }
    }
}
