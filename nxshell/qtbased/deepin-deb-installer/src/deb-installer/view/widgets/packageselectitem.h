// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PACKAGESELECTITEM_H
#define PACKAGESELECTITEM_H

#include "model/packageselectmodel.h"

#include <QWidget>
#include <DLabel>

class QCheckBox;

class PackageSelectItem : public QWidget
{
    Q_OBJECT
public:
    PackageSelectItem(QWidget *parent = nullptr);

    void setDebIR(const DebIr &ir);
    bool isChecked();
    bool isEnabled();
    void setChecked(bool checked);

signals:
    void checkStatusChanged(int state);

private:
    Dtk::Widget::DLabel *nameLabel;
    Dtk::Widget::DLabel *versionLabel;
    Dtk::Widget::DLabel *descriptionLabel;
    QCheckBox *checkBox;
};

#endif  // PACKAGESELECTITEM_H
