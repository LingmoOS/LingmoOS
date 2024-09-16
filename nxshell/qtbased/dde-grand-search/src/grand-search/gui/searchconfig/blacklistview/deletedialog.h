// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DELETEDIALOG_H
#define DELETEDIALOG_H

#include <DDialog>

namespace GrandSearch {

class DeleteDialog : public Dtk::Widget::DDialog
{
    Q_OBJECT
public:
    explicit DeleteDialog(QWidget *parent = nullptr);
    ~DeleteDialog();
};
}
#endif // DELETEDIALOG_H
