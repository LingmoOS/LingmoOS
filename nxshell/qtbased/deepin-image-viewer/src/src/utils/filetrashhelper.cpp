// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filetrashhelper.h"
#include "imagedata/imagefilewatcher.h"
#include "unionimage/baseutils.h"

#include <DSysInfo>

#include <QApplication>
#include <QDBusReply>
#include <QDebug>
#include <QFile>
#include <QFileSystemWatcher>
#include <QRegularExpression>

DCORE_USE_NAMESPACE
/*!
   \brief 拷贝自 dde-file-manager 代码，看图中仅使用 kRemovable 访问可能被卸载的设备
 */
enum DeviceQueryOption {
    kNoCondition = 0,
    kMountable = 1,
    kMounted = 1 << 1,
    kRemovable = 1 << 2,
    kNotIgnored = 1 << 3,
    kNotMounted = 1 << 4,
    kOptical = 1 << 5,
    kSystem = 1 << 6,
    kLoop = 1 << 7,
};

/*!
   \brief Result type for moving a file to the Trash via DBus.
 */
enum DeletionResult {
    kTrashTimeout,  // default error
    kTrashInterfaceError,
    kTrashInvalidUrl,
    kTrashSuccess,
};

/*!
   \class FileTrashHelper::FileTrashHelper
   \brief 文件回收站辅助类
   \details 用于判断文件是否可被移动到回收站中，以及移动文件到回收站
   \note 强关联文管DBus接口，需注意对应接口更新
 */
FileTrashHelper::FileTrashHelper(QObject *parent)
    : QObject(parent)
{
    // DFM 设备管理接口，访问文件挂载信息
    if (DSysInfo::majorVersion() == "25") {
        m_dfmDeviceManager.reset(new QDBusInterface(QStringLiteral(V25_FILEMANAGER_DAEMON_SERVICE),
                                                    QStringLiteral(V25_FILEMANAGER_DAEMON_PATH),
                                                    QStringLiteral(V25_FILEMANAGER_DAEMON_INTERFACE)));
    } else {
        m_dfmDeviceManager.reset(new QDBusInterface(QStringLiteral(V23_FILEMANAGER_DAEMON_SERVICE),
                                                    QStringLiteral(V23_FILEMANAGER_DAEMON_PATH),
                                                    QStringLiteral(V23_FILEMANAGER_DAEMON_INTERFACE)));
    }

    qInfo() << "m_dfmDeviceManager: majorVersion:" << DSysInfo::majorVersion()
            << "dbus service:" << m_dfmDeviceManager.data()->service() << "interface:" << m_dfmDeviceManager.data()->interface()
            << "object:" << m_dfmDeviceManager.data()->objectName() << "path:" << m_dfmDeviceManager.data()->path();
}

/*!
   \return 返回文件 \a url 是否可被移动到回收站中
 */
bool FileTrashHelper::fileCanTrash(const QUrl &url)
{
    // 检测当前文件和上次是否相同
    QDir currentDir(url.path());
    if (lastDir != currentDir) {
        lastDir = currentDir;
        resetMountInfo();
    }

    queryMountInfo();

    if (isGvfsFile(url)) {
        return false;
    }

    if (isExternalDevice(url.path())) {
        return false;
    }

    return true;
}

/*!
   \return 移动文件 \a url 到回收站
 */
bool FileTrashHelper::moveFileToTrash(const QUrl &url)
{
    // 优先采用文管后端 DBus 服务
    int ret = moveFileToTrashWithDBus(url);

    if (kTrashInterfaceError == ret) {
        qInfo() << qPrintable("Move file to trash DBus interface failed! Rollback to v20 version");
        // rollback v20 interface
        if (Libutils::base::trashFile(url.path())) {
            return true;
        }
    }

    if (kTrashSuccess != ret) {
        qWarning() << qPrintable("Move file to trash failed");
        return false;
    }

    return true;
}

/*!
   \return 返回删除文件 \a url 的结果
 */
bool FileTrashHelper::removeFile(const QUrl &url)
{
    QFile file(url.path());
    bool ret = file.remove();
    if (!ret) {
        qWarning() << qPrintable("Remove file failed:") << file.errorString();
    }

    return ret;
}

/*!
   \brief 重置文件挂载信息，仅在需要获取时重新取得挂载数据
    对于看图应用的特殊处理，在打开新的图片后，仅需在首次触发查询挂载设备信息时查询，
    对于中途取消挂载的设备，文件监控会提示文件不存在，因此无需处理;
    同时，看图只会访问同一目录内容，挂载信息仅需查询单次，无需重复查询或监控设备
 */
void FileTrashHelper::resetMountInfo()
{
    initData = false;
    mountDevices.clear();
}

/*!
   \brief 查询当前挂载设备信息
   \note 注意依赖文官接口的变动
 */
void FileTrashHelper::queryMountInfo()
{
    if (initData) {
        return;
    }

    initData = true;

    // 调用 DBus 接口查询可被卸载设备信息
    // GetBlockDevicesIdList(int opts)
    QDBusReply<QStringList> deviceListReply = m_dfmDeviceManager->call("GetBlockDevicesIdList", kRemovable);
    if (!deviceListReply.isValid()) {
        qWarning() << qPrintable("DBus call GetBlockDevicesIdList failed") << deviceListReply.error().message();
        return;
    }

    for (const QString &id : deviceListReply.value()) {
        // QueryBlockDeviceInfo(const QString &id, bool reload)
        QDBusReply<QVariantMap> deviceReply = m_dfmDeviceManager->call("QueryBlockDeviceInfo", id, false);
        if (!deviceReply.isValid()) {
            qWarning() << qPrintable("DBus call QueryBlockDeviceInfo failed") << deviceReply.error().message();
            continue;
        }

        const QVariantMap deviceInfo = deviceReply.value();
        if (QString("usb") == deviceInfo.value("ConnectionBus").toString()) {
            const QStringList mountPaths = deviceInfo.value("MountPoints").toStringList();
            if (mountPaths.isEmpty()) {
                const QString mountPath = deviceInfo.value("MountPoint").toString();

                if (!mountPath.isEmpty()) {
                    mountDevices.insert(id, mountPath);
                }
            } else {
                for (const QString &mount : mountPaths) {
                    if (!mount.isEmpty()) {
                        mountDevices.insert(id, mount);
                    }
                }
            }
        }
    }
}

/*!
   \return 返回文件路径 \a path 是否指向外部设备
 */
bool FileTrashHelper::isExternalDevice(const QString &path)
{
    for (auto itr = mountDevices.begin(); itr != mountDevices.end(); ++itr) {
        if (path.startsWith(itr.value())) {
            return true;
        }
    }

    return false;
}

/*!
   \return 判断 \a url 是否为远程挂载路径，此路径下同样无法恢复文件
 */
bool FileTrashHelper::isGvfsFile(const QUrl &url) const
{
    if (!url.isValid())
        return false;

    const QString &path = url.toLocalFile();
    static const QString gvfsMatch { "(^/run/user/\\d+/gvfs/|^/root/.gvfs/|^/media/[\\s\\S]*/smbmounts)" };
    QRegularExpression re { gvfsMatch };
    QRegularExpressionMatch match { re.match(path) };
    return match.hasMatch();
}

/*!
   \brief 通过后端文管接口将文件 \a url 移动到回收站
    注意文管接口是异步接口且没有返回值，此处通过文件变更信号判断文件是否被移动。
   \return 移动文件到回收站是否成功
 */
int FileTrashHelper::moveFileToTrashWithDBus(const QUrl &url)
{
    if (!url.isValid()) {
        return kTrashInvalidUrl;
    }

    QStringList list;
    list << url.toString();
    // 优先采用文管后端的DBus服务
    QDBusInterface interface(QStringLiteral("org.freedesktop.FileManager1"),
                             QStringLiteral("/org/freedesktop/FileManager1"),
                             QStringLiteral("org.freedesktop.FileManager1"));

    QEventLoop loop;
    int waitRet = kTrashTimeout;
    // wait file deleted signal
    QString filePath = url.path();
    auto conn = QObject::connect(ImageFileWatcher::instance(),
                                 &ImageFileWatcher::imageFileChanged,
                                 this,
                                 [&filePath, &loop, &waitRet](const QString &imagePath) {
                                     // 删除信息未通过 DBus 返回，直接判断文件是否已被删除
                                     if ((imagePath == filePath) && !QFile::exists(filePath)) {
                                         waitRet = kTrashSuccess;
                                         loop.quit();
                                     };
                                 });

    auto pendingCall = interface.asyncCall("Trash", list);
    // will return soon
    pendingCall.waitForFinished();

    if (pendingCall.isError()) {
        auto error = pendingCall.error();
        qWarning() << "Delete image by dbus error(directly):" << error.name() << error.message();
        QObject::disconnect(conn);
        return kTrashInterfaceError;
    }

    if (kTrashSuccess != waitRet) {
        // FIX-273813 Wait up to 10 seconds
        static constexpr int kDelTimeout = 1000 * 10;
        QTimer::singleShot(kDelTimeout, &loop, &QEventLoop::quit);
        loop.exec();
    }

    QObject::disconnect(conn);
    return waitRet;
}
