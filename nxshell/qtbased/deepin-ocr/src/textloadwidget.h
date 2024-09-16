// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEXTLOADWIDGET_H
#define TEXTLOADWIDGET_H

#include <DWidget>
#include <DSpinner>
#include <DLabel>

DWIDGET_USE_NAMESPACE
class QVBoxLayout;

class TextLoadWidget : public DWidget
{
    Q_OBJECT
public:
    TextLoadWidget(DWidget *parent = nullptr);
private :
    DSpinner *m_spinner{nullptr};
    QVBoxLayout *m_vboxLayout{nullptr};
    DLabel *m_label{nullptr};
};

#endif // TEXTLOADWIDGET_H
