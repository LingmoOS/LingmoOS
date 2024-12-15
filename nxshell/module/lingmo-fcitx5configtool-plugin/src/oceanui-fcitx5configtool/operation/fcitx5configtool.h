// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef FCITX5CONFIGTOOL_H
#define FCITX5CONFIGTOOL_H

#include "fcitx5configproxy.h"
#include "fcitx5addonsproxy.h"
#include "imlistmodel.h"

#include <model.h>

#include <QObject>
#include <QVariant>
#include <QVariantMap>

namespace lingmo {
namespace fcitx5configtool {
class Fcitx5ConfigToolWorkerPrivate;
class Fcitx5ConfigToolWorker : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Fcitx5ConfigProxy *fcitx5ConfigProxy READ fcitx5ConfigProxy NOTIFY requestConfigFinished)
    Q_PROPERTY(Fcitx5AddonsProxy *fcitx5AddonsProxy READ fcitx5AddonsProxy NOTIFY requestAddonsFinished)

public:
    explicit Fcitx5ConfigToolWorker(QObject *parent = nullptr);

    Fcitx5ConfigProxy *fcitx5ConfigProxy() const;
    Fcitx5AddonsProxy *fcitx5AddonsProxy() const;
    Q_INVOKABLE void showAddonSettingsDialog(const QString &addonStr, const QString &title) const;
    Q_INVOKABLE IMListModel *imlistModel() const;
    Q_INVOKABLE fcitx::kcm::IMProxyModel *imProxyModel() const;
    Q_INVOKABLE void openLingmoAppStore() const;
    Q_INVOKABLE void showIMSettingsDialog(int index) const;
    Q_INVOKABLE void addIM(int index);
    Q_INVOKABLE void launchConfigDialog(const QString &uri, const QString &title) const;

Q_SIGNALS:
    void requestConfigFinished();
    void requestAddonsFinished();

protected Q_SLOTS:
    void init();

private:
    friend class Fcitx5ConfigToolWorkerPrivate;
    Fcitx5ConfigToolWorkerPrivate *const d;
};
}   // namespace fcitx5configtool
}   // namespace lingmo

#endif   // FCITX5CONFIGTOOL_H
