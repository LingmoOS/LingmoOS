/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2021, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: deng hao <denghao@kylinos.cn>
 *
 */

#include "drive-rename.h"

#include <QDebug>
#include <file-info.h>
#include <QMessageBox>
#include <QInputDialog>
#include <gio/gio.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <sys/stat.h>
#include <udisks/udisks.h>
#include <QTranslator>
#include <QFile>
#include <QApplication>
#include <file-utils.h>
#include <file-enumerator.h>

typedef struct _DeviceRenameData        DeviceRenameData;

void device_rename(const char* devName, const char* name);
static void udisk_umounted (GMount* mount, GAsyncResult *res, gpointer udata);
UDisksObject* getObjectFromBlockDevice(UDisksClient* client, const gchar* bdevice);
static void udisk_setLabel_callback(UDisksFilesystem *diskFilesystem, GAsyncResult *res, gpointer udata);

struct _DeviceRenameData
{
    QString             devName;
    QString             rename;
    Peony::DriveRename* pThis;
};

Peony::DriveRename::DriveRename(QObject *parent) : QObject(parent), mEnable(true)
{
    QTranslator *t = new QTranslator(this);
    t->load(":/translations/explorer-drive-rename_"+QLocale::system().name());
    QApplication::installTranslator(t);
}

QList<QAction *> Peony::DriveRename::menuActions(Peony::MenuPluginInterface::Types types, const QString &uri, const QStringList &selectionUris)
{
    QList<QAction *> l;

    if (selectionUris.count() != 1 || types != Peony::MenuPluginInterface::Type::SideBar) {
        return l;
    }

    QString suri = selectionUris.first();

    g_autoptr(GFile) file = g_file_new_for_uri(suri.toUtf8().constData());
    g_return_val_if_fail(file, l);

    // check is mount point and get dev name
    g_autoptr (GError) error = NULL;
    g_autoptr (GFileInfo) fileInfo = g_file_query_info (file, G_FILE_ATTRIBUTE_UNIX_IS_MOUNTPOINT "," G_FILE_ATTRIBUTE_MOUNTABLE_CAN_UNMOUNT "," G_FILE_ATTRIBUTE_MOUNTABLE_UNIX_DEVICE_FILE "," G_FILE_ATTRIBUTE_STANDARD_TARGET_URI, G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL, &error);
    if (error) qDebug() << error->message;
    g_return_val_if_fail (G_IS_FILE_INFO (fileInfo) && !error, l);

    gboolean canUmount = g_file_info_get_attribute_boolean (fileInfo, G_FILE_ATTRIBUTE_MOUNTABLE_CAN_UNMOUNT);
    gboolean isMountPoint = g_file_info_get_attribute_boolean (fileInfo, G_FILE_ATTRIBUTE_UNIX_IS_MOUNTPOINT);

    g_autofree char* devName = g_file_info_get_attribute_as_string (fileInfo, G_FILE_ATTRIBUTE_MOUNTABLE_UNIX_DEVICE_FILE);

    bool canRename = (isMountPoint || canUmount || devName);

    qDebug() << "uri: " << uri << "  " << (canRename ? "can rename" : "can not rename");
    g_return_val_if_fail (canRename, l);

    QString type = Peony::FileUtils::getFileSystemType(suri);
    // if device has mounted
    GMount* mount = NULL;
    if (isMountPoint || canUmount) {
        if (uri.startsWith ("computer://")) {
            g_autofree char* targetUri = g_file_info_get_attribute_as_string (fileInfo, G_FILE_ATTRIBUTE_STANDARD_TARGET_URI);
            if (targetUri) {

                // related to: #105070
//                bool isWayland = qApp->property("isWayland").toBool();
//                if (!isWayland) {
//                    QString tmp = targetUri;
//                    if (tmp == "file:///data") {
//                        // 如果是data盘，则跳过
//                        return l;
//                    }
//                }

                bool isWayland = qApp->property("isWayland").toBool();
                if (isWayland) {
                    QString tmp = targetUri;
                    if (tmp == "file:///data") {
                        return l;
                    }
                }

                g_autoptr (GFile) tfile = g_file_new_for_uri (targetUri);
                if (G_IS_FILE (tfile)) {
                    mount = g_file_find_enclosing_mount (tfile, NULL, &error);
                    if (error) qDebug() << error->message;
                }
            }
        } else {
            mount = g_file_find_enclosing_mount (file, NULL, &error);
            if (error) qDebug() << error->message;
        }

        g_return_val_if_fail (G_IS_MOUNT (mount), l);

        if(type.startsWith("iso")){
            g_autoptr (GFile) root = g_mount_get_root (mount);
            g_autoptr (GFileInfo) rootInfo = g_file_query_info (root, "access::", G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL, &error);
            if (error)      qDebug() << "error: " << error->message;
            bool canRename = true;
            if (G_IS_FILE_INFO (rootInfo)) {
                bool write = true;
                if (g_file_info_has_attribute (rootInfo, G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE)) {
                    write = g_file_info_get_attribute_boolean (rootInfo, G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE);
                }
                bool execute = true;
                if (g_file_info_has_attribute (rootInfo, G_FILE_ATTRIBUTE_ACCESS_CAN_EXECUTE)) {
                    execute = g_file_info_get_attribute_boolean (rootInfo, G_FILE_ATTRIBUTE_ACCESS_CAN_EXECUTE);
                }

                if (!write || !execute)   canRename = false;
            }

            g_return_val_if_fail (canRename, l);
        }

        FileEnumerator e;
        e.setEnumerateDirectory(suri);
        e.enumerateSync();
        QStringList uris = e.getChildrenUris();
        int count = 0;
        for (QString uri1 : uris) {
            if (uri1.contains("boot") || uri1.contains("EFI")) {
                ++count;
            }
            if (count >= 2) {
                return l;
            }
        }

        if (!devName) {
            g_autoptr (GVolume) volume = g_mount_get_volume (mount);
            g_return_val_if_fail (G_IS_VOLUME (volume), l);
            devName = g_volume_get_identifier (volume, G_DRIVE_IDENTIFIER_KIND_UNIX_DEVICE);
        }
    }

    g_return_val_if_fail (devName, l);

    // 注意要筛选的设备，块设备、非根设备
    // 判断是否为光盘设备(光盘设备不允许重命名)
    if (suri.startsWith("computer:///") && suri.endsWith(".drive")
            && 0 != g_ascii_strncasecmp ("/dev/cd", devName, 7)
            && 0 != g_ascii_strncasecmp ("/dev/sr", devName, 7)
            && 0 != g_ascii_strncasecmp ("/dev/loop", devName, 9)
            && !type.startsWith("iso")) {
        mDevName = devName;
        QAction* action = new QAction(tr("Rename"));
        if (action) {
            l << action;
        }

        // 这里 action 弹框并执行重命名函数
        connect(action, &QAction::triggered, this, [=] () {

            // 输入框出来，提示用户U盘重命名为 ...
            // 获取到用户的输入
            // 执行重命名
            bool ok = false;
            QString text = QInputDialog::getText(nullptr, tr("Rename"), tr("Device name:"), QLineEdit::Normal, "", &ok);
            if (ok && !text.isNull() && !text.isEmpty()) {
                //首字符是.提示非法 bug#93280
                if(text.at(0) == '.'){
                    QMessageBox::warning(nullptr, tr("Warning"), tr("Renaming cannot start with a decimal point, Please re-enter!"), QMessageBox::Ok);
                    return;
                }

                QString type = Peony::FileUtils::getFileSystemType(suri);
                if((type.compare(QString::fromLocal8Bit("vfat")) == 0 && text.toUtf8().length() > 11)
                        || (type.compare(QString::fromLocal8Bit("exfat")) == 0 && text.toUtf8().length() > 15)
                        || (type.compare(QString::fromLocal8Bit("ext4")) == 0 && text.toUtf8().length() > 16)){
                    QMessageBox::warning(nullptr, tr("Warning"), tr("The device name exceeds the character limit, rename failed!"), QMessageBox::Ok);
                    return;
                }

                // 修改名字
                if (mount) {
                    int ret = QMessageBox::warning (nullptr, tr("Warning"), tr("Renaming will unmount the device. Do you want to continue?"), QMessageBox::Ok | QMessageBox::Cancel);
                    if (QMessageBox::Cancel == ret) return;

                    // 释放、释放 GMount
                    DeviceRenameData* data = new DeviceRenameData;
                    data->devName = mDevName;
                    data->rename = text;
                    data->pThis = this;
                    g_autoptr (GMountOperation) mount_op = g_mount_operation_new();
                    g_mount_unmount_with_operation (mount, G_MOUNT_UNMOUNT_NONE, mount_op, NULL, (GAsyncReadyCallback) udisk_umounted, data);
                } else {
//                    int ret = device_rename(mDevName.toUtf8().constData(), text.toUtf8().constData());
//                    if (0 != ret) {
//                        QMessageBox::warning (nullptr, tr("Warning"), tr("The device may not support the rename operation, rename failed!"), QMessageBox::Ok);
//                    }
                    device_rename(mDevName.toUtf8().constData(), text.toUtf8().constData());
                }
            }
        });
    }

    Q_UNUSED(uri)
    Q_UNUSED(types)

    return l;
}


void device_rename(const char* devName, const char* name)
{
    //判断参数个数是否合法
    g_return_if_fail(devName && name);

    UDisksClient* client = udisks_client_new_sync(NULL, NULL);
    g_return_if_fail(client);


    UDisksObject* udiskObj = getObjectFromBlockDevice(client, devName);
    g_return_if_fail(udiskObj);

    //从设备名获取文件系统类型
    UDisksFilesystem* diskFilesystem = udisks_object_get_filesystem(udiskObj);
    g_return_if_fail(diskFilesystem);
    GVariantBuilder optionsBuilder;
    g_variant_builder_init(&optionsBuilder, G_VARIANT_TYPE_VARDICT);
    g_variant_builder_add (&optionsBuilder, "{sv}", "label", g_variant_new_string (devName));
    g_variant_builder_add (&optionsBuilder, "{sv}", "take-ownership", g_variant_new_boolean (TRUE));

//    g_autoptr (GError) error = NULL;
//    gboolean ret = udisks_filesystem_call_set_label_sync (diskFilesystem, name, g_variant_builder_end(&optionsBuilder), NULL, &error);
//    if (error) {
//        qDebug() << error->message;
//    }

//    return ret ? 0 : -1;
    QList<Experimental_Peony::Volume> *volumeLists;
    Experimental_Peony::Volume *volume = nullptr;
    volumeLists = Experimental_Peony::VolumeManager::getInstance()->allVaildVolumes();
    int volumeCount = volumeLists->count();
    for (int i = 0; i < volumeCount; ++i) {
       auto tmpVolume = volumeLists->at(i);
       if (tmpVolume.device() == devName) {
           volume = new Experimental_Peony::Volume(tmpVolume);
           break;
       }
    }

    udisks_filesystem_call_set_label(diskFilesystem, name, g_variant_builder_end(&optionsBuilder), NULL, GAsyncReadyCallback(udisk_setLabel_callback), volume);
}


UDisksObject* getObjectFromBlockDevice(UDisksClient* client, const gchar* bdevice)
{
    struct stat statbuf;
    UDisksBlock* block = NULL;
    UDisksObject* object = NULL;
    UDisksObject* cryptoBackingObject = NULL;
    g_autofree const gchar* cryptoBackingDevice = NULL;

    g_return_val_if_fail(stat(bdevice, &statbuf) == 0, object);

    block = udisks_client_get_block_for_dev (client, statbuf.st_rdev);
    g_return_val_if_fail(block != NULL, object);

    object = UDISKS_OBJECT (g_dbus_interface_dup_object (G_DBUS_INTERFACE (block)));

    cryptoBackingDevice = udisks_block_get_crypto_backing_device ((udisks_object_peek_block (object)));
    cryptoBackingObject = udisks_client_get_object (client, cryptoBackingDevice);
    if (cryptoBackingObject != NULL) {
        g_object_unref (object);
        object = cryptoBackingObject;
    }

    g_object_unref (block);

    return object;
}

static void udisk_umounted (GMount* mount, GAsyncResult *res, gpointer udata)
{
    g_autoptr (GError) error = NULL;

    //int ret = -1;
    DeviceRenameData* data = (DeviceRenameData*) udata;

    if (g_mount_unmount_with_operation_finish (G_MOUNT (mount), res, &error)) {
        device_rename(data->devName.toUtf8().constData(), data->rename.toUtf8().constData());
//        error = g_error_new (1, G_IO_ERROR_FAILED, data->pThis->tr("The device may not support the rename operation, rename failed!").toUtf8 ().constData (), NULL);
    } else {
        if (error) qDebug() << error->message;
    }

    if (/*ret != 0 &&*/ error) {
        if(strcmp(error->message, "Not authorized to perform operation")){/* gmountOperation会弹出授权框，防止二次弹框,linkto bug#101075 */
            QMessageBox::warning(nullptr, data->pThis->tr("Warning"), error->message, QMessageBox::Ok);
        }
    }

    if (data)   delete data;
    if (mount)  g_object_unref (mount);
}

static void udisk_setLabel_callback(UDisksFilesystem *diskFilesystem, GAsyncResult *res, gpointer udata)
{
    Q_UNUSED(udata);
    g_autoptr (GError) error = NULL;

    Experimental_Peony::Volume *volume = static_cast<Experimental_Peony::Volume*>(udata);

    gboolean ret = udisks_filesystem_call_set_label_finish(diskFilesystem, res, &error);
    if (!ret && error) {
        qDebug() << "udisk_setLabel_callback error:" << error->code << error->message;
        QMessageBox::warning (nullptr, QObject::tr("Warning"), QObject::tr("The device may not support the rename operation, rename failed!"), QMessageBox::Ok);
    } else if (!error) {
        volume->mount();
    }

    if (volume) {
        delete volume;
    }
}
