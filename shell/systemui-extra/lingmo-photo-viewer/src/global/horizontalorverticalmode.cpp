#include "horizontalorverticalmode.h"
#include "variable.h"

static const QString LINGMO_ROTATION_PATH = "/";

static const QString LINGMO_ROTATION_SERVICE = "com.lingmo.statusmanager.interface";

static const QString LINGMO_ROTATION_INTERFACE = "com.lingmo.statusmanager.interface";

HorizontalOrVerticalMode::HorizontalOrVerticalMode()
{
    QDBusConnection::sessionBus().connect(LINGMO_ROTATION_SERVICE, LINGMO_ROTATION_PATH, LINGMO_ROTATION_INTERFACE,
                                          QString("rotations_change_signal"), this, SLOT(rotationChanged(QString)));
    QDBusConnection::sessionBus().connect(LINGMO_ROTATION_SERVICE, LINGMO_ROTATION_PATH, LINGMO_ROTATION_INTERFACE,
                                          QString("mode_change_signal"), this, SLOT(modeChanged(bool)));
}
deviceMode HorizontalOrVerticalMode::defaultModeCapture()
{ // method
    QDBusMessage message_pcORpad = QDBusMessage::createMethodCall(
        LINGMO_ROTATION_SERVICE, LINGMO_ROTATION_PATH, LINGMO_ROTATION_INTERFACE, QString("get_current_tabletmode"));
    QDBusPendingReply<bool> reply_pcORpad = QDBusConnection::sessionBus().call(message_pcORpad);
    if (!reply_pcORpad.isValid()) {
        Variable::g_mainlineVersion = false;
        return PCMode;
    } else {
        Variable::g_mainlineVersion = false;
    }
    if (reply_pcORpad.value()) { //平板模式
        QDBusMessage message = QDBusMessage::createMethodCall(
            LINGMO_ROTATION_SERVICE, LINGMO_ROTATION_PATH, LINGMO_ROTATION_INTERFACE, QString("get_current_rotation"));
        QDBusPendingReply<QString> reply = QDBusConnection::sessionBus().call(message);
        if (reply.value() == "normal" || reply.value() == "upside-down") {
            return PADHorizontalMode; //横屏
        } else {
            return PADVerticalMode; //竖屏
        }
        return PADHorizontalMode; //横屏

    } else {
        return PCMode; // pc模式
    }
    return PCMode; // pc模式
}

void HorizontalOrVerticalMode::rotationChanged(QString res)
{ // signal
    if (res == QString::fromLocal8Bit("normal") || res == QString::fromLocal8Bit("upside-down")) {
        Q_EMIT RotationSig(PADHorizontalMode); //平板横屏
    } else {
        Q_EMIT RotationSig(PADVerticalMode); //平板竖屏
    }
}
void HorizontalOrVerticalMode::modeChanged(bool res)
{
    if (res) {
        QDBusMessage message = QDBusMessage::createMethodCall(
            LINGMO_ROTATION_SERVICE, LINGMO_ROTATION_PATH, LINGMO_ROTATION_INTERFACE, QString("get_current_rotation"));
        QDBusPendingReply<QString> reply = QDBusConnection::sessionBus().call(message);
        if (reply.value() == "normal" || reply.value() == "upside-down") {
            Q_EMIT RotationSig(PADHorizontalMode); //平板横屏

        } else {
            Q_EMIT RotationSig(PADVerticalMode); //平板竖屏
        }
    } else {
        Q_EMIT RotationSig(PCMode); // pc模式
    }
    Q_EMIT RotationSig(PCMode); // pc模式
}
