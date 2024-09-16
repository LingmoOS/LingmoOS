// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TRANSFERHELPER_P_H
#define TRANSFERHELPER_P_H

#include "transferhelper.h"
#include "dialogs/cooperationdialog.h"

#include <QTimer>
#include <QMap>

#ifdef __linux__
#    include <net/linux/noticeutil.h>
#endif

namespace cooperation_core {

class TransferHelper;
class TransferHelperPrivate : public QObject
{
    Q_OBJECT
    friend class TransferHelper;

public:
    struct TransferInfo
    {
        int64_t totalSize = 0;   // 总量
        int64_t transferSize = 0;   // 当前传输量
        int64_t maxTimeS = 0;   // 耗时

        void clear()
        {
            totalSize = 0;
            transferSize = 0;
            maxTimeS = 0;
        }
    };

    explicit TransferHelperPrivate(TransferHelper *qq);
    ~TransferHelperPrivate();

    CooperationTransDialog *transDialog();

    void reportTransferResult(bool result);
    void notifyMessage(const QString &body, const QStringList &actions, int expireTimeout);
    void initConnect();

private:
    TransferHelper *q;

    CooperationTransDialog *dialog { nullptr };

    QStringList readyToSendFiles;

    TransferHelper::TransferType role;
    QString who;

    QAtomicInt status { TransferHelper::Idle };
    TransferInfo transferInfo;
    bool isTransTimeout = false;
    QString recvFilesSavePath;

    QTimer confirmTimer;

#ifdef __linux__
    NoticeUtil *notice { nullptr };
#endif
};

}   // namespace cooperation_core

#endif   // TRANSFERHELPER_P_H
