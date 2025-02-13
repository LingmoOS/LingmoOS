#include "permissionsurveillance.h"
#include <QDebug>

PermissionSurveillance::PermissionSurveillance(QObject *parent)
    : QObject{parent}
{
    m_permissionSurveillanceVisible = false;
    //建立到session bus的连接
    QDBusConnection connection = QDBusConnection::sessionBus();
    if(!connection.registerService("com.lingmo.Statusbar"))
    {
        qDebug() << "error:" << connection.lastError().message();
    }
    if(!connection.registerObject("/Statusbar/PermissionSurveillance", this,QDBusConnection::ExportAllSlots))
    {
        qDebug() << "error:对象注册失败";
    }
}
void PermissionSurveillance::sendCameraUser(QString text)
{
    qDebug() << "接受到相机使用信号！！！";
    if(text=="{U-APPLE_QAQ-U}")
    {
        m_permissionSurveillanceVisible = false;
        m_cameraUser = "";
        emit PermissionSurveillanceVisibleChanged();
        emit cameraUserChanged();
        return;
    }
    m_permissionSurveillanceVisible = true;
    emit PermissionSurveillanceVisibleChanged();
    m_cameraUser = text;
    emit cameraUserChanged();
}
QString PermissionSurveillance::getCameraUser()
{
    return m_cameraUser;
}
bool PermissionSurveillance::getPermissionSurveillanceVisible()
{
    return m_permissionSurveillanceVisible;
}
