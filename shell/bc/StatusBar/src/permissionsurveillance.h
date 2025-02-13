#ifndef PERMISSIONSURVEILLANCE_H
#define PERMISSIONSURVEILLANCE_H

#include <QObject>
#include <QDBusConnection>
#include <QDBusMessage>

class PermissionSurveillance : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.lingmo.Statusbar")
    Q_PROPERTY(QString cameraUser READ getCameraUser NOTIFY cameraUserChanged)
    Q_PROPERTY(bool permissionSurveillanceVisible READ getPermissionSurveillanceVisible NOTIFY PermissionSurveillanceVisibleChanged)

public:
    explicit PermissionSurveillance(QObject *parent = nullptr);
    QString getCameraUser();
    bool getPermissionSurveillanceVisible();
public slots:
    void sendCameraUser(QString text);
private:
    QString m_cameraUser;
    bool m_permissionSurveillanceVisible;
signals:
    void cameraUserChanged();
    void PermissionSurveillanceVisibleChanged();
};

#endif // PERMISSIONSURVEILLANCE_H
