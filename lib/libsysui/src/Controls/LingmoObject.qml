// A QtObject with a list of children
import QtQuick
import QtQuick.Controls
import LingmoUI

QtObject {
    // List of childrens in this Object
    default property list<QtObject> children
    id:control
}
