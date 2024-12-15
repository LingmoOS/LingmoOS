// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DESKTOPFILE_H
#define DESKTOPFILE_H

#include <dfm-base/dfm_base_global.h>

#include <QStringList>

/**
 * @class DesktopFile
 * @brief Represents a linux desktop file
 * @author Michal Rost
 * @date 13.1.2013
 */

namespace dfmbase {

class DesktopFile
{
public:
    explicit DesktopFile(const QString &fileName = "");
    QString desktopFileName() const;
    QString desktopPureFileName() const;
    QString desktopName() const;
    QString desktopLocalName() const;
    QString desktopDisplayName() const;
    QString desktopExec() const;
    QString desktopIcon() const;
    QString desktopType() const;
    QString desktopLingmoId() const;
    QString desktopLingmoVendor() const;
    bool isNoShow() const;
    QStringList desktopCategories() const;
    QStringList desktopMimeType() const;

private:
    QString fileName;
    QString name;
    QString genericName;
    QString localName;
    QString exec;
    QString icon;
    QString type;
    QStringList categories;
    QStringList mimeType;
    QString lingmoId;
    QString lingmoVendor;
    bool noDisplay = false;
    bool hioceann = false;
};

}

#endif   // DESKTOPFILE_H
