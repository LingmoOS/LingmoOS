import QtQuick 2.15
import QtTest 1.0
import "../../org.lingmo.ocean/contents/lockscreen"


LockScreen {

    SignalSpy {
        id: successSpy
        target: authenticator
        signalName: "succeeded"
    }

    SignalSpy {
        id: quitSpy
        target: engine
        signalName: "quit"
    }

    TestCase {
        when: windowShown
        function test_performLogin() {
            let password = "myPassword";
            for (let i in password) {
                keyClick(password[i]);
            }
            keyClick(Qt.Key_Enter);
            successSpy.wait();
            quitSpy.wait();
        }
    }
}
