import QtQuick 2.14
import QtQuick.Controls 2.14 as Controls
import QtQuick.Layouts 1.14

import org.kde.lingmoui 2.14 as LingmoUI

LingmoUI.ApplicationWindow
{
    pageStack.initialPage: LingmoUI.Page {
        actions.main: LingmoUI.Action {
            icon.name: "edit-delete"
            icon.color: LingmoUI.Theme.negativeTextColor
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
