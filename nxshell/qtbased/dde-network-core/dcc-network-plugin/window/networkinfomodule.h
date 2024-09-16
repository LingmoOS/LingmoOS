// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef NETWORKINFOMODULE_H
#define NETWORKINFOMODULE_H
#include "interface/pagemodule.h"

#include <dtkwidget_global.h>

DWIDGET_BEGIN_NAMESPACE
class DListView;
DWIDGET_END_NAMESPACE

namespace dde {
namespace network {
class ControllItems;
}
}

class NetworkInfoModule : public DCC_NAMESPACE::PageModule
{
    Q_OBJECT
public:
    explicit NetworkInfoModule(QObject *parent = nullptr);

private Q_SLOTS:
    void onUpdateNetworkInfo();
};

#endif // NETWORKINFOMODULE_H
