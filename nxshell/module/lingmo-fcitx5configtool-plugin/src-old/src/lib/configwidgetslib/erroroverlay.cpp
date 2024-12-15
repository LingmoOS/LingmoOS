/*
 * SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "erroroverlay.h"
#include "dbusprovider.h"
#include "ui_erroroverlay.h"
#include <QAbstractButton>
#include <QIcon>
#include <fcitx-utils/standardpath.h>

namespace fcitx {
namespace kcm {

ErrorOverlay::ErrorOverlay(DBusProvider *dbus, QWidget *parent)
    : QWidget(parent), ui_(std::make_unique<Ui::ErrorOverlay>()),
      baseWidget_(parent) {
    ui_->setupUi(this);
    setVisible(false);

    baseWidget_->installEventFilter(this);
    ui_->pixmapLabel->setPixmap(
        QIcon::fromTheme("dialog-error-symbolic").pixmap(64));

    connect(baseWidget_, &QObject::destroyed, this, &QObject::deleteLater);
    connect(dbus, &DBusProvider::availabilityChanged, this,
            &ErrorOverlay::availabilityChanged);

    connect(ui_->runFcitxButton, &QAbstractButton::pressed, this,
            &ErrorOverlay::runFcitx5);
    availabilityChanged(dbus->available());
}

ErrorOverlay::~ErrorOverlay() {}

void ErrorOverlay::availabilityChanged(bool avail) {
    const bool newEnabled = !avail;
    if (enabled_ != newEnabled) {
        enabled_ = newEnabled;
        setVisible(newEnabled);
        if (newEnabled) {
            reposition();
        }
    }
}

void ErrorOverlay::runFcitx5() {
    QProcess::startDetached(
        QString::fromStdString(StandardPath::fcitxPath("bindir", "fcitx5")),
        QStringList());
}

void ErrorOverlay::reposition() {
    if (!baseWidget_) {
        return;
    }

    // follow base widget visibility
    // needed eg. in tab widgets
    if (!baseWidget_->isVisible()) {
        hide();
        return;
    }

    show();

    // follow position changes
    const QPoint topLevelPos = baseWidget_->mapTo(window(), QPoint(0, 0));
    const QPoint parentPos = parentWidget()->mapFrom(window(), topLevelPos);
    move(parentPos);

    // follow size changes
    // TODO: hide/scale icon if we don't have enough space
    resize(baseWidget_->size());
    raise();
}

bool ErrorOverlay::eventFilter(QObject *object, QEvent *event) {
    if (enabled_ && object == baseWidget_ &&
        (event->type() == QEvent::Move || event->type() == QEvent::Resize ||
         event->type() == QEvent::Show || event->type() == QEvent::Hide ||
         event->type() == QEvent::ParentChange)) {
        reposition();
    }
    return QWidget::eventFilter(object, event);
}

} // namespace kcm
} // namespace fcitx
