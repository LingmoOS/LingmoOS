/*
 * SPDX-FileCopyrightText: 2012~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <KFontChooser>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>

#include "font.h"
#include "fontbutton.h"

namespace fcitx {
namespace kcm {

FontButton::FontButton(QWidget *parent) : QWidget(parent) {
    setupUi(this);
    connect(fontSelectButton, &QPushButton::clicked, this,
            &FontButton::selectFont);
}

FontButton::~FontButton() {}

const QFont &FontButton::font() { return font_; }

QString FontButton::fontName() { return fontPreviewLabel->text(); }

void FontButton::setFont(const QFont &font) {
    font_ = font;
    if (font.family() != font_.family()) {
        Q_EMIT fontChanged(font_);
    }
    fontPreviewLabel->setText(fontToString(font_));
    fontPreviewLabel->setFont(font_);
}

void FontButton::selectFont() {
    QDialog dialog(NULL);
    KFontChooser *chooser = new KFontChooser(&dialog);
    chooser->setFont(font_);
    QVBoxLayout *dialogLayout = new QVBoxLayout;
    dialog.setLayout(dialogLayout);
    QDialogButtonBox *buttonBox =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttonBox->button(QDialogButtonBox::Ok)->setText(_("&OK"));
    buttonBox->button(QDialogButtonBox::Cancel)->setText(_("&Cancel"));
    dialogLayout->addWidget(chooser);
    dialogLayout->addWidget(buttonBox);
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        setFont(chooser->font());
    }
}

} // namespace kcm
} // namespace fcitx
