// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UDISKSUTILS_H
#define UDISKSUTILS_H

#include <QString>
#include <QScopedPointer>

class DBlockDevice;
class UDisksBlock
{
public:
    UDisksBlock(const QString &devnode);
    ~UDisksBlock();

    bool isReadOnly() const;
    QString displayName() const;
    QString iconName() const;
    QString fsType() const;
    qint64 sizeTotal() const;
    qint64 sizeUsed() const;

    DBlockDevice* operator ->();
private:
    QScopedPointer<DBlockDevice> blk;
};
#endif // UDISKSUTILS_H
