// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <DBlurEffectWidget>
#include <DFrame>

QT_BEGIN_NAMESPACE
class QVBoxLayout;
class QHBoxLayout;
QT_END_NAMESPACE

DWIDGET_USE_NAMESPACE

class UsbInfoDialog : public DFrame
{
    Q_OBJECT
public:
    explicit UsbInfoDialog(QWidget *parent = nullptr);
    inline QVBoxLayout *mainLayout() {return m_mainLayout;}

Q_SIGNALS:

public Q_SLOTS:

protected:
    void resizeEvent(QResizeEvent *event);
    void closeEvent(QCloseEvent *event);
    void paintEvent(QPaintEvent *event);

private:
    void initUI();

private:
    DBlurEffectWidget *m_blurEffect;
    QWidget *m_mainWidgt;
    QVBoxLayout *m_mainLayout;
};
