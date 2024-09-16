// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DBUSCLOUDSYNC_H
#define DBUSCLOUDSYNC_H

#include "dservicebase.h"
#include "daccountmodule.h"

class Dbuscloudsync : public DServiceBase
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface","com.deepin.dataserver.cloudsync")
public:
    Dbuscloudsync(QObject* parent = nullptr);

public slots:
   /**
    * @brief MsgCallBack        云同步信息回调
    * @return
    */
   void MsgCallBack(const QByteArray &msg);
};

#endif // DBUSCLOUDSYNC_H
