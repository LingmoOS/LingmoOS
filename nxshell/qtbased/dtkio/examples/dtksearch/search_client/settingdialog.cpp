// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "settingdialog.h"

#include <DLabel>

#include <QGridLayout>
#include <QStandardPaths>
#include <QDir>

SettingDialog::SettingDialog(QWidget *parent)
    :DDialog (parent)
{
    initUI();
}

QVariantMap SettingDialog::searchConfig()
{
    QVariantMap map;
    map.insert("path", pathEdit->text());
    map.insert("index", indexPathEdit->text());
    map.insert("mode", modeCheckBox->currentText());
    map.insert("count", maxResultBox->value());

    return map;
}

void SettingDialog::initUI()
{
    QWidget *widget = new QWidget(this);
    QGridLayout *layout = new QGridLayout;
    widget->setLayout(layout);

    DLabel *modeLabel = new DLabel(tr("Search mode:"), this);
    modeCheckBox = new DComboBox(this);
    modeCheckBox->addItems({tr("File name"), tr("Pinyin"), tr("Full-Text"), tr("All")});

    DLabel *pathLabel = new DLabel(tr("Search path:"), this);
    pathEdit = new DLineEdit(this);
    pathEdit->setText(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));

    DLabel *indexPathLabel = new DLabel(tr("Index path:"), this);
    indexPathEdit = new DLineEdit(this);
    indexPathEdit->setText(QDir::currentPath() + "/index");

    DLabel *maxResultLabel = new DLabel(tr("Max result count:"), this);
    maxResultBox = new DSpinBox(this);
    maxResultBox->setMaximum(INT32_MAX);
    maxResultBox->setMinimum(1);
    maxResultBox->setValue(100);

    layout->addWidget(modeLabel, 0, 0);
    layout->addWidget(modeCheckBox, 0, 1);
    layout->addWidget(pathLabel, 1, 0);
    layout->addWidget(pathEdit, 1, 1);
    layout->addWidget(indexPathLabel, 2, 0);
    layout->addWidget(indexPathEdit, 2, 1);
    layout->addWidget(maxResultLabel, 3, 0);
    layout->addWidget(maxResultBox, 3, 1);

    addButton(tr("Save"), true, DDialog::ButtonRecommend);
    addContent(widget);
}
