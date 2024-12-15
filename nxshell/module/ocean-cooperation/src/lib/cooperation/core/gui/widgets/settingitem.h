// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SETTINGITEM_H
#define SETTINGITEM_H

#include <QFrame>
#include <QHBoxLayout>

namespace cooperation_core {

class SettingItem : public QFrame
{
    Q_OBJECT

public:
    explicit SettingItem(QWidget *parent = nullptr);

    void setItemInfo(const QString &text, QWidget *w);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QHBoxLayout *mainLayout { nullptr };
};

}   // namespace cooperation_core

#endif   // SETTINGITEM_H
