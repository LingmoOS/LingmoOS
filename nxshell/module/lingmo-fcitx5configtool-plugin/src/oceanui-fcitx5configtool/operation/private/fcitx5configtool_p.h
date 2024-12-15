// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef FCITX5CONFIGTOOL_P_H
#define FCITX5CONFIGTOOL_P_H

#include <QObject>
#include <QVariant>
#include <QVariantMap>

namespace fcitx::kcm {
class IMConfig;
class DBusProvider;
}

namespace lingmo {
namespace fcitx5configtool {
class Fcitx5ConfigProxy;
class Fcitx5AddonsProxy;
class IMListModel;
class Fcitx5ConfigToolWorker;
class Fcitx5ConfigToolWorkerPrivate : public QObject
{
    Q_OBJECT
    friend class Fcitx5ConfigToolWorker;
    Fcitx5ConfigToolWorker *const q;

    fcitx::kcm::IMConfig *imConfig { nullptr };
    fcitx::kcm::DBusProvider *dbusProvider { nullptr };
    Fcitx5ConfigProxy *configProxy { nullptr };
    Fcitx5AddonsProxy *addonsProxy { nullptr };
    IMListModel *imListModel { nullptr };

private:
    void init();
    void initConnect();

public:
    explicit Fcitx5ConfigToolWorkerPrivate(Fcitx5ConfigToolWorker *parent = nullptr);
};
}   // namespace fcitx5configtool
}   // namespace lingmo

#endif   // FCITX5CONFIGTOOL_P_H
