// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DCC_MODULE_LAYOUTWIDGET_H
#define DCC_MODULE_LAYOUTWIDGET_H

#include <DFrame>

struct xkb_context;

class QLabel;

enum class LayoutStatus;

class LayoutWidget : public DTK_WIDGET_NAMESPACE::DFrame
{
    Q_OBJECT

public:
    LayoutWidget(QWidget *parent = 0);
    ~LayoutWidget();

    void setKeyboardLayout(const QString &layout, const QString &variant = QString());
    void showMulti();
    void showNoLayout();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    xkb_context *m_ctx;

    LayoutStatus m_status;
    std::string m_layout;
    std::string m_variant;

    QLabel *m_label;

    void paintLayout(QPainter &painter);
    void showNoLayoutLabel();
    void showMultiLayoutLabel();
};

#endif
