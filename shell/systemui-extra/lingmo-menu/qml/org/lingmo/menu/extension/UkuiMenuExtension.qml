import QtQuick 2.0

Item {
    property var extensionData;
    property Component extensionMenu: null;
    signal send(var data);
}
