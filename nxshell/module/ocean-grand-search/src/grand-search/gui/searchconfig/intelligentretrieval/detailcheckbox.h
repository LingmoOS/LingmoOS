// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DETAILCHECKBOX_H
#define DETAILCHECKBOX_H

#include "gui/searchconfig/roundedbackground.h"

#include <DCheckBox>

class QVBoxLayout;
class QLabel;
namespace GrandSearch {

class DetailCheckBox : public RoundedBackground
{
    Q_OBJECT
public:
    explicit DetailCheckBox(const QString &title, QWidget *parent = nullptr);
    explicit DetailCheckBox(const QString &title, QWidget *detail, QWidget *parent);
    inline Dtk::Widget::DCheckBox *checkBox() const {
        return m_checkBox;
    }
    void setDetail(const QString &detail);
signals:

public slots:
protected:
    Dtk::Widget::DCheckBox *m_checkBox = nullptr;
    QVBoxLayout *m_mainLayout = nullptr;
    QWidget *m_detail = nullptr;
};

}
#endif // DETAILCHECKBOX_H
