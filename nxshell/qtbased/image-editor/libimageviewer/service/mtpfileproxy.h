// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MTPFILEPROXY_H
#define MTPFILEPROXY_H

#include <QHash>
#include <QObject>
#include <QSharedPointer>
#include <QDateTime>
#include <QTemporaryDir>

// MTP挂载设备文件代理
class MtpFileProxy : public QObject
{
    Q_OBJECT

    MtpFileProxy();
    ~MtpFileProxy();

public:
    static MtpFileProxy *instance();

    bool isValid() const;
    bool contains(const QString &proxyFile) const;
    bool checkAndCreateProxyFile(QStringList &paths, QString &firstPath);
    bool checkFileDeviceIsMtp(const QString &filePath);
    bool submitChangesToMTP(const QString &proxyFile);
    bool supportDFMIO() const;

    // 文件状态
    enum FileState { None, Loading, LoadSucc, LoadFailed, FileDelete };
    FileState state(const QString &proxyFile) const;

    QString createPorxyFile(const QString &filePath);
    QString mapToOriginFile(const QString &proxyFile) const;
    void loadFinished(const QString &proxyFile, bool ret);
    Q_SIGNAL void createProxyFileFinished(const QString &proxyFile, bool ret);
    void triggerOriginFileChanged(const QString &originFile);

private:
    struct ProxyInfo
    {
        FileState fileState = None;  // 异步处理，加载成功标识
        QString proxyFileName;
        QString originFileName;
        QDateTime lastModified;
        QTemporaryDir tempDir;  // 缓存文件目录，自动移除
    };
    void copyFileFromMtpAsync(const QSharedPointer<ProxyInfo> &proxyPtr);

    QHash<QString, QSharedPointer<ProxyInfo>> proxyCache;  // 文件缓存，程序消耗时移除
};

#endif  // MTPFILEPROXY_H
