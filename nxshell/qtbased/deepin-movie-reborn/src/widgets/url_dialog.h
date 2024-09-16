// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QtWidgets>
#include <ddialog.h>

#include <dlineedit.h>
#include <dimagebutton.h>

DWIDGET_USE_NAMESPACE

namespace dmr {
class LineEdit;

class UrlDialog: public DDialog {
public:
    explicit UrlDialog(QWidget* parent = 0);
    QUrl url() const;

public:
    void slotTextchanged();

protected:
    void showEvent(QShowEvent* se) override;

private:
    LineEdit *m_lineEdit {nullptr};
};
}
