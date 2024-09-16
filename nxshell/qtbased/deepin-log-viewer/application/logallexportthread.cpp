// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logallexportthread.h"
#include "dbusproxy/dldbushandler.h"
#include "logapplicationhelper.h"
#include "utils.h"

#include <QLoggingCategory>
#ifdef QT_DEBUG
Q_LOGGING_CATEGORY(logExportAll, "org.deepin.log.viewer.exportall.work")
#else
Q_LOGGING_CATEGORY(logExportAll, "org.deepin.log.viewer.exportall.work", QtInfoMsg)
#endif

LogAllExportThread::LogAllExportThread(const QStringList &types, const QString &outfile, QObject *parent)
    : QObject(parent)
    , m_types(types)
    , m_outfile(outfile)
{
}

void LogAllExportThread::run()
{
    //判断权限
    qCInfo(logExportAll) << "outFile: " << m_outfile;
    QFileInfo info(m_outfile);
    if (!QFileInfo(info.path()).isWritable()) {
        qCCritical(logExportAll) << QString("outdir:%1 it not writable or is not exist.").arg(info.absolutePath());
        emit exportFinsh(false);
        return;
    }

    QList<EXPORTALL_DATA> eList;

    int nCount = 0;
    //获取所有文件
    for (auto &it : m_types) {
        EXPORTALL_DATA data;
        if (it.contains(JOUR_TREE_DATA, Qt::CaseInsensitive)) {
            data.logCategory = "system";
            data.commands.push_back("journalctl_system");
        } else if (it.contains(BOOT_KLU_TREE_DATA, Qt::CaseInsensitive)) {
            data.logCategory = "boot";
            data.commands.push_back("journalctl_boot");
        } else if (it.contains(DMESG_TREE_DATA, Qt::CaseInsensitive)) {
            data.logCategory = "kernel";
            data.commands.push_back("dmesg");
        } else if (it.contains(LAST_TREE_DATA, Qt::CaseInsensitive)) {
            data.logCategory = "boot-shutdown-event";
            data.commands.push_back("last");
        } else if (it.contains(DPKG_TREE_DATA, Qt::CaseInsensitive)) {
            data.logCategory = "dpkg";
            data.files.append(DLDBusHandler::instance(nullptr)->getFileInfo("dpkg", false));
        } else if (it.contains(KERN_TREE_DATA, Qt::CaseInsensitive)) {
            data.logCategory = "kernel";
            data.files.append(DLDBusHandler::instance(nullptr)->getFileInfo("kern", false));
        } else if (it.contains(XORG_TREE_DATA, Qt::CaseInsensitive)) {
            data.logCategory = "xorg";
            data.files.append(DLDBusHandler::instance(nullptr)->getFileInfo("Xorg", false));
        } else if (it.contains(DNF_TREE_DATA, Qt::CaseInsensitive)) {
            data.logCategory = "dnf";
            data.files.append(DLDBusHandler::instance(nullptr)->getFileInfo("dnf", false));
        } else if (it.contains(BOOT_TREE_DATA, Qt::CaseInsensitive)) {
            data.logCategory = "boot";
            data.files.append(DLDBusHandler::instance(nullptr)->getFileInfo("boot", false));
        } else if (it.contains(KWIN_TREE_DATA, Qt::CaseInsensitive)) {
            data.logCategory = "kwin";
            data.files.append(KWIN_TREE_DATA);
        } else if (it.contains(APP_TREE_DATA, Qt::CaseInsensitive)) {
            data.logCategory = "apps";
            AppLogConfigList appConfigs = LogApplicationHelper::instance()->getAppLogConfigs();
            for (auto appLogConfig : appConfigs) {
                QString appName = appLogConfig.name;
                if(appName.isEmpty() || !appLogConfig.visible)
                    continue;

                for (int i = 0; i < appLogConfig.subModules.size(); i++) {
                    SubModuleConfig& submodule = appLogConfig.subModules[i];
                    QString dir = appLogConfig.name;
                    QString subDir = QString("%1/%2").arg(dir).arg(submodule.name);
                    if (appLogConfig.subModules.size() == 1 &&  submodule.name == appLogConfig.name)
                        subDir = dir;
                    if (submodule.logType == "file") {
                        QStringList logPaths = DLDBusHandler::instance(nullptr)->getFileInfo(submodule.logPath);
                        logPaths.removeDuplicates();
                        if (logPaths.size() > 0) {
                            data.dir2Files[subDir] = logPaths;
                        } else {
                            qCWarning(logExportAll) << QString("app:%1 submodule:%2, logPath:%3 not found log files.").arg(appName).arg(submodule.name).arg(submodule.logPath);
                        }
                    } else if (submodule.logType == "journal") {
                        if (submodule.filter.endsWith("*"))
                            qCWarning(logExportAll) << QString("app:%1 submodule:%2, Export journal logs with wildcard not supported.").arg(appName).arg(submodule.name);
                        else {
                            QJsonObject obj = submodule.toJson();
                            data.dir2Cmds[subDir] = QStringList() << QJsonDocument(obj).toJson(QJsonDocument::Compact);
                        }
                    }
                }
            }
        } else if (it.contains(COREDUMP_TREE_DATA, Qt::CaseInsensitive)) {
            data.logCategory = "coredump";
            data.files.append(DLDBusHandler::instance(nullptr)->getFileInfo("coredump", false));
        } else if (it.contains(OTHER_TREE_DATA, Qt::CaseInsensitive)) {
            data.logCategory = "others";
            auto otherLogListPair = LogApplicationHelper::instance()->getOtherLogList();
            for (auto &it2 : otherLogListPair) {
                QStringList paths = DLDBusHandler::instance(nullptr)->getOtherFileInfo(it2.at(1));
                paths.removeDuplicates();
                if (paths.size() > 1)
                    data.dir2Files[it2.at(0)] = paths;
                else if (paths.size() == 1)
                    data.files.append(paths);
            }
        } else if (it.contains(CUSTOM_TREE_DATA, Qt::CaseInsensitive)) {
            data.logCategory = "customized";
            auto customLogListPair = LogApplicationHelper::instance()->getCustomLogList();
            for (auto &it2 : customLogListPair) {
                data.files.append(it2.at(1));
            }
        } else if (it.contains(AUDIT_TREE_DATA, Qt::CaseInsensitive)) {
            data.logCategory = "audit";
            data.files.append(DLDBusHandler::instance(nullptr)->getFileInfo("audit", false));
        }

        eList.push_back(data);
        data.files.removeDuplicates();
        data.commands.removeDuplicates();
        nCount += data.files.size() + data.commands.size() + data.dir2FilesCount() + data.dir2CmdsCount();

        //取消导出直接返回
        if (m_cancel) {
            emit exportFinsh(false);
            return;
        }
    }

    if (eList.isEmpty()) {
        emit exportFinsh(false);
        return;
    }

    //删除原文件
    if (info.exists() && !QFile::remove(m_outfile)) {
        emit exportFinsh(false);
        return;
    }

    int tolProcess = nCount + 10;
    int currentProcess = 1;
    emit updateTolProcess(tolProcess);
    QString tmpPath = Utils::getAppDataPath() + "/tmp/";
    QDir dir(tmpPath);
    //删除临时目录
    dir.removeRecursively();
    //创建临时目录
    Utils::mkMutiDir(tmpPath);
    for (auto &it : eList) {
        //复制文件到一级目录
        QString tmpCategoryPath = QString("%1%2/").arg(tmpPath).arg(it.logCategory);
        Utils::mkMutiDir(tmpCategoryPath);
        for (auto &file : it.files) {
            DLDBusHandler::instance(this)->exportLog(tmpCategoryPath, file, true);
            emit updatecurrentProcess(currentProcess++);
            if (m_cancel) {
                break;
            }
        }

        // 复制文件到二级目录
        if (!m_cancel) {
            QMapIterator<QString, QStringList> itMap(it.dir2Files);
            while (itMap.hasNext()) {
                itMap.next();
                if (m_cancel)
                    break;

                if (itMap.value().size() > 0) {
                    QString tmpSubCategoryPath = QString("%1%2/").arg(tmpCategoryPath).arg(itMap.key());
                    Utils::mkMutiDir(tmpSubCategoryPath);
                    for (auto &path : itMap.value()) {
                        DLDBusHandler::instance(this)->exportLog(tmpSubCategoryPath, path, true);
                        emit updatecurrentProcess(currentProcess++);
                        if (m_cancel) {
                            break;
                        }
                    }
                }
            }
        } else
            break;

        // 导出命令查询内容到二级目录
        if (!m_cancel) {
            QMapIterator<QString, QStringList> itMap(it.dir2Cmds);
            while (itMap.hasNext()) {
                itMap.next();
                if (m_cancel)
                    break;

                if (itMap.value().size() > 0) {
                    QString tmpSubCategoryPath = QString("%1%2/").arg(tmpCategoryPath).arg(itMap.key());
                    Utils::mkMutiDir(tmpSubCategoryPath);
                    for (auto &cmd : itMap.value()) {
                        DLDBusHandler::instance(this)->exportLog(tmpSubCategoryPath, cmd, false);
                        emit updatecurrentProcess(currentProcess++);
                        if (m_cancel) {
                            break;
                        }
                    }
                }
            }
        } else
            break;

        // 执行获取日志命令
        if (!m_cancel) {
            for (auto &command : it.commands) {
                DLDBusHandler::instance(this)->exportLog(tmpCategoryPath, command, false);
                emit updatecurrentProcess(currentProcess++);
                if (m_cancel) {
                    break;
                }
            }
        } else
            break;
    }

    if (!m_cancel) {
        //打包日志文件
        QProcess procss;
        procss.setWorkingDirectory(tmpPath);
        QStringList arg = {"-c"};
        arg.append(QString("chmod -R 777 '%1';zip -r tmp.zip ./*;mv tmp.zip '%2';chmod 777 '%3'").arg(Utils::getAppDataPath()).arg(m_outfile).arg(m_outfile));
        procss.start("/bin/bash", arg);
        procss.waitForFinished(-1);
        currentProcess += 9;
        emit updatecurrentProcess(currentProcess);
    }

    //删除临时目录
    dir.removeRecursively();
    //取消导出删除输出文件
    if (m_cancel) {
        QFile::remove(m_outfile);
    }
    emit exportFinsh(!m_cancel && QFileInfo(m_outfile).exists());
}
