/*
 * SPDX-FileCopyrightText: 2012~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef _KCM_FCITX_FONTBUTTON_H_
#define _KCM_FCITX_FONTBUTTON_H_

#include "ui_fontbutton.h"

namespace fcitx {
namespace kcm {

class FontButton : public QWidget, public Ui::FontButton {
    Q_OBJECT
public:
    explicit FontButton(QWidget *parent = 0);
    virtual ~FontButton();
    const QFont &font();
    QString fontName();

public Q_SLOTS:
    void setFont(const QFont &font);
Q_SIGNALS:
    void fontChanged(const QFont &font);
private Q_SLOTS:
    void selectFont();

private:
    QFont font_;
};

} // namespace kcm
} // namespace fcitx

#endif // _KCM_FCITX_FONTBUTTON_H_
