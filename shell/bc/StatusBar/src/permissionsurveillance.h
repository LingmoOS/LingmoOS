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
    Q_PROPERTY(QString screenCaptureUser READ getScreenCaptureUser NOTIFY screenCaptureUserChanged)
    Q_PROPERTY(bool screenCaptureVisible READ getScreenCaptureVisible NOTIFY screenCaptureVisibleChanged)

public:
    explicit PermissionSurveillance(QObject *parent = nullptr);
    QString getCameraUser();
    bool getPermissionSurveillanceVisible();
    QString getScreenCaptureUser();
    bool getScreenCaptureVisible();

public slots:
    void sendCameraUser(QString text);
    void sendScreenCaptureUser(QString text);

private:
    QString m_cameraUser;
    bool m_permissionSurveillanceVisible;
    QString m_screenCaptureUser;
    bool m_screenCaptureVisible;

signals:
    void cameraUserChanged();
    void PermissionSurveillanceVisibleChanged();
    void screenCaptureUserChanged();
    void screenCaptureVisibleChanged();
};

#endif // PERMISSIONSURVEILLANCE_H
