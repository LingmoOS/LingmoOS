#ifndef TOOL_THREAD_H
#define TOOL_THREAD_H

#include <QThread>
#include <QVariant>
#include <QLibrary>
#include <QFile>
#include<sys/types.h>
#include<unistd.h>
#include <polkit-qt5-1/polkitqt1-authority.h>

#define LOCAL_PACKAGE_FLAG "local_pkg"
#define LOCAL_ANDROID_PACKAGE_FLAG "local_Android_pkg"
#define ANDROID_SHARE_UNINSTALL_LIB "/usr/lib/libkmre.so"

#define LINGMOOS_KMRE_SERVICE_NAME "cn.lingmoos.Kmre"
#define LINGMOOS_KMRE_SERVICE_PATH "/cn/lingmoos/Kmre"
#define LINGMOOS_KMRE_INTERFACE_NAME "cn.lingmoos.Kmre"

#define ORG_FREEDESKTOP_NOTIFICATIONS_NAME "org.freedesktop.Notifications"
#define ORG_FREEDESKTOP_NOTIFICATIONS_PATH "org/freedesktop/Notifications"
#define ORG_FREEDESKTOP_NOTIFICATIONS_INTERFACE_NAME "org.freedesktop.Notifications"

#define LINGMOOS_KYDROID_SERVICE_NAME "cn.lingmoos.Kydroid"
#define LINGMOOS_KYDROID_SERVICE_PATH "/cn/lingmoos/Kydroid"
#define LINGMOOS_KYDROID_INTERFACE_NAME "cn.lingmoos.Kydroid"

#define SYS_KMRE_BOOT_COMPLETED "sys.kmre.boot_completed"
#define SYS_KYDROID_BOOT_COMPLETED "sys.kydroid.boot_completed"

#define KAIMING_PACKAGE "kaiming"

class tool_thread : public QThread
{
   Q_OBJECT

public:
    tool_thread(QString runtime, QString app_name, QString version, QString zh_name, QString desktop_name);
    void uninstallAndroidInterface();
    bool getAndroidEnvWetherStart();
    void Slot_Message_Notification(int install,QString errorinfo);

protected:
   void run();

private:
   QString m_app_name;
   QString m_version;
   QString m_runtime;
   QString m_zh_name;
   QString libPath;
   QString m_desktop_name;

signals:
   void sig_uninstall_result(int);
   void sig_uninstall_status(int);
public slots:
   void slot_local_package_uninstall_result(bool status,QString error_string,QString error_desc);
   void slot_kaiming_package_uninstall_result(QString appName, int  type, int errorCode, QString error_desc);
   void slot_local_package();
   //bool polkit();
};

#endif // TOOL_THREAD_H
