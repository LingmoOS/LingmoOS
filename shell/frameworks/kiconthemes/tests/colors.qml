import QtQuick 2.14
import QtQuick.Controls 2.14 as Controls
import QtQuick.Layouts 1.14

import org.kde.kirigami 2.14 as Kirigami

Kirigami.ApplicationWindow
{
    pageStack.initialPage: Kirigami.Page {
        actions.main: Kirigami.Action {
            icon.name: "edit-delete"
            icon.color: Kirigami.Theme.negativeTextColor
        }

        RowLayout {
            Controls.Button {
                flat: true
                icon.name: "overflow-menu"
                icon.color: "white"
            }

            Controls.Button {
                flat: true
                icon.name: "overflow-menu"
            }

            Controls.Button {
                icon.name: "overflow-menu"
                icon.color: "white"
            }
            Controls.Button {
                icon.name: "uninstall"
                icon.color: "Red"
            }

            Controls.Button {
                icon.name: "overflow-menu"
            }
        }
    }
}
