// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SWITCHWIDGET_H
#define SWITCHWIDGET_H

#include "gui/searchconfig/roundedbackground.h"

#include <DSwitchButton>

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>

namespace GrandSearch {

class SwitchWidget : public RoundedBackground
{
    Q_OBJECT
public:
    explicit SwitchWidget(const QString &title, QWidget *parent = nullptr);
    explicit SwitchWidget(QWidget *parent = nullptr, QWidget *leftWidget = nullptr);

    void setChecked(const bool checked = true);
    bool checked() const;

    void setTitle(const QString &title);
    QString title() const;

    void setIconEnable(bool e);
    void setIcon(const QIcon &icon, const QSize &size);

    void setEnableBackground(const bool enable = true);
    bool enableBackground() const;

protected:
    void paintEvent(QPaintEvent *event) override;


signals:
    void checkedChanged(const bool checked) const;

private:
    bool m_hasBack = false;

    QHBoxLayout *m_mainLayout = nullptr;
    QLabel *m_iconLabel = nullptr;
    QWidget *m_leftWidget = nullptr;
    Dtk::Widget::DSwitchButton *m_switchBtn = nullptr;
};

}

#endif // SWITCHWIDGET_H
