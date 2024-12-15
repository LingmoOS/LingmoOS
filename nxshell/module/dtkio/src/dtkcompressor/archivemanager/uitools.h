// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef UITOOLS_H
#define UITOOLS_H

#include "archiveinterface.h"
#include "mimetypes.h"
#include "plugin.h"
#include "uistruct.h"
#include "dcompressornamespace.h"
#include <QObject>

#define SAFE_DELETE_ELE(ptr) \
    if (ptr != nullptr) {    \
        delete ptr;          \
        ptr = nullptr;       \
    }

#define SAFE_DELETE_ARRAY(ptr) \
    if (ptr != nullptr) {      \
        delete[] ptr;          \
        ptr = nullptr;         \
    }

#define SAFE_DELETE_TABLE(ptr)        \
    if (ptr != nullptr) {             \
        for (i = 0; i < row; i++) {   \
            SAFE_DELETE_ARRAY(ptr[i]) \
            delete[] ptr;             \
        }                             \
        ptr = nullptr;                \
    }

DCOMPRESSOR_BEGIN_NAMESPACE

class UiTools : public QObject
{
    Q_OBJECT

public:
    explicit UiTools(QObject *parent = nullptr);
    ~UiTools();

    enum AssignPluginType {
        APT_Auto,
        APT_Cli7z,
        APT_Libarchive,
        APT_Libzip,
        APT_Libpigz
    };

    static QString getConfigPath();
    static QString humanReadableSize(const qint64 &size, int precision);
    static bool isArchiveFile(const QString &strFileName);
    static bool isExistMimeType(const QString &strMimeType, bool &bArchive);
    static QString readConf();
    static QString toShortString(QString strSrc, int limitCounts = 16, int left = 8);
    static ReadOnlyArchiveInterface *createInterface(const QString &fileName, bool bWrite = false, AssignPluginType eType = APT_Auto);
    static ReadOnlyArchiveInterface *createInterface(const QString &fileName, const CustomMimeType &mimeType, Plugin *plugin);
    static void transSplitFileName(QString &fileName, UnCompressParameter::SplitType &eSplitType);
    static QString handleFileName(const QString &strFileName);
    static bool isLocalDeviceFile(const QString &strFileName);
    static QStringList removeSameFileName(const QStringList &listFiles);
    static bool isWayland();

public:
    static QStringList m_associtionlist;
};
DCOMPRESSOR_END_NAMESPACE
#endif   // UITOOLS_H
