// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DBUSPROVIDER_H
#define DBUSPROVIDER_H

#include <fcitxqtcontrollerproxy.h>
#include <fcitxqtinputmethodproxy.h>
#include <fcitxqtwatcher.h>

#include <QObject>

using namespace fcitx;

namespace org {
namespace deepin {
namespace dim {
class DBusProvider : public QObject
{
    Q_OBJECT

public:
    explicit DBusProvider(QObject *parent = nullptr);
    ~DBusProvider() override;

    inline bool available() const { return controller_ != nullptr; }

    inline FcitxQtWatcher *watch() const { return watcher_; }

    inline FcitxQtControllerProxy *controller() { return controller_; }

    inline FcitxQtInputMethodProxy *imProxy() { return imProxy_; }

signals:
    void availabilityChanged(bool avail);

private slots:
    void fcitxAvailabilityChanged(bool avail);

private:
    FcitxQtWatcher *watcher_;
    FcitxQtControllerProxy *controller_ = nullptr;
    FcitxQtInputMethodProxy *imProxy_ = nullptr;
};

} // namespace dim
} // namespace deepin
} // namespace org

#endif
