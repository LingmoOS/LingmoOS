// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MEM_H
#define MEM_H

#include <QSharedDataPointer>

namespace core {
namespace system {

class MemInfoPrivate;
class MemInfo
{
public:
    explicit MemInfo();
    MemInfo(const MemInfo &other);
    MemInfo &operator=(const MemInfo &rhs);
    virtual ~MemInfo();

    qulonglong memTotal() const;
    qulonglong memAvailable() const;
    qulonglong buffers() const;
    qulonglong cached() const;
    qulonglong active() const;
    qulonglong inactive() const;

    qulonglong swapTotal() const;
    qulonglong swapFree() const;
    qulonglong swapCached() const;
    qulonglong shmem() const;
    qulonglong slab() const;
    qulonglong dirty() const;
    qulonglong mapped() const;

    void readMemInfo();

private:
    QSharedDataPointer<MemInfoPrivate> d;
};

} // namespace system
} // namespace core

#endif // MEM_H
