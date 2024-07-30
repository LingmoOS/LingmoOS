import QtQuick
import QtTest
import QtQuick.Dialogs
TestCase {
    id: testCase
    width: 400
    height: 400
    visible: true
    when: windowShown
    name: "FileDialogTest"

    FileDialog {
        id: fileDialog
        currentFolder: shortcuts.home
    }

    function test_filedialog() {
        //At the moment it just makes sure that something opens and doesn't crash.
        //Since this is a QGuiApplication (and not a QApplication)
        fileDialog.visible = true
        testCase.wait(200)
        fileDialog.visible = false
    }
}
