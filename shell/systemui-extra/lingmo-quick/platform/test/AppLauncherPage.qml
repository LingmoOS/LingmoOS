import QtQuick 2.15
import org.lingmo.quick.platform 1.0 as PlatForm
import org.lingmo.quick.items 1.0

Column {
    id: root
    Row {
        width: 900
        height: 50
        spacing: 4
        TextInputWidget {
            id: desktopFileText
            width: 400
            height: parent.height
            focus: true
        }
        TextInputWidget {
            id: launchAppArg
            width: 100
            height: parent.height
        }
        AppLauncherButton {
            text: "launchApp"
            width: 200;
            height: parent.height
            onClicked: {
                PlatForm.AppLauncher.launchApp(desktopFileText.text);
            }
        }
        AppLauncherButton {
            text: "launchAppWithArgs"
            width: 200;
            height: parent.height
            onClicked: {
                PlatForm.AppLauncher.launchAppWithArguments(desktopFileText.text, launchAppArg.text.split(' '));
            }
        }
    }
    Row {
        width: 900
        height: 50
        spacing: 4
        TextInputWidget {
            id: uriText
            width: 400
            height: parent.height
            focus: true
        }
        AppLauncherButton {
            text: "openUri"
            width: 200;
            height: parent.height
            onClicked: {
                PlatForm.AppLauncher.openUri(uriText.text, "");
            }
        }
    }
}