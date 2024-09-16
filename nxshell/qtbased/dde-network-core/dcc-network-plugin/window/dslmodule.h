// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DSLMODULE_H
#define DSLMODULE_H
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

class DSLModule : public DCC_NAMESPACE::PageModule
{
    Q_OBJECT
public:
    explicit DSLModule(QObject *parent = nullptr);

private Q_SLOTS:
    void initDSLList(DTK_WIDGET_NAMESPACE::DListView *lvsettings);
    void editConnection(dde::network::ControllItems *item, QWidget *parent);
};

#endif // DSLMODULE_H
