// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CHECKBOXWIDGET_H
#define CHECKBOXWIDGET_H

#include <DWidget>

#include <QVBoxLayout>

namespace GrandSearch {

class CheckBoxItem;
class CheckBoxWidget : public Dtk::Widget::DWidget
{
    Q_OBJECT
public:
    explicit CheckBoxWidget(QWidget *parent = nullptr);
    ~CheckBoxWidget();

    int addCheckBox(const QString &text, bool isChecked = false);
    void addCheckBox(int index, CheckBoxItem *item, bool isChecked = false);
    bool setCheckBoxChecked(int index, bool isChecked);
    bool setCheckBoxChecked(const QString &text, bool isChecked);

signals:
    void checkedChanged(int index, bool checked);

private slots:
    void onCheckBoxChecked(bool checked);

private:
    QList<CheckBoxItem *> m_checkBoxList;
    QVBoxLayout *m_mainLayout;
};

}

#endif   // CHECKBOXWIDGET_H
