// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PREVIEWPROXYINTERFACE_H
#define PREVIEWPROXYINTERFACE_H

#include <QObject>

namespace GrandSearch {

class PreviewPlugin;

inline bool requestUpdateDetailInfo(QObject *proxy, PreviewPlugin *self)
{
    return QMetaObject::invokeMethod(proxy, "updateDetailInfo", Qt::AutoConnection, Q_ARG(GrandSearch::PreviewPlugin*, self));
}

}
#endif // PREVIEWPROXYINTERFACE_H
