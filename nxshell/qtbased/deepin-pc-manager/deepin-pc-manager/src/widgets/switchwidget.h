// Copyright (C) 2011 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SWITCHWIDGET_H
#define SWITCHWIDGET_H

#include "widgets/settingsitem.h"

#include <dswitchbutton.h>

QT_BEGIN_NAMESPACE
class QLabel;
QT_END_NAMESPACE

namespace def {
namespace widgets {

class SwitchWidget : public SettingsItem
{
    Q_OBJECT

public:
    //    explicit SwitchWidget(QWidget *parent = nullptr);
    explicit SwitchWidget(const QString &title, QWidget *parent = nullptr);
    explicit SwitchWidget(QWidget *parent = nullptr, QWidget *widget = nullptr);

    void setChecked(const bool checked = true);
    void setTitle(const QString &title);
    bool checked() const;

    QWidget *leftWidget() const { return m_leftWidget; }

public:
    inline DTK_WIDGET_NAMESPACE::DSwitchButton *switchButton() const { return m_switchBtn; }

Q_SIGNALS:
    void checkedChanged(bool checked) const;
    void clicked();

protected:
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

private:
    QWidget *m_leftWidget;
    Dtk::Widget::DSwitchButton *m_switchBtn;
};

} // namespace widgets
} // namespace def

#endif // SWITCHWIDGET_H
