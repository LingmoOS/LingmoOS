import QtQuick
import QtQuick.Controls
import QtQuick.Window
import org.kde.lingmoui as LingmoUI
import QtTest
import "../tests"

TestCase {
    id: testCase

    LingmoUI.MnemonicData.enabled: true
    LingmoUI.MnemonicData.label: "设置(&S)"

    width: 400
    height: 400

    function test_press() {
        compare(LingmoUI.MnemonicData.richTextLabel, "设置")
    }
}
