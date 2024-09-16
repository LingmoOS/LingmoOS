// Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#include "bootdoctor.h"
#include "../corelib/helper.h"
#include "../corelib/ddevicediskinfo.h"
#include "../corelib/ddevicepartinfo.h"

QString BootDoctor::m_lastErrorString;

bool BootDoctor::fix(const QString &partDevice)
{
    m_lastErrorString.clear();

    DDevicePartInfo part_info(partDevice);
    const QString part_old_uuid = part_info.uuid();

    if (Helper::processExec("lsblk", {"-s", "-d", "-n", "-o", "UUID"}) == 0) {
        if (Helper::lastProcessStandardOutput().contains(part_old_uuid.toLatin1())) {
            // reset uuid
            if (Helper::resetPartUUID(part_info)) {
                QThread::sleep(1);
                part_info.refresh();

                qDebug() << part_old_uuid << part_info.uuid();
            } else {
                dCWarning("Failed to reset uuid");
            }
        }
    }

    bool device_is_mounted = Helper::isMounted(partDevice);
    const QString &mount_root = Helper::temporaryMountDevice(partDevice, QFileInfo(partDevice).fileName());

    if (mount_root.isEmpty()) {
        m_lastErrorString = QObject::tr("Failed to mount partition \"%1\"").arg(partDevice);
        goto failed;
    }

    {
        const QString tmp_dir = "/var/cache/deepin-clone";

        if (!QDir::current().mkpath(tmp_dir)) {
            dCError("mkpath \"%s\" failed", qPrintable(tmp_dir));
            goto failed;
        }

        const QString &repo_path = tmp_dir + "/repo.iso";

        if (!QFile::exists(repo_path)
                && !QFile::copy(QString(":/repo_%1.iso").arg(HOST_ARCH), repo_path)) {
            dCError("copy file failed, new name: %s", qPrintable(repo_path));
            goto failed;
        }

        bool ok = false;

        const QString &repo_mount_point = mount_root + "/deepin-clone";
        QFile file_boot_fix(mount_root + "/boot_fix.sh");

        do {
            if (!QDir(mount_root).exists("deepin-clone") && !QDir(mount_root).mkdir("deepin-clone")) {
                dCError("Create \"deepin-clone\" dir failed(\"%s\")", qPrintable(mount_root));
                break;
            }

            if (!Helper::mountDevice(repo_path, repo_mount_point, true)) {
                m_lastErrorString = QObject::tr("Failed to mount partition \"%1\"").arg(repo_path);
                break;
            }

            if (file_boot_fix.exists()) {
                file_boot_fix.remove();
            }

            if (!QFile::copy(QString(":/scripts/boot_fix_%1.sh").arg(
                     #if defined(HOST_ARCH_x86_64) || defined(HOST_ARCH_i386) || defined(HOST_ARCH_i686)
                                 "x86"
                     #elif defined(HOST_ARCH_mips64) || defined(HOST_ARCH_mips32)
                                 "mips"
                     #elif defined(HOST_ARCH_sw_64)
                                 "sw_64"
                     #elif defined(HOST_ARCH_aarch64)
                                 "aarch64"
                     #elif defined(HOST_ARCH_ppc64)
                                 "ppc64"
                     #elif defined(HOST_ARCH_ppc64le)
                                 "ppc64le"
                     #else
                                "unknow"
                     #endif
                                 ), file_boot_fix.fileName())) {
                dCError("copy file failed, new name: %s", qPrintable(file_boot_fix.fileName()));
                break;
            }

            if (!file_boot_fix.setPermissions(file_boot_fix.permissions() | QFile::ExeUser)) {
                dCError("Set \"%s\" permissions failed", qPrintable(file_boot_fix.fileName()));
                break;
            }

            if (Helper::processExec("mount", {"--bind", "-v", "--bind", "/dev", mount_root + "/dev"}) != 0) {
                dCError("Failed to bind /dev");
                break;
            }

            if (Helper::processExec("mount", {"--bind", "-v", "--bind", "/dev/pts", mount_root + "/dev/pts"}) != 0) {
                dCError("Failed to bind /dev/pts");
                break;
            }

            if (Helper::processExec("mount", {"--bind", "-v", "--bind", "/proc", mount_root + "/proc"}) != 0) {
                dCError("Failed to bind /proc");
                break;
            }

            if (Helper::processExec("mount", {"--bind", "-v", "--bind", "/sys", mount_root + "/sys"}) != 0) {
                dCError("Failed to bind /sys");
                break;
            }

            ok = true;
        } while (0);

        QProcess process;

        if (ok) {
            const QString &parent_device = Helper::parentDevice(partDevice);

            bool is_efi = false;

            if (!parent_device.isEmpty()) {
                DDeviceDiskInfo info(parent_device);

                dCDebug("Disk partition table type: %d", info.ptType());

                if (info.ptType() == DDeviceDiskInfo::GPT) {
                    for (const DPartInfo &part : info.childrenPartList()) {
                        if (part.guidType() == DPartInfo::EFI_SP_None) {
                            const QString &efi_path = mount_root + "/boot/efi";

                            QDir::current().mkpath(efi_path);

                            if (Helper::processExec("mount", {part.filePath(), efi_path}) != 0) {
                                dCError("Failed to mount EFI partition");
                                m_lastErrorString = QObject::tr("Failed to mount partition \"%1\"").arg(part.filePath());
                                ok = false;
                                break;
                            }

                            is_efi = true;

                            break;
                        }
                    }

                    if (!is_efi && m_lastErrorString.isEmpty()) {
                        m_lastErrorString = QObject::tr("EFI partition not found");
                        ok = false;
                    }
                } else if (info.ptType() == DDeviceDiskInfo::Unknow) {
                    m_lastErrorString = QObject::tr("Unknown partition style");
                    ok = false;
                }
            }

            if (ok) {
                process.setProcessChannelMode(QProcess::MergedChannels);
                process.start("chroot", {mount_root, "./boot_fix.sh", parent_device, is_efi ? "true" : "false", "/deepin-clone"});

                while (process.waitForReadyRead()) {
                    const QByteArray &data = process.readAll().simplified().constData();

                    dCDebug(data.constData());
                }

                process.waitForFinished(-1);

                switch (process.exitCode()) {
                case 1:
                    m_lastErrorString = QObject::tr("Boot for install system failed");
                    break;
                case 2:
                    m_lastErrorString = QObject::tr("Boot for update system failed");
                    break;
                default:
                    break;
                }
            }
        }

        // clear
        Helper::processExec("umount", {repo_mount_point});
        QDir(mount_root).rmdir("deepin-clone");
        file_boot_fix.remove();
        Helper::processExec("umount", {mount_root + "/dev/pts"});
        Helper::processExec("umount", {mount_root + "/dev"});
        Helper::processExec("umount", {mount_root + "/proc"});
        Helper::processExec("umount", {mount_root + "/sys"});
        Helper::processExec("umount", {mount_root + "/boot/efi"});

        if (ok && process.exitCode() == 0) {
            if (part_old_uuid != part_info.uuid()) {
                dCDebug("Reset the uuid from \"%s\" to \"%s\"", qPrintable(part_old_uuid), qPrintable(part_info.uuid()));

                // update /etc/fstab
                QFile file(mount_root + "/etc/fstab");

                if (file.exists() && file.open(QIODevice::ReadWrite)) {
                    QByteArray data = file.readAll();

                    if (file.seek(0)) {
                        file.write(data.replace(part_old_uuid.toLatin1(), part_info.uuid().toLatin1()));
                    }

                    file.close();
                } else {
                    dCWarning("Failed to update /etc/fstab, error: %s", qPrintable(file.errorString()));
                }

                file.setFileName(mount_root + "/etc/crypttab");

                if (file.exists() && file.open(QIODevice::ReadWrite)) {
                    QByteArray data = file.readAll();

                    if (file.seek(0)) {
                        file.write(data.replace(part_old_uuid.toLatin1(), part_info.uuid().toLatin1()));
                    }

                    file.close();
                } else {
                    dCWarning("Failed to update /etc/crypttab, error: %s", qPrintable(file.errorString()));
                }
            }

            if (!device_is_mounted)
                Helper::umountDevice(partDevice);

            return true;
        }
    }

failed:
    if (!device_is_mounted)
        Helper::umountDevice(partDevice);

    if (m_lastErrorString.isEmpty())
        m_lastErrorString = QObject::tr("Boot for repair system failed");

    dCDebug("Restore partition uuid");

    if (!Helper::resetPartUUID(part_info, part_old_uuid.toLatin1())) {
        dCWarning("Failed to restore partition uuid, part: %s, uuid: %s", qPrintable(partDevice), qPrintable(part_old_uuid));
    }

    return false;
}

QString BootDoctor::errorString()
{
    return m_lastErrorString;
}
