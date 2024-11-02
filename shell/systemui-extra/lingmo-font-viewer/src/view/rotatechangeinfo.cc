#include "rotatechangeinfo.h"

RotateChangeInfo::RotateChangeInfo(QObject *parent) : QObject(parent)
{
    qDebug() << "rotate object has created!";

    connect(QApplication::primaryScreen(), &QScreen::primaryOrientationChanged, this, &RotateChangeInfo::slotRotateChanged);
    QDBusConnection::sessionBus().connect(LINGMO_ROTATION_SERVICE, LINGMO_ROTATION_PATH, LINGMO_ROTATION_INTERFACE,
                                          QString("mode_change_signal"), this, SLOT(slotModeChanged(bool)));
    getCurrentScale();
    getCurrentMode();
}

void RotateChangeInfo::slotRotateChanged(){
    QPair<int,int> scale = getCurrentScale();
    qDebug() << "horizontal & vertical mode changed sig has received! current scale: " << scale.first << "*" ;
    emit sigRotationChanged(m_isPCMode, scale.first, scale.second);
}

void RotateChangeInfo::slotModeChanged(bool isPadMode){
    if (isPadMode) {
        m_isPCMode = false;
    } else {
        m_isPCMode = true;
    }
    qDebug() << "pc & pad mode changed sig has received! current mode: " << m_isPCMode;
    emit sigRotationChanged(m_isPCMode, m_screenWidth, m_screenHeight);
}

QPair<int, int> RotateChangeInfo::getCurrentScale(){
    QScreen *screen = qApp->primaryScreen();
    m_screenWidth = screen->size().width();
    m_screenHeight = screen->size().height();
    QPair<int,int> scale;
    scale.first = m_screenWidth;
    scale.second = m_screenHeight;
    qDebug() << "screen size: " << m_screenWidth << "*" << m_screenHeight;
    return scale;
}

bool RotateChangeInfo::getCurrentMode(){
    QDBusMessage message_pcORpad = QDBusMessage::createMethodCall(LINGMO_ROTATION_SERVICE, LINGMO_ROTATION_PATH,
                                                          LINGMO_ROTATION_INTERFACE, QString("get_current_tabletmode"));
    QDBusPendingReply<bool> reply_pcORpad = QDBusConnection::sessionBus().call(message_pcORpad);
    if (!reply_pcORpad.isValid()) {
        m_isPCMode = true;
        return m_isPCMode;
    }
    if (reply_pcORpad.value()) {
        m_isPCMode = false;
    } else {
        m_isPCMode = true;
    }
    return m_isPCMode;
}
