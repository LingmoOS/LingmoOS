#ifndef LINGMOINSTALLDBUSCALLTHREAD_H
#define LINGMOINSTALLDBUSCALLTHREAD_H

/*#define LINGMO_OS_DBUS_SERVICE_NAME "cn.lingmoos.LingmoUpdateManager"
#define LINGMO_OS_DBUS_PATH  "/cn/lingmoos/LingmoUpdateManager"
#define LINGMO_OS_DBUS_INTERFACE_NAME "cn.lingmoos.LingmoUpdateManager"
*/

#define LINGMO_OS_DBUS_SERVICE_NAME "com.lingmo.systemupgrade"
#define LINGMO_OS_DBUS_PATH  "/com/lingmo/systemupgrade"
#define LINGMO_OS_DBUS_INTERFACE_NAME "com.lingmo.systemupgrade.interface"


#include <QObject>
#include <QThread>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDebug>
#include <QDBusReply>
#include "kareclass.h"


class LingmoInstalldbusCallThread  : public QThread
{
    Q_OBJECT
public:
    LingmoInstalldbusCallThread(QString debPath,QString m_appName,QString m_application,QString m_applicationZh,QString m_appVersion, QString kareImage);

private:
    QString           m_debPath;

    QString           m_appName;
    QString           m_application;
    QString           m_applicationZh;
    QString           m_appVersion;

    QString           m_osVersion;

    QDBusInterface   *m_pOsServiceInterface;
    QString           m_loginUserName;
    QString           m_loginUserId;
    QString           m_kareImage;

    int SIGNAL1 = 0;

protected:
    void run();

signals:
    void lingmoOsinstallStatus(bool status,QString msg);
    void lingmoOsinstalldingStatus(int status);
    void lingmobatchOsinstalldingStatus(int status,QString installname);


public slots:
    void osInstallOverSoftwareAptSlots(bool status,QString erro_string,QString erro_desc);
    void osInstallOverSoftwareSnapSlots(QString string, QMap<QString, QVariant> map);
    void osInstallOverstatus(int status,QString erro_string,QString erro_desc);
    QString getUserName();
    QString getUid();
    QString osVersion();
    bool copyFile(const QString &srcFile, const QString &destFile);
  //  void onCopyFinished(const QString &fileName, bool success);
    void installApp(const QString &fileName, const QString &pkgName, const QString &application, const QString &applicationZh, const QString &version);
    void updateDekstopAndIcon(const QString &pkgName, const QString &application, const QString &applicationZh, const QString &version);
    bool generateDesktop(const QString &pkgName, const QString &application, const QString &applicationZh, const QString &version);
    void getCpuAndGpuInfo(QString &cpuType, QString &gpuVendor);
    bool isAndroidReady(const QString &username, const QString &uid);
    void generateIcon(const QString &pkgName);
   // void sendApkInfoToServer(const QString &os, const QString &cpuType, const QString &gpuVendor, const QString &pkgName, const QString &appName);
    QByteArray getInstalledAppListJsonStr();
  /*  QByteArray encrypt(const QByteArray &md5, const QByteArray &keyBase64);
    void evpError(void);*/





};

#endif // LINGMOINSTALLDBUSCALLTHREAD_H
