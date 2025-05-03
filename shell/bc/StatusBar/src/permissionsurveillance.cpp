#include "permissionsurveillance.h"
#include <QDebug>

PermissionSurveillance::PermissionSurveillance(QObject *parent)
    : QObject{parent}
{
    m_permissionSurveillanceVisible = false;
    m_screenCaptureVisible = false;
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

void PermissionSurveillance::sendScreenCaptureUser(QString text)
{
    qDebug() << "接受到屏幕捕获信号！！！";
    if(text=="{U-APPLE_QAQ-U}")
    {
        m_screenCaptureVisible = false;
        m_screenCaptureUser = "";
        emit screenCaptureVisibleChanged();
        emit screenCaptureUserChanged();
        return;
    }
    m_screenCaptureVisible = true;
    emit screenCaptureVisibleChanged();
    m_screenCaptureUser = text;
    emit screenCaptureUserChanged();
}

QString PermissionSurveillance::getCameraUser()
{
    return m_cameraUser;
}

bool PermissionSurveillance::getPermissionSurveillanceVisible()
{
    return m_permissionSurveillanceVisible;
}

QString PermissionSurveillance::getScreenCaptureUser()
{
    return m_screenCaptureUser;
}

bool PermissionSurveillance::getScreenCaptureVisible()
{
    return m_screenCaptureVisible;
}
