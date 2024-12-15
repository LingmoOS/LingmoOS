/*
 * SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "layoutselector.h"
#include "dbusprovider.h"
#include "keyboardlayoutwidget.h"
#include "layoutmodel.h"
#include "ui_layoutselector.h"
#include <QDBusPendingCallWatcher>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QStringListModel>
#include <fcitx-utils/i18n.h>
#include <fcitxqtcontrollerproxy.h>
#include <fcitxqtdbustypes.h>

namespace fcitx {
namespace kcm {

LayoutSelector::LayoutSelector(DBusProvider *dbus, QWidget *parent)
    : QWidget(parent), ui_(std::make_unique<Ui::LayoutSelector>()), dbus_(dbus),
      layoutProvider_(new LayoutProvider(dbus, this)) {
    ui_->setupUi(this);

    ui_->languageComboBox->setModel(layoutProvider_->languageModel());
    ui_->layoutComboBox->setModel(layoutProvider_->layoutModel());
    ui_->variantComboBox->setModel(layoutProvider_->variantModel());

    connect(layoutProvider_, &LayoutProvider::loadedChanged, this, [this]() {
        if (layoutProvider_->loaded()) {
            setLayout(preSelectLayout_, preSelectVariant_);
        }
    });

    connect(ui_->languageComboBox,
            qOverload<int>(&QComboBox::currentIndexChanged), this,
            &LayoutSelector::languageComboBoxChanged);
    connect(ui_->layoutComboBox,
            qOverload<int>(&QComboBox::currentIndexChanged), this,
            &LayoutSelector::layoutComboBoxChanged);
    connect(ui_->variantComboBox,
            qOverload<int>(&QComboBox::currentIndexChanged), this,
            &LayoutSelector::variantComboBoxChanged);
    if (qApp->platformName() == "xcb") {
        keyboardLayoutWidget_ = new KeyboardLayoutWidget(this);
        keyboardLayoutWidget_->setMinimumSize(QSize(400, 200));
        keyboardLayoutWidget_->setSizePolicy(QSizePolicy::Expanding,
                                             QSizePolicy::Expanding);
        ui_->verticalLayout->addWidget(keyboardLayoutWidget_);
    }
}

LayoutSelector::~LayoutSelector() {}

QPair<QString, QString>
LayoutSelector::selectLayout(QWidget *parent, DBusProvider *dbus,
                             const QString &title, const QString &layout,
                             const QString &variant, bool *ok) {
    QPointer<QDialog> dialog(new QDialog(parent));
    auto mainLayout = new QVBoxLayout(dialog);
    dialog->setLayout(mainLayout);
    dialog->setWindowTitle(title);
    auto layoutSelector = new LayoutSelector(dbus, dialog);
    mainLayout->addWidget(layoutSelector);
    layoutSelector->setLayout(layout, variant);

    auto buttonBox =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                             Qt::Horizontal, dialog);
    buttonBox->button(QDialogButtonBox::Ok)->setText(_("&OK"));
    buttonBox->button(QDialogButtonBox::Cancel)->setText(_("&Clear"));
    connect(buttonBox, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
    mainLayout->addWidget(buttonBox);

    auto ret = dialog->exec();
    if (ok) {
        *ok = !!ret;
    }
    if (ret) {
        return layoutSelector->layout();
    } else {
        return {};
    }
}

void LayoutSelector::setLayout(const QString &layout, const QString &variant) {
    if (!layoutProvider_->loaded()) {
        preSelectLayout_ = layout;
        preSelectVariant_ = variant;
        return;
    }
    ui_->languageComboBox->setCurrentIndex(0);
    ui_->layoutComboBox->setCurrentIndex(layoutProvider_->layoutIndex(layout));
    if (variant.isEmpty()) {
        ui_->variantComboBox->setCurrentIndex(0);
    } else {
        ui_->variantComboBox->setCurrentIndex(
            layoutProvider_->variantIndex(variant));
    }
    preSelectLayout_.clear();
    preSelectVariant_.clear();
}

QPair<QString, QString> LayoutSelector::layout() const {
    return {ui_->layoutComboBox->currentData().toString(),
            ui_->variantComboBox->currentData().toString()};
}

void LayoutSelector::languageComboBoxChanged() {
    layoutProvider_->layoutModel()->setLanguage(
        ui_->languageComboBox->currentData().toString());
}

void LayoutSelector::layoutComboBoxChanged() {
    ui_->variantComboBox->clear();
    if (ui_->layoutComboBox->currentIndex() < 0) {
        return;
    }

    layoutProvider_->setVariantInfo(
        ui_->layoutComboBox->currentData(LayoutInfoRole)
            .value<FcitxQtLayoutInfo>());
    ui_->variantComboBox->setCurrentIndex(0);
}

void LayoutSelector::variantComboBoxChanged() {
    if (!keyboardLayoutWidget_) {
        return;
    }

    auto layout = ui_->layoutComboBox->currentData().toString();
    auto variant = ui_->variantComboBox->currentData().toString();
    if (layout.isEmpty()) {
        keyboardLayoutWidget_->setVisible(false);
    } else {
        keyboardLayoutWidget_->setKeyboardLayout(layout, variant);
        keyboardLayoutWidget_->setVisible(true);
    }
}

} // namespace kcm
} // namespace fcitx
