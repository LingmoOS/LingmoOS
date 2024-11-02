
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.impl 2.12
import QtQuick.Templates 2.12 as T

import org.lingmo.qqc2style.private 1.0 as StylePrivate

T.Label {
    id:control

    color:StylePrivate.StyleHelper.windowtextcolorrole
    linkColor: control.palette.link

    font: StylePrivate.StyleHelper.font

}
