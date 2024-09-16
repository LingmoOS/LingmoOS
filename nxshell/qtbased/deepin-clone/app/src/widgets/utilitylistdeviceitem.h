// Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#ifndef UTILITYLISTDEVICEITEM_H
#define UTILITYLISTDEVICEITEM_H

#include "utilitylistitem.h"

QT_BEGIN_NAMESPACE
class QProgressBar;
QT_END_NAMESPACE

class UtilityListDeviceItem : public UtilityListItem
{
    Q_OBJECT
public:
    explicit UtilityListDeviceItem(QWidget *parent = 0);

    void setSizeInfo(qint64 used, qint64 total);

private:
    QProgressBar *m_progressBar;
    QLabel *m_sizeLabel;
};

#endif // UTILITYLISTDEVICEITEM_H
