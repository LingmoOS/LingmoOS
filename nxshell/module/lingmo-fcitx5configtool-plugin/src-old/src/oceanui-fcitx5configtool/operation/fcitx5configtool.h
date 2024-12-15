// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef FCITX5CONFIGTOOL_H
#define FCITX5CONFIGTOOL_H

#include <QObject>
#include <QVariant>
#include <QVariantMap>

namespace lingmo {
namespace fcitx5configtool {

class Fcitx5ConfigToolWorker : public QObject
{
    Q_OBJECT

public:
    explicit Fcitx5ConfigToolWorker(QObject *parent = nullptr);
};
} // namespace fcitx5configtool
} // namespace lingmo

#endif // FCITX5CONFIGTOOL_H
