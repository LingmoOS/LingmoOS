// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TAILWIDGET_H
#define TAILERWIDGET_H

#include "checkboxwidget/checkboxwidget.h"

#include <DWidget>
#include <DLabel>
#include <DCheckBox>

#include <QVBoxLayout>

namespace GrandSearch {

class TailerWidget : public Dtk::Widget::DWidget
{
    Q_OBJECT
public:
    enum TailerProperty
    {
        ParentDirectory,
        TimeModified
    };

    explicit TailerWidget(QWidget *parent = nullptr);
    ~TailerWidget();

private slots:
    void onCheckBoxStateChanged(int index, bool checked);

private:
    QVBoxLayout *m_mainLayout = nullptr;
    QLabel *m_groupLabel = nullptr;
    QLabel *m_contentLabel = nullptr;
    CheckBoxWidget *m_checkBoxWidget = nullptr;
};

}

#endif   // TAILERWIDGET_H
