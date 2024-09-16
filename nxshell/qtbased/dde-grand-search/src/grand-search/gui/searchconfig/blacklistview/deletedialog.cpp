// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "deletedialog.h"

#include <QAbstractButton>

using namespace GrandSearch;

DeleteDialog::DeleteDialog(QWidget *parent)
    : DDialog(parent)
{
    const QString content(tr("Do you want to remove the path from the exclusion list?"));
    setMessage(content);
    setIcon(QIcon(QString(":/icons/%1.svg").arg("dde-grand-search-setting")));

    // the cancel button
    {
        int idx = addButton(tr("Cancel", "button"));
        auto btn = getButton(idx);
        Q_ASSERT(btn);
        connect(btn, &QAbstractButton::clicked, this, &QDialog::reject);
    }

    // the confirm button
    {
        auto idx = addButton(tr("Confirm", "button"), false, ButtonWarning);
        auto btn = getButton(idx);
        Q_ASSERT(btn);
        connect(btn, &QAbstractButton::clicked, this, &QDialog::accept);
    }
}

DeleteDialog::~DeleteDialog()
{

}
