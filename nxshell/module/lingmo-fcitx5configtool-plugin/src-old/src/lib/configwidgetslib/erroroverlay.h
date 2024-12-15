/*
 * SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef _FCITX_ERROROVERLAY_H_
#define _FCITX_ERROROVERLAY_H_

#include <QPointer>
#include <QWidget>
#include <memory>

namespace Ui {
class ErrorOverlay;
}

namespace fcitx {
namespace kcm {

class DBusProvider;

class ErrorOverlay : public QWidget {
    Q_OBJECT
public:
    explicit ErrorOverlay(DBusProvider *dbus, QWidget *parent);
    ~ErrorOverlay();

    bool eventFilter(QObject *watched, QEvent *event) override;
private Q_SLOTS:
    void availabilityChanged(bool avail);
    void runFcitx5();

private:
    void reposition();
    std::unique_ptr<Ui::ErrorOverlay> ui_;
    DBusProvider *dbus;
    QPointer<QWidget> baseWidget_;
    bool enabled_ = false;
};

} // namespace kcm
} // namespace fcitx

#endif // _FCITX_ERROROVERLAY_H_
