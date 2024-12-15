// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILETRASHHELPER_H
#define FILETRASHHELPER_H

#include <QDBusInterface>
#include <QDir>
#include <QObject>
#include <QUrl>

// 文件移动至回收站 辅助类
class FileTrashHelper : public QObject
{
    Q_OBJECT

public:
    explicit FileTrashHelper(QObject *parent = nullptr);

    Q_INVOKABLE bool fileCanTrash(const QUrl &url);
    Q_INVOKABLE bool moveFileToTrash(const QUrl &url);
    Q_INVOKABLE bool removeFile(const QUrl &url);

    Q_INVOKABLE void resetMountInfo();

private:
    void queryMountInfo();
    bool isExternalDevice(const QString &path);
    bool isGvfsFile(const QUrl &url) const;

    int moveFileToTrashWithDBus(const QUrl &url);

private:
    QScopedPointer<QDBusInterface> m_dfmDeviceManager;

    bool initData { false };                    // 挂载数据是否被初始化
    QDir lastDir;                               // 上一次访问的文件目录
    QMultiHash<QString, QString> mountDevices;  // 当前挂载设备信息
};

#endif  // FILETRASHHELPER_H
