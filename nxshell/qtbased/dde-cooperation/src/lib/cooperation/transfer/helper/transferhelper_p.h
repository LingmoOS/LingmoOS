// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TRANSFERHELPER_P_H
#define TRANSFERHELPER_P_H

#include "transferhelper.h"

#include <QTimer>
#include <QMap>

class CuteIPCInterface;
namespace cooperation_transfer {

class TransferHelper;
class TransferHelperPrivate : public QObject
{
    Q_OBJECT
    friend class TransferHelper;

public:
    explicit TransferHelperPrivate(TransferHelper *qq);
    ~TransferHelperPrivate();

private:
    TransferHelper *q;
    CuteIPCInterface *ipcInterface { nullptr };

    bool backendOk { false };
};

}   // namespace cooperation_transfer

#endif   // TRANSFERHELPER_P_H
