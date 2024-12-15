// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CHECKBOXITEM_H
#define CHECKBOXITEM_H

#include "gui/searchconfig/roundedbackground.h"
#include <DCheckBox>

namespace GrandSearch {

class CheckBoxItem : public RoundedBackground
{
    Q_OBJECT
public:
    explicit CheckBoxItem(const QString &text, QWidget *parent = nullptr);
    ~CheckBoxItem();

    void setChecked(bool checked);
    QString text() const;

signals:
    void toggled(bool checked);

private:
    Dtk::Widget::DCheckBox *m_checkBox = nullptr;
};

}

#endif   // CHECKBOXITEM_H
