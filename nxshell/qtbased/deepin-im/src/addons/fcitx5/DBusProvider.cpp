// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DBusProvider.h"

using namespace org::deepin::dim;

DBusProvider::DBusProvider(QObject *parent)
    : QObject(parent)
    , watcher_(new FcitxQtWatcher(QDBusConnection::sessionBus(), this))
{
    registerFcitxQtDBusTypes();
    connect(watcher_,
            &FcitxQtWatcher::availabilityChanged,
            this,
            &DBusProvider::fcitxAvailabilityChanged);
    watcher_->watch();
}

DBusProvider::~DBusProvider()
{
    watcher_->unwatch();
}

void DBusProvider::fcitxAvailabilityChanged(bool avail)
{
    if (controller_) {
        delete controller_;
        controller_ = nullptr;
    }

    if (imProxy_) {
        delete imProxy_;
        imProxy_ = nullptr;
    }

    if (avail) {
        const auto connection = watcher_->connection();
        const auto service = watcher_->serviceName();

        controller_ = new FcitxQtControllerProxy(service, "/controller", connection, this);
        controller_->setTimeout(3000);

        auto owner = connection.interface()->serviceOwner(service);
        if (!owner.isValid()) {
            return;
        }

        imProxy_ =
            new FcitxQtInputMethodProxy(owner,
                                        QStringLiteral("/org/freedesktop/portal/inputmethod"),
                                        connection,
                                        this);
    }

    emit availabilityChanged(available());
}
