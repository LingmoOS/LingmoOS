// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QRunnable>
#include <QThreadPool>

#define FONTS_DESKTOP_DIR (QString("%1/%2/").arg(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)).arg(QApplication::translate("DFontMgrMainWindow", "Fonts")))

/**
* @brief 拷贝文件线程类
* 进行文件的批量拷贝
*/
class CopyFontThread : public QObject, public QRunnable
{
    Q_OBJECT
public:
    /**
    * @brief The OPType （拷贝类型：导出/安装）
    */
    enum OPType {
        EXPORT = 0,
        INSTALL,
        INVALID,
    };

    CopyFontThread(OPType type, short index);
    void run() override;
    void appendFile(const QString &filePath);

signals:
    void fileInstalled(const QString &familyName, const QString &targetName);

private:
    //拷贝类型：导出 安装
    short m_opType;
    //线程下标
    short m_index;
    //需要拷贝文件的源路径列表
    QStringList m_srcFiles;
    //需要拷贝文件的目标路径列表
    QStringList m_targetFiles;
};


class QGSettings;
/**
* @brief 拷贝文件管理类
* 进行文件批量拷贝的管理
*/
class DCopyFilesManager : public QObject
{
    Q_OBJECT

public:
    explicit DCopyFilesManager(QObject *parent = nullptr);

    static DCopyFilesManager *instance();
    //拷贝文件列表
    void copyFiles(CopyFontThread::OPType type, QStringList &fontList);
    //取消安装
    static inline void cancelInstall()
    {
        m_installCanceled = true;
    }

    //安装是否已被取消
    static inline bool isInstallCanceled()
    {
        return m_installCanceled;
    }

    //获取字体源路径、目标路径和familyName
    static QString getTargetPath(const QString &inPath, QString &srcPath, QString &targetPath);

    //删除取消安装时已经安装的字体文件列表
    static void deleteFiles(const QStringList  &fileList, bool isTarget);

    //获取线程池
    inline QThreadPool *getPool()
    {
        if (m_useGlobalPool)
            return QThreadPool::globalInstance();

        return m_localPool;
    }

    //获取排序后的字体列表，默认升序
    inline void sortFontList(QStringList &fonts)
    {
        if (m_sortOrder == 0 || fonts.isEmpty() || fonts.size() == 1)
            return;

        int end = fonts.size() / 2;

        for (int i = 0; i < end; ++i) {
            fonts.swap(i, fonts.size() - 1 - i);
        }
    }

private:
    //单例
    static DCopyFilesManager *inst;
    //专有线程池
    QThreadPool *m_localPool;
    //文件拷贝类型：导出 安装
    static qint8 m_type;
    //安装是否被取消
    static volatile bool m_installCanceled;
    //使用专有线程池还是全局线程池
    bool m_useGlobalPool;
    //最大线程个数
    qint8 m_maxThreadCnt;
    //导出最大线程个数
    qint8 m_exportMaxThreadCnt;
    //安装最大线程个数
    qint8 m_installMaxThreadCnt;
    //升序倒序 0: asc 1 : desc
    qint8 m_sortOrder;
    //expiracy time
    int m_expiryTimeout;
};
