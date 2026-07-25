// A simple wrapper for Dynamic component loader
import QtQuick

Loader {
    Component.onDestruction: sourceComponent = undefined
}
