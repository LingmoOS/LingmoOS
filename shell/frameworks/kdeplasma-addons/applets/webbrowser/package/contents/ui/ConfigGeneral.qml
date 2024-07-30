import QtQuick
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.12 as QQC2
import org.kde.plasma.components 3.0 as PlasmaComponents3

import org.kde.kirigami 2.20 as Kirigami
import org.kde.kcmutils as KCM

KCM.SimpleKCM {
    property alias cfg_useMinViewWidth: useMinViewWidth.checked
    property alias cfg_minViewWidth: minViewWidth.value
    property alias cfg_constantZoomFactor: constantZoomFactor.value
    property alias cfg_useDefaultUrl: useDefaultUrlRadio.checked
    property alias cfg_defaultUrl: defaultUrl.text

    Kirigami.FormLayout {
        QQC2.ButtonGroup { id: defaultUrlGroup }

        QQC2.RadioButton {
            id: useUrlRadio
            text: i18nc("@option:radio", "Load last-visited page")
            checked: !cfg_useDefaultUrl
            QQC2.ButtonGroup.group: defaultUrlGroup

            Kirigami.FormData.label: i18nc("@title:group", "On startup:")

            onToggled: cfg_useDefaultUrl = false;
        }

        QQC2.RadioButton {
            id: useDefaultUrlRadio
            text: i18nc("@option:radio", "Always load this page:")
            checked: cfg_useDefaultUrl
            QQC2.ButtonGroup.group: defaultUrlGroup

            onToggled: {
                cfg_useDefaultUrl = true;
                defaultUrl.forceActiveFocus();
                defaultUrl.selectAll();
            }
        }

        RowLayout {
            spacing: 0
            Layout.fillWidth: true

            // HACK: Workaround for Kirigami bug 434625
            // due to which a simple Layout.leftMargin doesn't work
            Item { implicitWidth: Kirigami.Units.gridUnit }

            PlasmaComponents3.TextField {
                id: defaultUrl
                onAccepted: {
                    let url = text;
                    if (url.indexOf(":/") < 0) {
                        url = "http://" + url;
                    }
                }

                Layout.fillWidth: true

                text: cfg_defaultUrl
                enabled: useDefaultUrlRadio.checked
                Accessible.description: text.length > 0 ? text : i18nc("@info", "Type a URL")
            }
        }     

        Item {
            Kirigami.FormData.isSection: true
        }

        QQC2.ButtonGroup { id: zoomGroup }

        RowLayout {
            Kirigami.FormData.label: i18nc("@title:group", "Content scaling:")

            QQC2.RadioButton {
                id: useConstantZoom
                text: i18nc("@option:radio", "Fixed scale:")

                QQC2.ButtonGroup.group: zoomGroup

                onClicked: {
                    constantZoomFactor.forceActiveFocus();
                }
            }

            QQC2.SpinBox {
                id: constantZoomFactor
                editable: true
                enabled: useConstantZoom.checked

                validator: RegularExpressionValidator {
                    regularExpression: /[0-9]?[0-9]{2}[ ]?%/
                }

                textFromValue: function(value) {
                    return value+"%";
                }

                valueFromText: function(text) {
                    return text.split(" ")[0].split("%")[0];
                }

                from: 25
                to: 500
            }
        }

        RowLayout {
            QQC2.RadioButton {
                id: useMinViewWidth
                text: i18nc("@option:radio", "Automatic scaling if width is below")

                QQC2.ButtonGroup.group: zoomGroup

                onClicked: {
                    minViewWidth.forceActiveFocus();
                }
            }

            QQC2.SpinBox {
                id: minViewWidth
                editable: true
                enabled: useMinViewWidth.checked

                validator: RegularExpressionValidator {
                    regularExpression: /[0-9]?[0-9]{3}[ ]?px/
                }

                textFromValue: function(value) {
                    return value+"px";
                }

                valueFromText: function(text) {
                    return text.split(" ")[0].split("px")[0];
                }

                from: 320
                to: 3840
            }
        }
    }
}
