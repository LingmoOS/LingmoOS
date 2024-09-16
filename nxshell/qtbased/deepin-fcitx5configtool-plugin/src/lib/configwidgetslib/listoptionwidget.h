/*
 * SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#ifndef _KCM_FCITX5_LISTOPTIONWIDGET_H_
#define _KCM_FCITX5_LISTOPTIONWIDGET_H_

#include "optionwidget.h"
#include "ui_listoptionwidget.h"

namespace fcitx {
namespace kcm {

class ListOptionWidgetModel;

class ListOptionWidget : public OptionWidget, public Ui::ListOptionWidget {
    Q_OBJECT
public:
    ListOptionWidget(const FcitxQtConfigOption &option, const QString &path,
                     QWidget *parent);

    void readValueFrom(const QVariantMap &map) override;
    void writeValueTo(QVariantMap &map) override;
    void restoreToDefault() override;

    const auto &subOption() { return subOption_; }

private:
    void updateButton();
    ListOptionWidgetModel *model_;
    FcitxQtConfigOption subOption_;
    QVariantMap defaultValue_;
};

} // namespace kcm
} // namespace fcitx

#endif // _KCM_FCITX5_LISTOPTIONWIDGET_H_
